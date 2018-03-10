#include <iostream>
#include <random>
#include <iomanip>
#include <string>
#include <map>
using namespace std;
/*
这个模拟器的作用是按照指定要求生成请求序列，并存储于文件中，文件格式如下：
 请求id | 时间间隔us | 服务名称 | 用户权重
*/



int main()
{
	std::random_device rd;
	std::default_random_engine gen(rd());

	// 若网站访问平均每秒1000次，
	// 则到下次访问前要多少毫秒的时间？
	std::exponential_distribution<> d(1000);

	std::map<int, int> hist;
	for (int n = 0; n < 100; ++n) {
		cout << d(gen) * 1000 << endl;
	}

	return 0;
}