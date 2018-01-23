#include "NodeData.h"

NodeData * NodeData::m_instance = NULL;

NodeData * NodeData::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new NodeData();
		m_instance->m_ip = "223.3.87.0";//GetLocalIP(); // TODO
		m_instance->m_port = 16161;
	}
	return m_instance;
}

