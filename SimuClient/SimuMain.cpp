#include "SimuFunc.h"

int main(int argc, char **argv)
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
	StartSimu(traffic, rstData, retMap);

	
	return 0;
}


