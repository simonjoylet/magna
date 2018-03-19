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
int ReadStressFile()
{
	string fileName = "Comp_1_223.3.87.60.stress";
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
	for (size_t i = 0; i < reqLogCount; ++i)
	{
		ReqLog log;
		fread(&log, sizeof(ReqLog), 1, stressFile);
		reqLogList.push_back(log);
	}
	fclose(stressFile);
}

int main()
{
	// 测试读取压测数据文件
	ReadStressFile();


	vector<string> services = { "Comp_2"};
	vector<uint32_t> weights = { 1, 2, 3 };
	const char * filePathTemplate = "./Comp_2_%d.dat";
	const int period = 10;
	for (int i = 1; i <= 15; ++i)
	{
		char filePath[32] = {};
		int lamda = 10 * i;
		sprintf(filePath, filePathTemplate, 10 * i);
		GenerateTraffic(lamda, lamda * period, services, weights, filePath);
	}
	
	
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