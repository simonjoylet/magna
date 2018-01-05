#include "NodeData.h"

NodeData * NodeData::m_instance = NULL;

NodeData * NodeData::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new NodeData();
		m_instance->m_ip = GetLocalIP();
	}
	return m_instance;
}

