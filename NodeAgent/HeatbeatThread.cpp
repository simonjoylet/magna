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
	printf("\nNode heatbeat thread start running...\n");
	while (hbThread->m_shouldrun)
	{
		sleep(2);
		g_adminProxy->NodeHeatbeat();
	}
	printf("\nNode heatbeat thread stopped...\n");
	return NULL;
}