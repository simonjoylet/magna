#include "AdminData.h"
#include "../SimuClient/ReqAnalytics.h"
#include <math.h>
#include "../NodeAgent/node_client.h"

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

void StartOneCompPerNode(vector<string> compVec)
{
	AdminData * ad = AdminData::GetInstance();
	uint32_t compIndex = 0;
	// 每个机器上分别启动一个组件
	ad->lock();
	for (auto it = ad->m_nodeList.begin(); it != ad->m_nodeList.end() && compIndex < compVec.size(); ++it, ++compIndex)
	{
		string compName = compVec[compIndex];
		magna::StartComponentResponse rsp = ad->StartComp(it->second.addr, compName);
		localdata::RouterItem item;
		item.compName = compName;
		item.percentage = 1;
		if (rsp.success())
		{
			item.ip = rsp.ip();
			item.port = rsp.port();
			item.pid = rsp.pid();
			ad->m_router.push_back(item);
		}
		else
		{
			printf("start component failed\n");
		}
	}
	ad->unlock();
}


void StartCompGroupPerNode(vector<string> compVec)
{
	AdminData * ad = AdminData::GetInstance();
	// 每个机器上分别启动一个组件
	ad->lock();
	for (auto it = ad->m_nodeList.begin(); it != ad->m_nodeList.end(); ++it)
	{
		for (uint32_t compIndex = 0; compIndex < compVec.size(); ++compIndex)
		{
			string compName = compVec[compIndex];
			magna::StartComponentResponse rsp = ad->StartComp(it->second.addr, compName);
			localdata::RouterItem item;
			item.compName = compName;
			item.percentage = 1 / ad->m_nodeList.size();
			if (rsp.success())
			{
				item.ip = rsp.ip();
				item.port = rsp.port();
				item.pid = rsp.pid();
				ad->m_router.push_back(item);
			}
			else
			{
				printf("start component failed\n");
			}
		}		
	}
	ad->unlock();
}

void AdminData::InitServiceTable()
{
	vector<string> compVec = {"Comp_1", "Comp_2", "Comp_3"};
	StartCompGroupPerNode(compVec);
	//StartOneCompPerNode(compVec);//测试传统模式
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

magna::StartComponentResponse AdminData::StartComp(localdata::InetAddress & nodeAddr, string compName)
{
	map<string, string> nameToPath;
	nameToPath["Comp_1"] = "comp1";
	nameToPath["Comp_2"] = "comp2";
	nameToPath["Comp_3"] = "comp3";
	magna::StartComponentRequest req;
	magna::StartComponentResponse rsp;
	req.set_path(nameToPath[compName]);

	phxrpc::Endpoint_t ep;
	strcpy(ep.ip, nodeAddr.ip.c_str());
	ep.port = nodeAddr.port;
	NodeClient nc;
	int ret = nc.StartComponent(ep, req, &rsp);
	if (ret == 0)
	{
		printf("StartComponent\n\nresp: {\n%s}\n", rsp.DebugString().c_str());
	}
	return rsp;
}

// 计算单位CPU的其他资源占用，求差的平方，数值越大，说明资源互补性越高，亲缘度越高。
double CalAffinity(vector<double> & vec1, vector<double> &vec2)
{
	// 避免被除数为0
	if (vec1[0] == 0)
	{
		vec1[0] = 0.01;
	}
	if (vec2[0] == 0)
	{
		vec2[0] = 0.01;
	}
	// 以数组中第一个值为单位量。
	for (uint32_t i = 1; i < vec1.size(); ++i)
	{
		vec1[i] /= vec1[0];
	}
	for (uint32_t i = 1; i < vec2.size(); ++i)
	{
		vec2[i] /= vec2[0];
	}
	double affinity = 0; // 差的平方
	for (uint32_t i = 0; i < vec1.size(); ++i)
	{
		affinity += (vec1[i] - vec2[i])*(vec1[i] - vec2[i]);
	}
	return affinity;
}
/*
int32_t AdminData::UpdateServiceTable()
{
	if (m_serviceList.empty())
	{
		return -1;
	}
	// 按名字累计每种服务的到达率
	map<string, uint32_t> serviceLamda;
	m_mutex.lock();
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
	m_mutex.unlock();

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
	if (needMachineAmount == 0)
	{
		// 此时仿真尚未开始
		return -2;
	}

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
		map<string, uint32_t> leftServiceLamda = serviceLamda;

		// 构建机器数组
		struct MachineLoad 
		{
			localdata::InetAddress addr;
			double cpuUsed;
			double diskUsed;
			MachineLoad() : cpuUsed(0), diskUsed(0){}
		};
		vector<MachineLoad> machineVec(needMachineAmount);
		uint32_t curMachineId = 0;
		for (auto it = m_nodeList.begin(); it != m_nodeList.end() && curMachineId < machineVec.size(); ++it, ++curMachineId)
		{
			machineVec[curMachineId].addr = it->second.addr;
		}
		curMachineId = 0;
		// 按组件分配资源
		while (!leftServiceVec.empty())
		{
			// 找到亲缘度最高的组件
			double affinity = 0;
			string serviceName = "";
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
			if (lamdaOffer >= leftServiceLamda[serviceName])
			{
				lamdaOffer = leftServiceLamda[serviceName];
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
				leftServiceLamda[serviceName] -= lamdaOffer;
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
		
		// 计算各类组件在哪启动，流量占比
		vector<localdata::RouterItem> rstRouter;

		for (uint32_t i = 0; i < rstVec.size(); ++i)
		{
			localdata::RouterItem item;
			item.compName = rstVec[i].name;
			if (serviceLamda[rstVec[i].name] == 0)
			{
				item.percentage = 1;
			}
			else
			{
				item.percentage = rstVec[i].lamda / serviceLamda[rstVec[i].name];
			}
			

			// 启动组件，拿到IP和端口
			MachineLoad & machine = machineVec[rstVec[i].machineId];
			magna::StartComponentResponse rsp = StartComp(machine.addr, rstVec[i].name);
			if (rsp.success())
			{
				item.ip = rsp.ip();
				item.port = rsp.port();
				item.pid = rsp.pid();
				m_router.push_back(item);
			}
			else
			{
				printf("start component failed\n");
			}
		}
		m_mutex.lock();
		m_router = rstRouter;
		m_mutex.unlock();


		return 0;
	}
	// 如果不够用，按照流量价值进行路由表计算。
	else
	{
		// TODO
	}
	
	return -1;
}
*/

// 按名字累计每种服务的到达率
void GetServiceLamda(map<string, uint32_t> & serviceLamda)
{
	AdminData * ad = AdminData::GetInstance();
	ad->lock();
	for (auto it = ad->m_serviceList.begin(); it != ad->m_serviceList.end(); it++)
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
	ad->unlock();
}

// 计算需要的总资源
void GetTotalNeedResource(map<string, uint32_t> & serviceLamda, double & cpuNeed, double & diskNeed)
{
	for (auto it = serviceLamda.begin(); it != serviceLamda.end(); it++)
	{
		string serviceName = it->first;
		localdata::CompStress & compStress = AdminData::GetInstance()->m_stressMap[serviceName];
		uint32_t curLamda = it->second;
		cpuNeed += curLamda * compStress.cpuPerLamda;
		diskNeed += curLamda * compStress.diskPerLamda;
	}
}

void GetCurrentWorkingNodes(map<string, localdata::NodeInfo> & curWorkingNodes)
{
	AdminData * ad = AdminData::GetInstance();
	for (auto it = ad->m_router.begin(); it != ad->m_router.end(); it++)
	{
		localdata::RouterItem & item = *it;
		curWorkingNodes[item.ip] = ad->m_nodeList[item.ip];
	}
}

int32_t AdminData::UpdateServiceTable()
{
	//return 0;// 测试传统模式
	if (m_serviceList.empty())
	{
		return -1;
	}
	// 按名字累计每种服务的到达率
	map<string, uint32_t> serviceLamda;
	GetServiceLamda(serviceLamda);
	for (auto it = serviceLamda.begin(); it != serviceLamda.end(); ++it)
	{
		printf("%s lamda: %d\n", it->first.c_str(), it->second);
	}

	// 计算所需的总资源以及机器台数。
	double cpuNeed = 0, diskNeed = 0;
	GetTotalNeedResource(serviceLamda, cpuNeed, diskNeed);
	uint32_t needMachineAmount = ceil((cpuNeed > diskNeed ? cpuNeed : diskNeed) / MAX_UTILIZATION);
	printf("cpuNeed: %.2f, diskNeed: %.2f\n", cpuNeed, diskNeed);
	if (needMachineAmount == 0)
	{
		// 此时仿真尚未开始
		printf("needMachineAmount is 0\n");
		return -2;
	}

	// 找到当前工作节点
	map<string, localdata::NodeInfo> curWorkingNodes;
	GetCurrentWorkingNodes(curWorkingNodes);
	
	int32_t ret = 0;
	// 判断是否需要扩容或者缩容
	if (needMachineAmount > curWorkingNodes.size())
	{
		// 此时需要扩容，下面执行扩容操作，即找到一个节点加入当前工作节点
		auto tmpNodeList = m_nodeList;
		for (auto it = curWorkingNodes.begin(); it != curWorkingNodes.end(); ++it)
		{
			tmpNodeList.erase(it->first);
		}
		uint32_t expandNumber = needMachineAmount - curWorkingNodes.size();
		auto nodeIt = tmpNodeList.begin();
		for (uint32_t i = 0; i < expandNumber && nodeIt != tmpNodeList.end(); ++i, ++nodeIt)
		{
			curWorkingNodes[nodeIt->first] = nodeIt->second;
		}		
	}
	else if (needMachineAmount < curWorkingNodes.size())
	{
		// 此时需要缩容，随便移除一个节点，反正均分流量
		uint32_t shrinkNumber = curWorkingNodes.size() - needMachineAmount;
		auto eraseIt = curWorkingNodes.begin();
		for (uint32_t i = 0; i < shrinkNumber; ++i)
		{
			curWorkingNodes.erase(eraseIt++);
		}

	}
	// 选定工作节点之后，要做的就是内部调整流量
	vector<localdata::RouterItem> curRouter;
	for (auto it = m_serviceList.begin(); it != m_serviceList.end(); ++it)
	{
		// 找到在当前工作节点中的组件，均分流量，加入路由表
		string curIp = it->second.addr.ip;
		if (curWorkingNodes.find(curIp) == curWorkingNodes.end())
		{
			continue;
		}
		localdata::RouterItem item;
		item.compName = it->second.name;
		item.ip = it->second.addr.ip;
		item.port = it->second.addr.port;
		item.percentage = 1.0 / needMachineAmount;
		curRouter.push_back(item);
	}

	lock();
	m_router = curRouter;
	unlock();

	printf("working nodes count: %d\n", curWorkingNodes.size());
	uint32_t printIndex = 0;
	for (auto it = curWorkingNodes.begin(); it != curWorkingNodes.end(); ++it)
	{
		printf("working node %d: %s\n", ++printIndex, it->first.c_str());
	}
	printf("\n");
	return ret;
}