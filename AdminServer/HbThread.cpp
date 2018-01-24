#include "HbThread.h"
#include "AdminData.h"
#include <stdio.h>
#include <unistd.h>
#include "phxrpc/rpc.h"
#include <mutex>

HbThread * HbThread::m_instance = NULL;

void * AdminHeatbeatFunc(void *);

HbThread * HbThread::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new HbThread;
	}
	return m_instance;
}

int HbThread::Start()
{
	return pthread_create(&m_tid, NULL, AdminHeatbeatFunc, NULL);
}

int HbThread::Stop()
{
	m_shouldrun = false;
	return 0;
}


extern phxrpc::ClientConfig global_nodeclient_config_;
extern std::mutex * g_nodelist_mutex;
void * AdminHeatbeatFunc(void * param)
{
	HbThread * hbThread = HbThread::GetInstance();
	AdminData * adminData = AdminData::GetInstance();

	printf("\nAdmin HbThread start running...\n");

	// 时钟轮盘
	while (hbThread->m_shouldrun)
	{
		static int32 count = 0;
		sleep(1);
		++count;

		// 每2秒心跳计数加1，移除心跳超时的节点。
		if (count % 2 == 0)
		{
			g_nodelist_mutex->lock();
			for (auto it = adminData->m_nodeList.begin(); it != adminData->m_nodeList.end(); ++it)
			{
				it->second.heatbeat += 1;
				if (it->second.heatbeat > 5)
				{
					phxrpc::Endpoint_t ep;
					snprintf(ep.ip, sizeof(ep.ip), "%s", it->second.addr.ip.c_str());
					ep.port = it->second.addr.port;
					global_nodeclient_config_.Remove(ep);
					adminData->m_nodeList.erase(it++);
					printf("\nnode: %s: %d removed\n", ep.ip, ep.port);
					continue;
				}
			}
			g_nodelist_mutex->unlock();
		}
		
	}
	printf("\nAdmin HbThread stopped...\n");
	return NULL;
}