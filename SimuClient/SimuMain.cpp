#include "../AdminServer/admin_client.h"
#include "../Comp/comp_client.h"

// #include <google/protobuf/stubs/common.h>
// using google::protobuf::int32;
// using google::protobuf::uint32;

#include "ReqAnalytics.h"
#include "ServiceSelector.h"
#include "phxrpc/network/timer.h"
#include <iostream>
#include <map>
#include <vector>
#include <random>
using namespace std;

AdminClient * g_adminProxy;
map<string, ServiceSelector> * g_serviceTable;

bool TestAccessAdminServer();

bool ReadTrafficFile(string filePath, vector<AppReq> & traffic);

int UpdateServiceTable(map<string, ServiceSelector> & table);

int StartSimu(
	const vector<AppReq> & traffic, 
	vector<ReqLog> & rstData, 
	map<int32_t, int32_t> & retMap);

int main()
{
	AdminClient::Init("../AdminServer/admin_client.conf");
	CompClient::Init("../Comp/comp_client.conf");
	g_adminProxy = new AdminClient;

	// 测试是否可以访问AdminServer
	if (!TestAccessAdminServer())
	{
		cout << "AdminServer not available\n";
		return -1;
	}

	// 读入测试流量集
	string dataFile = "../TrafficGenerator/test.dat";
	vector<AppReq> traffic;
	if (!ReadTrafficFile(dataFile, traffic)) 
	{
		cout << "Traffic file read error\n";
		return -2;
	}

	// 获取服务路由表
	map<string, ServiceSelector> serviceTable;
	if (UpdateServiceTable(serviceTable) < 0)
	{
		cout << "Fail to get ServiceTable\n";
		return -3;
	}
	g_serviceTable = &serviceTable;

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
		string serviceName = traffic[i].service;
		simuReq.set_servicename(serviceName);

		ReqLog reqLog;
		reqLog.req = traffic[i];

		reqLog.begin = phxrpc::Timer::GetSteadyClockMS();
		map<string, ServiceSelector>::iterator foundIt = g_serviceTable->find(serviceName);
		if (foundIt == g_serviceTable->end())
		{
			cout << "Service not found\n";
			continue;// 需要慎重考虑是否可以直接跳过后续代码
		}
		
		CompClient cc;
		
		int32_t ret = cc.Handle(foundIt->second.GetService(), simuReq, &simuRsp);
		
		//int32_t ret = g_adminProxy->Handle(simuReq, &simuRsp);

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

int UpdateServiceTable(map<string, ServiceSelector> & table)
{
	magna::ServiceTableRequest req;
	magna::ServiceTableResponse rsp;
	int ret = g_adminProxy->GetServiceTable(req, &rsp);
	if (ret != 0)
	{
		return -1;
	}

	table.clear();
	phxrpc::Endpoint_t ep;
	for (int32_t i = 0; i < rsp.routertable().size(); ++i)
	{
		string name = rsp.routertable(i).name();
		memset(&ep, 0, sizeof(ep));
		strcpy(ep.ip, rsp.routertable(i).ep().ip().c_str());
		ep.port = rsp.routertable(i).ep().port();
		double percentage = rsp.routertable(i).percentage();
		table[name].AddService(ep, percentage);
	}
	return 0;
}

