#ifndef REQ_ANALYTICS_H
#define REQ_ANALYTICS_H
#include <stdint.h>
#include "../TrafficGenerator/TrafficDefine.h"

struct ReqLog 
{
	AppReq req;
	uint64_t begin;
	uint64_t end;
	ReqLog() : begin(0), end(0){}
};

#endif//REQ_ANALYTICS_H
