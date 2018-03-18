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

uint16_t NodeData::GetNewCompPort()
{
	lock();
	uint16_t rst = ++m_compPortCount;
	unlock();
	return rst;
}

void NodeData::UpdateLoadData(double cpuLoad, double diskLoad)
{
	lock();
	m_loadList.push_back(NodeLoad(cpuLoad, diskLoad));
	// 暂时没做定时删除，可能会造成内存持续增长，后续需要写文件。
	unlock();
}

void NodeData::GetLoadData(double & cpuLoad, double & diskLoad, int round)
{
	if (m_loadList.empty())
	{
		return;
	}
	lock();
	if (round > m_loadList.size())
	{
		round = m_loadList.size();
	}
	cpuLoad = 0;
	diskLoad = 0;
	list<NodeLoad>::iterator it = m_loadList.end();
	for (int i = 0; i < round; ++i)
	{
		it--;
	}
	for (; it != m_loadList.end(); it++)
	{
		cpuLoad += it->cpuLoad;
		diskLoad += it->diskLoad;
	}
	cpuLoad = cpuLoad / round;
	diskLoad = diskLoad / round;
	unlock();
}

