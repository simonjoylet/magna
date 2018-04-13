#include <iostream>
#include <random>
#include <time.h>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <set>
using namespace std;

// 设置关键参数
const string dataFile = "test.data";
const uint32_t n = 20;				//组件数量
const uint32_t m = 5;				//资源纬度
const uint32_t l = 4;				//最大分组规模
// const uint32_t n = 3;				//组件数量
// const uint32_t m = 2;				//资源纬度
// const uint32_t l = 3;				//最大分组规模
const uint32_t popSize = 50;		//种群大小
const double crossRate = 0.6;		//交叉率
const double mutateRate = 0.3;		//变异率
const double surviveRate = 0.3;		//生存比例

vector<vector<double>> data;

void InitRand()
{
	time_t ts;
	time(&ts);
	srand(ts);
}
double RandDouble()
{
	const uint32_t RAND_RANGE = 10000;
	return 1.0 * (rand() % RAND_RANGE) / RAND_RANGE;
}

int GenereteTestData(uint32_t n, uint32_t m, string fileName)
{
	// 生成流量比例
	vector<uint32_t> randVec;
	for (uint32_t i = 0; i < n; ++i)
	{
		randVec.push_back(rand() % 100);
	}
	double sum = accumulate(randVec.begin(), randVec.end(), 0);

	vector<double> flowVec(n);// 每个组件的流量比例
	for (uint32_t i = 0; i < n; ++i)
	{
		flowVec[i] = 100.0 * randVec[i] / sum;
	}

	// 生成测试数据并写入文件
	FILE * f = fopen(fileName.c_str(), "wb");
	fwrite(&n, sizeof(n), 1, f);
	fwrite(&m, sizeof(m), 1, f);
	for (uint32_t i = 0; i < n; ++i)
	{

		for (uint32_t j = 0; j < m; ++j)
		{
			double tmp = RandDouble(); // 生成[0,1]之间的随机数
			tmp *= flowVec[i];// 乘上流量比例
			fwrite(&tmp, sizeof(tmp), 1, f);
		}
	}
	fclose(f);

	return 0;
}

int ReadData(string fileName, vector<vector<double>> & data)
{
	FILE * f = fopen(fileName.c_str(), "rb");
	uint32_t n = 0, m = 0;
	fread(&n, sizeof(n), 1, f);
	fread(&m, sizeof(m), 1, f);
	for (uint32_t i = 0; i < n; ++i)
	{
		vector<double> resVec;
		for (uint32_t j = 0; j < m; ++j)
		{
			double tmp = 0;
			fread(&tmp, sizeof(tmp), 1, f);
			resVec.push_back(tmp);
		}
		data.push_back(resVec);
	}
	fclose(f);
	return 0;
}

typedef vector<uint32_t> Chrom;
typedef vector<vector<uint32_t>> Entity;

void InitEntity(Entity & e)
{
	e.resize(n);
	for (uint32_t i = 0; i < n; ++i)
	{
		e[i].resize(l);
	}
}

void InitPop(vector<Entity> & pop)
{
	for (uint32_t i = 0; i < popSize; ++i)
	{
		Entity tmpEntity;
		InitEntity(tmpEntity);
		for (uint32_t nIndex = 0; nIndex < n;)
		{
			uint32_t chromIndex = rand() % n;
			uint32_t geneIndex = rand() % l;
			if (tmpEntity[chromIndex][geneIndex] == 0)
			{
				tmpEntity[chromIndex][geneIndex] = nIndex + 1;
				++nIndex;
			}
		}
		pop.push_back(tmpEntity);
	}
}

bool testEntity(const Entity & tmpEntity)
{
	vector<uint32_t> test;
	for (uint32_t chromIndex = 0; chromIndex < n; ++chromIndex)
	{
		for (uint32_t geneIndex = 0; geneIndex < l; ++geneIndex)
		{
			test.push_back(tmpEntity[chromIndex][geneIndex]);
		}
	}
	sort(test.begin(), test.end());
	reverse(test.begin(), test.end());
	bool rst = accumulate(test.begin(), test.end(), 0) == (1 + n)*n / 2;
	return rst;
}

double GetCod(const vector<double> & resVec) //计算离散系数
{
	double sum = 0;
	for (uint32_t i = 0; i < m; ++i)
	{
		sum += resVec[i];
	}
	// 和为0说明数组为空
	if (sum == 0)
	{
		return 0;
	}

	double average = sum / m;
	sum = 0;
	for (uint32_t i = 0; i < m; ++i)
	{
		sum += (resVec[i] - average) * (resVec[i] - average);
	}
	double sigma = sqrt(sum / m);// 标准差
	double cod = sigma / average;
	return cod;
}

double GetBalance(const Entity & e)
{
	// 构造结果数据，即每条染色体的资源数组
	vector<vector<double>> rstData(n);
	for (uint32_t i = 0; i < n; ++i)
	{
		const Chrom & tmpChrom = e[i];

		// 一条染色体即为一个组件分组，计算累加的资源数组
		vector<double> & sumResVec = rstData[i];
		sumResVec.resize(m);
		for (uint32_t j = 0; j < l; ++j)
		{
			uint32_t index = tmpChrom[j];
			if (index == 0)
			{
				continue;
			}
			else
			{
				index -= 1; // 找到组件编号
			}
			const vector<double> & resVec = data[index];
			for (uint32_t mIndex = 0; mIndex < m; ++mIndex)
			{
				sumResVec[mIndex] += resVec[mIndex];
			}
		}
	}

	// 按照加权平均，计算个体的均衡度
	double sumWeight = 0;// 权重
	double sumCod = 0;// 离散系数
	for (uint32_t i = 0; i < n; ++i)
	{
		vector<double> & resVec = rstData[i];
		double tmpWeight = *max_element(resVec.begin(), resVec.end());
		double tmpCod = GetCod(resVec);
		sumWeight += tmpWeight;
		sumCod += tmpWeight*tmpCod;
	}
	double balance = sumCod / sumWeight;
	return balance;
}

double PopSelect(vector<Entity> & pop, Entity & bestEntity)
{
	vector<pair<double, Entity>> balanceVec;
	for (uint32_t i = 0; i < popSize; ++i)
	{
		Entity & e = pop[i];
		double balance = GetBalance(e);
		balanceVec.push_back(make_pair(balance, e));
	}
	sort(balanceVec.begin(), balanceVec.end());
	bestEntity = balanceVec.begin()->second;
	double bestBalance = balanceVec.begin()->first;

	vector<Entity> nextPop;
	uint32_t survive = popSize * surviveRate;
	for (uint32_t i = 0; i < survive; ++i)
	{
		nextPop.push_back(balanceVec[i].second);
	}
	
	pop = nextPop;

	return bestBalance;
}

uint32_t GetBestChrom(Entity & e)
{
	uint32_t bestIndex = 0;
	double minBalance = 100000;
	for (uint32_t i = 0; i < n; ++i)
	{
		Chrom & tmpChrom = e[i];
		if (accumulate(tmpChrom.begin(), tmpChrom.end(), 0) == 0)
		{
			continue;
		}
		vector<double> sumResVec(m);
		for (uint32_t j = 0; j < l; ++j)
		{
			uint32_t index = tmpChrom[j];
			if (index == 0)
			{
				continue;
			}
			else
			{
				index -= 1; // 找到组件编号
			}
			const vector<double> & resVec = data[index];
			for (uint32_t mIndex = 0; mIndex < m; ++mIndex)
			{
				sumResVec[mIndex] += resVec[mIndex];
			}
		}
		double tmpBalance = GetCod(sumResVec);
		if (tmpBalance < minBalance)
		{
			minBalance = tmpBalance;
			bestIndex = i;
		}
	}
	return bestIndex;
}

int32_t GetEmptyChrom(Entity & e)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		Chrom & tmpChrom = e[i];
		if (accumulate(tmpChrom.begin(), tmpChrom.end(), 0) == 0)
		{
			return i;
		}
		
	}
	return -1;
}
Entity PopCross(vector<Entity> & pop)
{
	// 把a中最好的一条染色体交换给b // todo：可以考虑更加合理的交换方式
	Entity & a = pop[rand() % pop.size()];
	uint32_t bestIndex = GetBestChrom(a);
	

	Entity b;
	while (1)
	{
		Entity & tmpB = pop[rand() % pop.size()];
		
		if (GetEmptyChrom(tmpB) < 0)
		{
			continue;
		}
		b = tmpB;
		break;
	}
	int32_t emptyIndex = GetEmptyChrom(b);

	// 先移除b中的bestChrom元素
	Chrom & bestChrom = a[bestIndex];
	vector<uint32_t> rmVec;
	for (uint32_t i = 0; i < l; ++i)
	{
		if (bestChrom[i] != 0)
		{
			rmVec.push_back(bestChrom[i]);
		}
	}
	for (uint32_t rmIndex = 0; rmIndex < rmVec.size(); ++rmIndex)
	{
		uint32_t rmTarget = rmVec[rmIndex];
		for (uint32_t i = 0; i < n; ++i)
		{
			Chrom & tmpChrom = b[i];
			for (uint32_t j = 0; j < l; ++j)
			{
				if (tmpChrom[j] == rmTarget)
				{
					tmpChrom[j] = 0;
				}
			}
		}
	}
	
	b[emptyIndex] = bestChrom;
	return b;
}

Entity PopMutate(vector<Entity> & pop)
{
	// 随机找到一个个体
	Entity e = pop[rand() % pop.size()];

	// 随机交换其中一个非0元素的位置
	uint32_t chromIndex = 0;
	uint32_t geneIndex = 0;
	uint32_t element = 0;
	while (1)
	{
		chromIndex = rand() % n;
		geneIndex = rand() % l;
		element = e[chromIndex][geneIndex];
		if (element != 0)
		{
			break;
		}
	}

	uint32_t randChromIndex = rand() % n;
	uint32_t randGeneIndex = rand() % l;

	e[chromIndex][geneIndex] = e[randChromIndex][randGeneIndex];
	e[randChromIndex][randGeneIndex] = element;

	return e;
}

int main()
{
	InitRand();
	//GenereteTestData(n, m, dataFile);

	// 读取实验数据
	ReadData(dataFile, data);
// 	vector<double> comp1 = { 0.5847, 0 };
// 	vector<double> comp2 = { 0.2028, 0.2369 };
// 	vector<double> comp3 = { 0.051, 0 };
// 	data.push_back(comp1);
// 	data.push_back(comp2);
// 	data.push_back(comp3);

	// 构造初始种群
	vector<Entity> pop;
	InitPop(pop);
	
	// 迭代
	Entity bestEntity;
	unsigned int genCount = 0;
	while (++genCount)
	{		
		// 选择
		double bestValue = PopSelect(pop, bestEntity);
		printf("gen: %d, best: %.4f\n", genCount, bestValue);

		// 判断迭代终止条件
		if (genCount > 2000)
		{
			break;
		}

		// 构建下一代种群
		vector<Entity> nextPop = pop;
		while (nextPop.size() < popSize)
		{
			// 交叉
			if (RandDouble() < crossRate)
			{
				nextPop.push_back(PopCross(pop));
			}
			if (nextPop.size() == popSize)
			{
				break;
			}

			// 变异
			if (RandDouble() < mutateRate)
			{
				nextPop.push_back(PopMutate(pop));
			}
		}

		pop = nextPop;
	}

	//getchar();
	return 0;
}

