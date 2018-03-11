#include <iostream>
#include <random>
#include <stdio.h>
#include <time.h>
using namespace std;

int main()
{
	time_t t;
	time(&t);
	srand(t);
	int lamda = 1000;
	for (int i = 0; i < 100; ++i)
	{
		double rst = (log(lamda) - log(rand() % lamda)) / lamda;
		cout << rst * 1000 << endl;
	}
	
	getchar();
}