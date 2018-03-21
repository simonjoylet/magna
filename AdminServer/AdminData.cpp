#include "AdminData.h"
#include "../SimuClient/ReqAnalytics.h"
#include <math.h>

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

void AdminData::InitServiceTable(vector<localdata::RouterItem> & router)
{
	m_router = router;
// 	for (uint32_t i = 0; i < router.size(); ++i)
// 	{
// 		m_router.push_back(router[i]);
// 	}
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
	map<uint32_t, vector<LoadLog>> loadMap;
	uint32_t loadLogCount = 0;
	fread(&loadLogCount, sizeof(loadLogCount), 1, stressFile);
	for (size_t i = 0; i < loadLogCount; i++)
	{
		LoadLog log;
		fread(&log, sizeof(LoadLog), 1, stressFile);
		loadMap[log.sendLamda].push_back(log);
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
	// 计算各个发送到达强度下平均的资源利用率和处理时间
	localdata::CompStress compStress;
	compStress.name = compName;
	vector<localdata::StressInfo> & stressVec = compStress.stressVec;
	uint32_t finishCount = 0;
	for (auto it = loadMap.begin(); it != loadMap.end(); ++it)
	{
		localdata::StressInfo stressInfo;
		stressInfo.lamda = it->first;
		vector<LoadLog> & loadList = it->second;
		uint32_t logCount = 0;
		uint32_t firstId = 0, lastId = 0; // 用于计算队列增长速度
		for (uint32_t i = 0; i < loadList.size(); ++i)
		{
			++logCount;
			stressInfo.cpuLoad += loadList[i].cpuLoad;
			stressInfo.diskLoad += loadList[i].diskLoad;

			uint32_t curId = loadList[i].sendId;
			ReqLog & currentReq = reqLogList[curId];
			if (currentReq.localEnd > 0)
			{
				++finishCount;
				compStress.processTime += currentReq.processTime;
				lastId = curId;
			}
			
			if (curId == reqLogList.size()) // 最后一条记录也只统计一次。
			{
				break;
			}
		}
		// 计算队列增长速度
		if (reqLogList[loadList[0].sendId].localEnd > 0)
		{
			firstId = loadList[0].sendId;
		}
		ReqLog & firstLog = reqLogList[firstId];
		ReqLog & lastLog = reqLogList[lastId];
		if ((lastLog.localBegin - firstLog.localBegin) != 0)
		{
			stressInfo.queueGrowSpeed = 1000.0 * (lastLog.queueLength - firstLog.queueLength) / (lastLog.localBegin - firstLog.localBegin);
		}
		
		stressInfo.cpuLoad /= logCount;
		stressInfo.diskLoad /= logCount;
		

		stressVec.push_back(stressInfo);
	}

	// 计算每个请求的平均处理时间
	if (finishCount > 0)
	{
		compStress.processTime /= finishCount;
	}

	// 计算单位到达强度的资源消耗
	for (uint32_t i = 0; i < stressVec.size(); ++i)
	{
		if (stressVec[i].queueGrowSpeed > 1)
		{
			double deltaLamda = (stressVec[i - 1].lamda - stressVec[0].lamda);
			compStress.cpuPerLamda = ((stressVec[i - 1].cpuLoad - compStress.cpuBase) - (stressVec[0].cpuLoad - compStress.cpuBase)) / deltaLamda;
			compStress.diskPerLamda = ((stressVec[i - 1].diskLoad - compStress.diskBase) - (stressVec[0].diskLoad - compStress.diskBase)) / deltaLamda;
			compStress.cpuPerLamda = compStress.cpuPerLamda > 0 ? compStress.cpuPerLamda : 0; // 资源消耗必须大于0
			compStress.diskPerLamda = compStress.diskPerLamda > 0 ? compStress.diskPerLamda : 0; 
			break;
		}
		
	}

	m_stressMap[compName] = compStress;
	return 0;
}

// 计算单位CPU的其他资源占用，求差的平方，数值越大，说明资源互补性越高，亲缘度越高。
double CalAffinity(vector<double> & vec1, vector<double> &vec2)
{

}

int32_t AdminData::UpdateServiceTable()
{
	// 按名字累计每种服务的到达率
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

	// 计算最少所需的机器台数。
	double cpuNeed = 0, diskNeed = 0;
	for (auto it = serviceLamda.begin(); it != serviceLamda.end(); it++)
	{
		string serviceName = it->first;
		localdata::CompStress & compStress = m_stressMap[serviceName];
		uint32_t curLamda = it->second;
		cpuNeed += curLamda * compStress.cpuPerLamda;
		diskNeed += curLamda * compStress.diskPerLamda;
	}
	uint32_t needMachineAmount = ceil((cpuNeed > diskNeed ? cpuNeed : diskNeed) / MAX_UTILIZATION);

	// 构造结果数据结构
	struct ScheduleItem
	{
		string name;
		uint32_t machineId;
		uint32_t lamda;
	};
	vector<ScheduleItem> rstVec;

	// 如果够用，尽量在当前路由表的基础上做修改，追求负载均衡。
	if (needMachineAmount <= m_nodeList.size())
	{
		// 计算各节点最均衡的负载值
		double cpuOpt = cpuNeed / needMachineAmount;
		double diskOpt = diskNeed / needMachineAmount;

		// 用于计算亲缘性的负载
		double cpuAffinity = MAX_UTILIZATION - cpuOpt;
		double diskAffinity = MAX_UTILIZATION - diskOpt;

		// 根据组件亲缘度，按照贪心策略进行资源分配。
		map<string, localdata::CompStress> leftServiceVec = m_stressMap;

		// 构建机器数组
		struct MachineLoad 
		{
			double cpuUsed;
			double diskUsed;
			MachineLoad() : cpuUsed(0), diskUsed(0){}
		};
		vector<MachineLoad> machineVec(needMachineAmount);
		uint32_t curMachineId = 0;
		// 按组件分配资源
		while (!leftServiceVec.empty())
		{
			// 找到亲缘度最高的组件
			double affinity = 0;
			string serviceName = 0;
			for (auto it = leftServiceVec.begin(); it != leftServiceVec.end(); ++it)
			{
				localdata::CompStress & compStress = it->second;
				vector<double> vec1 = { cpuAffinity, diskAffinity };
				vector<double> vec2 = { compStress.cpuPerLamda, compStress.diskPerLamda };
				double curAffinity = CalAffinity(vec1, vec2);
				if (curAffinity > affinity)
				{
					affinity = curAffinity;
					serviceName = it->first;
				}
			}

			// 对该组件进行资源分配
			localdata::CompStress & foundService = leftServiceVec[serviceName];
			double lamdaCpu = cpuOpt / foundService.cpuPerLamda;
			double lamdaDisk = diskOpt / foundService.diskPerLamda;
			double lamdaTime = 1000 / foundService.processTime;
			double lamdaOffer = min(min(lamdaCpu, lamdaDisk), lamdaTime); // 以较小值作为能提供的lamda值
						
			// 保存分配表
			if (lamdaOffer >= serviceLamda[serviceName])
			{
				lamdaOffer = serviceLamda[serviceName];
				ScheduleItem item;
				item.name = serviceName;
				item.lamda = lamdaOffer;
				item.machineId = curMachineId;
				rstVec.push_back(item);
				leftServiceVec.erase(serviceName);
			}
			else
			{
				ScheduleItem item;
				item.name = serviceName;
				item.lamda = lamdaOffer;
				item.machineId = curMachineId;
				rstVec.push_back(item);
				serviceLamda[serviceName] -= lamdaOffer;
			}

			// 更新节点资源占用，资源不足时更换下一个节点
			double cpuOffer = lamdaOffer * foundService.cpuPerLamda;
			double diskOffer = lamdaOffer * foundService.diskPerLamda;
			machineVec[curMachineId].cpuUsed += cpuOffer;
			machineVec[curMachineId].diskUsed += diskOffer;

			static const double relax = 0.1; // 资源分配的松弛量，避免强行分配资源
			double cpuRelax = cpuOpt * relax;
			double diskRelax = diskOpt * relax;
			if (cpuAffinity >= MAX_UTILIZATION - cpuRelax || diskAffinity >= MAX_UTILIZATION - diskRelax)
			{
				// 该节点资源用完，换下一个节点。
				curMachineId = (curMachineId + 1) % needMachineAmount;
				MachineLoad & curMachine = machineVec[curMachineId];
				if (curMachine.cpuUsed >= cpuOpt || curMachine.diskUsed >= diskOpt)
				{
					// 如果新节点的负载已经超过了最优负载，那直接使用当前负载作为计算亲缘性的负载
					cpuAffinity = curMachine.cpuUsed;
					diskAffinity = curMachine.diskUsed;
				}
				else
				{
					cpuAffinity = MAX_UTILIZATION - cpuOpt + curMachine.cpuUsed;
					diskAffinity = MAX_UTILIZATION - diskOpt + curMachine.diskUsed;
				}

			}
			else
			{
				// 节点资源未用完时，更新用于计算亲缘度的负载
				cpuAffinity += cpuOffer;
				diskAffinity += diskOffer;
			}
		}
		

		// 计算各类组件在哪启动，流量占比, todo

	}
	// 如果不够用，按照流量价值进行路由表计算。
	else
	{

	}
	
	return -1;
}

