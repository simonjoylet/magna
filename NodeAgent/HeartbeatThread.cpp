#include "HeartbeatThread.h"
#include "NodeData.h"

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
	return 0;
}

#include "../AdminServer/admin_client.h"

extern AdminClient * g_adminProxy;

#include <unistd.h>

void * NodeHeatbeatFunc(void * param)
{
	HeartbeatThread * hbThread = HeartbeatThread::GetInstance();
	NodeData * nodeData = NodeData::GetInstance();
	printf("\nNode heatbeat thread start running...\n");
	magna::NodeHeartbeatRequest req;
	magna::NodeHeartbeatResponse rsp;

	// test info // TODO
	req.mutable_addr()->set_ip(nodeData->m_ip);
	req.mutable_addr()->set_port(nodeData->m_port);
	while (hbThread->m_shouldrun)
	{
		sleep(2);
		req.mutable_load()->set_cpu(0.1);
		int ret = g_adminProxy->NodeHeatbeat(req, &rsp);
		if (0 == ret)
		{
			if (rsp.ack())
			{
				printf("\nNode heatbeat ack received\n");
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