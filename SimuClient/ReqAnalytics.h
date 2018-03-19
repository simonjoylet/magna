#ifndef REQ_ANALYTICS_H
#define REQ_ANALYTICS_H
#include <stdint.h>
#include "../TrafficGenerator/TrafficDefine.h"
#include <string.h>
using std::string;

struct ReqLog 
{
	uint32_t reqId;
	char serviceName[16];
	uint32_t clientWeight;
	uint64_t localBegin;
	uint64_t localEnd;
	uint32_t compLamda;
// 	char sendUTC[32];
// 	char arriveUTC[32];
	uint32_t queueLength;
	uint32_t queueTime;
	uint32_t processTime;
	ReqLog() 
		: reqId(0), clientWeight(0), localBegin(0), localEnd(0),
		compLamda(0), queueLength(0), queueTime(0), processTime(0)
	{
		memset(serviceName, 0, sizeof(serviceName));
	}
};

struct LoadLog
{
	uint32_t sendId; // 当前正在发送的id
	uint32_t sendLamda; // 当前的发送lamda
	double cpuLoad;
	double diskLoad;
	LoadLog() : sendId(0), sendLamda(0), cpuLoad(0), diskLoad(0){}
};

#endif//REQ_ANALYTICS_H
