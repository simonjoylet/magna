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

	for (uint32_t i = 0; i < n; ++i) 
	{
		AppReq req;
		req.id = i + 1;
		req.interval = d(gen) * MILLION;
		strcpy(req.service, serviceVec[rand() % serviceVec.size()].c_str());
		req.weight = weightVec[rand() % weightVec.size()];
		
		fwrite(&req, sizeof(req), 1, f);
		records.push_back(req);
	}

	fclose(f);
	return 0;
}


int main()
{
	vector<string> services = { "Comp1", "Comp2"};
	vector<uint32_t> weights = { 1, 2, 3 };
	string filePath = "./test.dat";
	GenerateTraffic(1000, 10000, services, weights, filePath);
	
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