#ifndef HEARTBEAT_THREAD_H
#define HEARTBEAT_THREAD_H
#include <pthread.h>

class HeartbeatThread
{
public:
	static HeartbeatThread * GetInstance();
	int Start();
	int Stop();

	pthread_t m_tid;
	bool m_shouldrun;
private:
	static HeartbeatThread * m_instance;
	HeartbeatThread()
		:m_shouldrun(true), m_tid(0){}

	~HeartbeatThread(){}
};

#endif//HEARTBEAT_THREAD_H
