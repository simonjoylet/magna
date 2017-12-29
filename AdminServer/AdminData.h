#ifndef ADMIN_DATA_H
#define ADMIN_DATA_H
#include <map>
#include <set>
#include <string>

using std::set;
using std::map;
using std::string;

namespace localdata
{ 
struct InetAddress
{
	string ip;
	int32 port;
	InetAddress(string ip, int32 port);
	bool operator<(const InetAddress &);
};

struct NodeAddr
{
	InetAddress addr;
	int mips;			// cpu处理能力，百万指令每秒
	int32 heatbeat;		// 心跳计数，默认为0
};


struct NodeStatus
{
	float cpuload;
	map<string, int32> netrtt;
};

struct ServiceAddr
{
	int32 id;
	string name;
	InetAddress addr;
	int32 heatbeat;		// 心跳计数，默认为0
};
}

class AdminData
{
public:
	static AdminData * GetInstance();

private:
	// 注册的节点
	map<string/*ip*/, localdata::NodeAddr> m_nodeList;

	// 节点的状态
	map<string/*ip*/, localdata::NodeStatus> m_nodeStatus;

	// 注册的服务
	map<int32/*id*/, localdata::ServiceAddr> m_serviceList;
	map<string/*name*/, set<int32> > m_serviceDNS;


private:
	static AdminData * m_instance;

	// not allowed
	AdminData();
	~AdminData();
};

#endif//ADMIN_DATA_H
