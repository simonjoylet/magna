#ifndef HEARTBEAT_THREAD_H
#define HEARTBEAT_THREAD_H
#include <pthread.h>

class HeartbeatThread
{
public:
	static HeartbeatThread * GetInstance();
	int Start();
	int Stop();

	pthread_t m_tid = 0;
	bool m_shouldrun = true;
private:
	static HeartbeatThread * m_instance;
	HeartbeatThread(){}
	~HeartbeatThread(){}
};

#endif//HEARTBEAT_THREAD_H
