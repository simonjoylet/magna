#ifndef NODE_DATA_H
#define NODE_DATA_H

#include <string>
#include <set>
using std::string;
using std::set;

class NodeData
{
public:
	static NodeData * GetInstance();

	// 系统所有的Node
	set<string/*ip*/> m_allNodes;

	// 本地IP
	string m_ip;


private:
	static NodeData * m_instance;

	// not allowed
	NodeData(){}
	~NodeData(){}
};

#endif//NODE_DATA_H
