#ifndef HB_THREAD_H
#define HB_THREAD_H

#include <pthread.h>

class HbThread
{
public:
	static HbThread * GetInstance();
	int Start();
	int Stop();

	pthread_t m_tid = 0;
	bool m_shouldrun = true;
private:
	static HbThread * m_instance;
	HbThread(){}
	~HbThread(){}
};


#endif//HB_THREAD_H
