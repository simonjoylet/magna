#include "AdminData.h"
#include "../SimuClient/ReqAnalytics.h"

AdminData * AdminData::m_instance = NULL;

AdminData::AdminData()
{
	m_serviceIdCount = 0;
}

AdminData * AdminData::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new AdminData();
	}

	return m_instance;
}

void AdminData::InitServiceTable(vector<localdata::RouterItem> & router)
{
	m_router = router;
// 	for (uint32_t i = 0; i < router.size(); ++i)
// 	{
// 		m_router.push_back(router[i]);
// 	}
}

int32_t AdminData::UpdateServiceTable()
{
	// 按名字累计每种服务的到达率，计算所需的机器台数，进而判断资源是否够用。
	map<string, uint32_t> serviceLamda;
	for (auto it = m_serviceList.begin(); it != m_serviceList.end(); it++)
	{
		localdata::ServiceInfo & serviceInfo = it->second;
		if (serviceLamda.find(serviceInfo.name) == serviceLamda.end())
		{
			serviceLamda[serviceInfo.name] = serviceInfo.lamda;
		}
		else
		{
			serviceLamda[serviceInfo.name] += serviceInfo.lamda;
		}
	}




	// 如果够用，找到满足要求的路由表，对比当前路由表，启动或关闭组件。
	// 如果不够用，按照负载均衡进行路由表计算。

}


// 读取组件的压测数据
int32_t AdminData::ReadStressData(string compName, string filePath)
{
	FILE * stressFile = fopen(filePath.c_str(), "rb");
	if (stressFile == NULL)
	{
		printf("File open failed, path: %s\n", filePath.c_str());
		return -1;
	}

	// 读取负载数据
	vector<LoadLog> loadLogList;
	uint32_t loadLogCount = 0;
	fread(&loadLogCount, sizeof(loadLogCount), 1, stressFile);
	for (size_t i = 0; i < loadLogCount; i++)
	{
		LoadLog log;
		fread(&log, sizeof(LoadLog), 1, stressFile);
		loadLogList.push_back(log);
	}


	// 读取请求数据
	vector<ReqLog> reqLogList;
	uint32_t reqLogCount = 0;
	fread(&reqLogCount, sizeof(reqLogCount), 1, stressFile);
	for (size_t i = 0; i < reqLogCount; ++i)
	{
		ReqLog log;
		fread(&log, sizeof(ReqLog), 1, stressFile);
		reqLogList.push_back(log);
	}
	fclose(stressFile);

	// 分析数据并保存
	vector<localdata::StressInfo> stressVec;

	m_serviceStress[compName] = stressVec;
}

int32_t AdminData::GetNewServiceId()
{
	lock();
	int32_t rst = ++m_serviceIdCount;
	unlock();
	return rst;
}

localdata::InetAddress::InetAddress(string _ip, uint16_t _port)
{
	ip = _ip;
	port = _port;
}

bool localdata::InetAddress::operator<(const InetAddress & param)
{
	if (ip < param.ip)
	{
		return true;
	}
	else if (ip == param.ip)
	{
		return port < param.port;
	}
	else
	{
		return false;
	}
}
