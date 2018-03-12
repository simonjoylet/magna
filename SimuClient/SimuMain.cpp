#include "../AdminServer/admin_client.h"

// #include <google/protobuf/stubs/common.h>
// using google::protobuf::int32;
// using google::protobuf::uint32;

#include "ReqAnalytics.h"
#include "phxrpc/network/timer.h"
#include <iostream>
#include <map>
#include <vector>
using namespace std;

AdminClient * g_adminProxy;

bool TestAccessAdminServer();

bool ReadTrafficFile(string filePath, vector<AppReq> & traffic);

int StartSimu(
	const vector<AppReq> & traffic, 
	vector<ReqLog> & rstData, 
	map<int32_t, int32_t> & retMap);

int main()
{
	AdminClient::Init("../AdminServer/admin_client.conf");
	g_adminProxy = new AdminClient;

	// 测试是否可以访问AdminServer
	if (!TestAccessAdminServer())
	{
		cout << "AdminServer无法访问\n";
		return -1;
	}

	// 读入测试流量集
	string dataFile = "../TrafficGenerator/test.dat";
	vector<AppReq> traffic;
	if (!ReadTrafficFile(dataFile, traffic)) 
	{
		cout << "流量文件读取错误\n";
		return -2;
	}

	// 向AdminServer发出服务请求，并记录开始和截止的时间戳
	vector<ReqLog> rstData;
	map<int32_t, int32_t> retMap;
	uint64_t stamp = phxrpc::Timer::GetSteadyClockMS();
	StartSimu(traffic, rstData, retMap);
	cout << "Time Used: " << phxrpc::Timer::GetSteadyClockMS() - stamp << "ms\n";
	return 0;
}

bool TestAccessAdminServer()
{
	google::protobuf::StringValue req;
	google::protobuf::StringValue resp;
	req.set_value("Access AdminServer Success");
	int32_t ret = g_adminProxy->PhxEcho(req, &resp);
	printf("AdminServer.PhxEcho return %d\n", ret);
	printf("resp: {\n%s}\n", resp.DebugString().c_str());
	return ret == 0;
}

bool ReadTrafficFile(string filePath, vector<AppReq> & traffic)
{
	FILE * f = fopen(filePath.c_str(), "rb");
	if (f == NULL)
	{
		return false;
	}
	uint32_t n = 0;
	fread(&n, sizeof(n), 1, f);
	for (uint32_t i = 0; i < n; ++i)
	{
		AppReq req;
		fread(&req, sizeof(req), 1, f);
		traffic.push_back(req);
	}
	fclose(f);
	return true;
}

int StartSimu(const vector<AppReq> & traffic, vector<ReqLog> & rstData, map<int32_t, int32_t> & retMap)
{
	magna::AppRequest simuReq;
	magna::AppResponse simuRsp;
	for (uint32_t i = 0; i < traffic.size(); ++i)
	{

		simuReq.set_id(traffic[i].id);
		simuReq.set_clienttype(traffic[i].weight);
		simuReq.set_servicename(traffic[i].service);
		ReqLog reqLog;
		reqLog.req = traffic[i];
		reqLog.begin = phxrpc::Timer::GetSteadyClockMS();
		int32_t ret = g_adminProxy->Handle(simuReq, &simuRsp);

		if (ret == 0)
		{
			reqLog.end = phxrpc::Timer::GetSteadyClockMS();
			rstData.push_back(reqLog);
		}
		else
		{
			++retMap[ret];
		}

	}

	return 0;
}

