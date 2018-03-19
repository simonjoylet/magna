#ifndef ADMIN_DATA_H
#define ADMIN_DATA_H

#include <map>
#include <set>
#include <string>
#include <mutex>
#include <stdint.h>
#include <vector>
using namespace std;

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
	RouterItem() : compName(""), ip(""), port(0), percentage(0){}
};

struct StressInfo
{
	uint32_t lamda;
	double cpuLoad;
	double diskLoad;
	uint32_t processTime; // 每条请求的平均处理时间
	uint32_t queueUnitTime; // 队列中单位人数的排队时间，一般比处理时间略长。
	StressInfo() : lamda(0), cpuLoad(0), diskLoad(0), processTime(0), queueUnitTime(0){}
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
	
	// 服务的路由表
	vector<localdata::RouterItem> m_router;

	// 
	map<string/*name*/, vector<localdata::StressInfo>> m_serviceStress;

	void InitServiceTable(vector<localdata::RouterItem> & m_router);
	int32_t UpdateServiceTable(); // TODO

	int32_t ReadStressData(string compName, string filePath);


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
