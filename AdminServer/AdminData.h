#ifndef ADMIN_DATA_H
#define ADMIN_DATA_H

#include <map>
#include <set>
#include <string>
#include <mutex>
#include <stdint.h>
#include <vector>
#include "../NodeAgent/node_client.h"
using namespace std;

static const double MAX_UTILIZATION = 0.9; // 最大资源利用率

namespace localdata
{ 
struct InetAddress
{
	string ip;
	uint16_t port;
	InetAddress() :ip(""), port(0){}
	InetAddress(string _ip, uint16_t _port);
	bool operator<(const InetAddress &);
};

struct NodeInfo
{
	InetAddress addr;
	uint32_t type;			// 实例类型
	uint32_t heatbeat;		// 心跳计数，默认为0
	float cpuload;
	float diskload;
	map<string, uint32_t> netrtt;
	NodeInfo():type(0), heatbeat(0), cpuload(0), diskload(0){}
};


struct ServiceInfo
{
	uint32_t id;
	string name;
	InetAddress addr;
	uint32_t heatbeat;		// 心跳计数，默认为0
	uint32_t lamda;
	uint32_t queueLength;
	ServiceInfo() :id(0), heatbeat(0), lamda(0), queueLength(0){}
};

struct RouterItem
{
	string compName;
	string ip;
	uint16_t port;
	double percentage;
	uint16_t pid;
	RouterItem() : compName(""), ip(""), port(0), percentage(0), pid(0){}
};

struct StressInfo
{
	uint32_t lamda;
	double cpuLoad;
	double diskLoad;
	double queueGrowSpeed; // 队列增长速度，个/s
	StressInfo() : lamda(0), cpuLoad(0), diskLoad(0), queueGrowSpeed(0){}
};
struct CompStress
{
	string name; // 组件名称
	double processTime; // 每条请求的平均处理时间
	double cpuPerLamda; // 单位到达强度下的CPU消耗
	double diskPerLamda; // 单位到达强度下的disk消耗
	double cpuBase; // 空载时的CPU消耗
	double diskBase; // 空载时的disk消耗
	vector<StressInfo> stressVec;
	CompStress() :name(""), processTime(0), cpuPerLamda(0),
		diskPerLamda(0), cpuBase(0), diskBase(0){}
};
}

class AdminData
{
public:
	static AdminData * GetInstance();

	// 注册的节点
	map<string/*ip*/, localdata::NodeInfo> m_nodeList;
	
	// 注册的服务
	map<uint32_t/*id*/, localdata::ServiceInfo> m_serviceList;
	vector<uint32_t> m_tobeStopedVec;
	
	// 服务的路由表
	vector<localdata::RouterItem> m_router;

	// 各个组件的压测数据表
	map<string/*name*/, localdata::CompStress> m_stressMap;

	void InitServiceTable();
	int32_t UpdateServiceTable(); // TODO

	int32_t ReadStressData(string compName, string filePath);

	magna::StartComponentResponse StartComp(localdata::InetAddress & nodeAddr, string compName);

	int32_t GetNewServiceId();
	
	void lock() { m_mutex.lock(); }
	void unlock() { m_mutex.unlock(); }

private:
	static AdminData * m_instance;
	uint32_t m_serviceIdCount;
	std::mutex m_mutex;
	// not allowed
	AdminData();
	~AdminData(){}
};

#endif//ADMIN_DATA_H
