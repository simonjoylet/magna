#ifndef NODE_DATA_H
#define NODE_DATA_H

#include <string>
#include <set>
#include <mutex>
#include <list>
using std::string;
using std::set;
using std::list;

#include <google/protobuf/stubs/common.h>
using google::protobuf::int32;

struct NodeLoad 
{
	double cpuLoad;
	double diskLoad;
	NodeLoad() :cpuLoad(0), diskLoad(0){}
	NodeLoad(double _cpuLoad, double _diskLoad)
	{
		cpuLoad = _cpuLoad;
		diskLoad = _diskLoad;
	}
};

class NodeData
{
public:
	static NodeData * GetInstance();

	int32 Init(string ip, int32 port);

	// 系统所有的Node
	set<string/*ip*/> m_allNodes;

	// 本地IP
	string m_ip;

	int32 m_port;

	uint16_t GetNewCompPort();

	void lock(){ m_mutex.lock(); }
	void unlock(){ m_mutex.unlock(); }
	void UpdateLoadData(double cpuLoad, double diskLoad);
	void GetLoadData(double & cpuLoad, double & diskLoad, int round);

private:
	static NodeData * m_instance;
	uint16_t m_compPortCount;
	std::mutex m_mutex;
	list<NodeLoad> m_loadList;
	// not allowed
	NodeData() : m_port(0), m_compPortCount(20000){}
	~NodeData(){}
};

#endif//NODE_DATA_H
