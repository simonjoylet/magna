#include "HeatbeatThread.h"

HeartbeatThread * HeartbeatThread::m_instance = NULL;

void * NodeHeatbeatFunc(void *);

HeartbeatThread * HeartbeatThread::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new HeartbeatThread;
	}
	return m_instance;
}

int HeartbeatThread::Start()
{
	return pthread_create(&m_tid, NULL, NodeHeatbeatFunc, NULL);
}

int HeartbeatThread::Stop()
{
	m_shouldrun = false;
}

#include "../AdminServer/admin_client.h"

extern AdminClient * g_adminProxy;

#include <unistd.h>

void * NodeHeatbeatFunc(void * param)
{
	HeartbeatThread * hbThread = HeartbeatThread::GetInstance();
	NodeData * nodeData = NodeData::GetInstance();
	printf("\nNode heatbeat thread start running...\n");
	magna::InetAddress addr;
	magna::NodeStatus load;
	magna::NodeHeartbeatRequest req;
	magna::NodeHeartbeatResponse rsp;
	while (hbThread->m_shouldrun)
	{
		sleep(2);
		req.set_allocated_addr(&addr);
		req.set_allocated_load(&load);
		int ret = g_adminProxy->NodeHeatbeat(req, &rsp);
		if (0 == ret)
		{
			if (rsp.ack())
			{
				// 更新节点数据
			}
			else
			{
				printf("\nNode heatbeat response ERROR. msg:%s\n", rsp.msg().c_str());
			}
		}
	}
	printf("\nNode heatbeat thread stopped...\n");
	return NULL;
}