#ifndef TRAFFIC_DEFINE_H
#define TRAFFIC_DEFINE_H

#include <string>
#include <stdint.h>

struct AppReq
{
	uint32_t id; // 请求id
	uint32_t interval; // 访问时间间隔，单位us
	char service[32]; // 服务名称
	uint32_t weight; // 请求权重
	AppReq()
	{
		id = 0;
		interval = 0;
		memset(service, 0, sizeof(service));
		weight = 0;
	}
};

#endif//TRAFFIC_DEFINE_H
