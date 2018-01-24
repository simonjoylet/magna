#include "NodeData.h"
#include "phxrpc/file.h"

NodeData * NodeData::m_instance = NULL;

NodeData * NodeData::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new NodeData();
	}
	return m_instance;
}

int32 NodeData::Init(string ip, int32 port)
{
	m_ip = ip;
	m_port = port;
	return 0;
}

