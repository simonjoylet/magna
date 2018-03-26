#include <iostream>
#include <random>
#include <stdio.h>
#include "TrafficDefine.h"
using namespace std;
/*
这个模拟器的作用是按照指定要求生成请求序列，并存储于文件中，文件格式如下：
 请求id | 时间间隔us | 服务名称 | 用户权重
*/

static const unsigned MILLION = 1000000;

static vector<AppReq> records;

int GenerateTraffic(
	uint32_t lamda, // 到达强度
	uint32_t n, // 生成记录的条数
	vector<string> serviceVec, // 服务名称数组，生成纪录时均匀随机
	vector<unsigned> weightVec, // 权重数组，生成记录时均匀随机
	string filePath // 流量文件存储路径
	)
{
	if (lamda <= 0 || n <= 0 || serviceVec.empty() || weightVec.empty() || filePath.empty())
	{
		return -1;
	}

	FILE * f = fopen(filePath.c_str(), "wb");
	if (f == NULL) return -2;

	std::random_device rd;
	std::default_random_engine gen(rd());
	std::exponential_distribution<> d(lamda);
	srand(d(gen) * MILLION);

	fwrite(&n, sizeof(n), 1, f);
	static uint32_t idCount = 0;
	for (uint32_t i = 0; i < n; ++i) 
	{
		AppReq req;
		req.id = ++idCount;
		req.interval = d(gen) * MILLION;
		strcpy(req.service, serviceVec[rand() % serviceVec.size()].c_str());
		req.weight = weightVec[rand() % weightVec.size()];
		
		fwrite(&req, sizeof(req), 1, f);
		records.push_back(req);
	}

	fclose(f);
	return 0;
}
#include "../SimuClient/ReqAnalytics.h"
const vector<pair<uint32_t, uint32_t>> g_satisfactionMap = { { 500, 4 }, { 1000, 2 } };
double GetSatisfaction(double totalTime, uint32_t weight)
{
	if (totalTime <= g_satisfactionMap[0].first)
	{
		return g_satisfactionMap[0].second * weight;
	}
	else if (totalTime <= g_satisfactionMap[1].first)
	{
		return g_satisfactionMap[1].second * weight;
	}
	return 0;
}

int ReadStressFile(string fileName)
{
	FILE * stressFile = fopen(fileName.c_str(), "rb");
	if (stressFile == NULL)
	{
		printf("File open failed, path: %s\n", fileName.c_str());
		return -1;
	}

	// 保存负载数据
	vector<LoadLog> loadLogList;
	uint32_t loadLogCount = 0; 
	fread(&loadLogCount, sizeof(loadLogCount), 1, stressFile);
	for (size_t i = 0; i < loadLogCount; i++)
	{
		LoadLog log;
		fread(&log, sizeof(LoadLog), 1, stressFile);
		loadLogList.push_back(log);
	}


	// 保存请求数据
	vector<ReqLog> reqLogList;
	uint32_t reqLogCount = 0;
	fread(&reqLogCount, sizeof(reqLogCount), 1, stressFile);
	// 分析第一阶段

	for (uint32_t phaseIndex = 0; phaseIndex < 3; ++phaseIndex)
	{
		vector<vector<ReqLog>> comps(3);
		for (size_t i = 0; i < (phaseIndex + 1) * 1000; ++i)
		{
			ReqLog log;
			fread(&log, sizeof(ReqLog), 1, stressFile);
			reqLogList.push_back(log);
			if (strcmp(log.serviceName, "Comp_1") == 0)
			{
				comps[0].push_back(log);
			}
			if (strcmp(log.serviceName, "Comp_2") == 0)
			{
				comps[1].push_back(log);
			}
			if (strcmp(log.serviceName, "Comp_3") == 0)
			{
				comps[2].push_back(log);
			}
		}

		for (uint32_t compIndex = 0; compIndex < comps.size(); ++compIndex)
		{
			vector<ReqLog> & comp = comps[compIndex];
			double answerTime = 0;
			double queueTime = 0; 
			double dropCount = 0;
			uint32_t satisfaction = 0;
			for (uint32_t i = 0; i < comp.size(); ++i)
			{
				if (comp[i].localEnd == 0)
				{ 
					++dropCount;
					continue; 
				}
				uint32_t totalTime = (comp[i].localEnd - comp[i].localBegin);
				answerTime += totalTime;
				queueTime += comp[i].queueTime;
				satisfaction += GetSatisfaction(totalTime, comp[i].clientWeight);
			}
			printf("phase%d comp%d ---- answerTime: %.2f, queueTime: %.2f, drop rate: %.4f, satisfaction: %d\n", 
				phaseIndex+1, compIndex + 1, answerTime / comp.size(), queueTime / comp.size(), dropCount / comp.size(), satisfaction);
		}
	}

	fclose(stressFile);
}

int main()
{
	// 测试读取压测数据文件
	cout << "-----------tradition & fcfs-------------" << endl;
	ReadStressFile("simu_tradition_fcfs.stress");

	cout << "-----------tradition & cuttail-------------" << endl;
	ReadStressFile("simu_tradition_cuttail.stress");

	cout << "-----------tradition & minloss-------------" << endl;
	ReadStressFile("simu_tradition_minloss.stress");

	cout << "-----------tradition & pq-------------" << endl;
	ReadStressFile("simu_tradition_pq.stress");

	cout << "-----------tradition & pqminloss-------------" << endl;
	ReadStressFile("simu_tradition_pqminloss.stress");

	cout << "-----------magna & pqminloss-------------" << endl;
	ReadStressFile("simu_magna.stress");

// 	vector<string> services = { "Comp_1", "Comp_1", "Comp_1", 
// 		"Comp_2", 
// 		"Comp_3", "Comp_3", "Comp_3", "Comp_3", "Comp_3", "Comp_3" };
// 	vector<uint32_t> weights = { 1, 2, 3 };
// 	const char * filePath1 = "./simu1.dat";
// 	const char * filePath2 = "./simu2.dat";
// 	const char * filePath3 = "./simu3.dat";
// 	uint32_t lamda1 = 100;
// 	uint32_t lamda2 = 200;
// 	uint32_t lamda3 = 300;
// 	uint32_t period = 10;
// 	GenerateTraffic(lamda1, lamda1 * period, services, weights, filePath1);
// 	GenerateTraffic(lamda2, lamda2 * period, services, weights, filePath2);
// 	GenerateTraffic(lamda3, lamda3 * period, services, weights, filePath3);





// 	// 压测数据生成代码
// 	vector<string> services = { "Comp_2"};
// 	vector<uint32_t> weights = { 1, 2, 3 };
// 	const char * filePathTemplate = "./Comp_2_%d.dat";
// 	const int period = 10;
// 	for (int i = 1; i <= 15; ++i)
// 	{
// 		char filePath[32] = {};
// 		int lamda = 10 * i;
// 		sprintf(filePath, filePathTemplate, 10 * i);
// 		GenerateTraffic(lamda, lamda * period, services, weights, filePath);
// 	}
	
	
	// 测试读入的数据和写入的数据是否吻合。
	// testRead(filePath);
	getchar();
	return 0;
}

void testRead(string filePath)
{
	FILE * f = fopen(filePath.c_str(), "rb");
	uint32_t n = 0;
	fread(&n, sizeof(n), 1, f);
	vector<AppReq> records;
	for (int i = 0; i < n; ++i)
	{
		AppReq req;
		fread(&req, sizeof(req), 1, f);
		records.push_back(req);
	}
	fclose(f);
}