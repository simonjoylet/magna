#ifndef RESOURCE_COST_H
#define RESOURCE_COST_H

struct ResourceCost
{
	uint32_t lamda; // 到达强度，qps
	double cpuLoad; // cpu资源利用率，直接取用监测值
	double diskLoad; // 通过RDISK换算，RDISK写入速率上线是200MB/s
};

#endif//RESOURCE_COST_H
