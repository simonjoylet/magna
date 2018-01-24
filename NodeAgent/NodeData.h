#ifndef NODE_DATA_H
#define NODE_DATA_H

#include <string>
#include <set>
using std::string;
using std::set;

#include <google/protobuf/stubs/common.h>
using google::protobuf::int32;


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


private:
	static NodeData * m_instance;

	// not allowed
	NodeData(){}
	~NodeData(){}
};

#endif//NODE_DATA_H
