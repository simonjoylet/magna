#include <signal.h>    /* union sigval / struct sigevent */
#include <stdio.h>    /* printf */
#include <string.h>    /* memset */
#include <unistd.h> /* sleep */
#include <time.h>
#include <stdint.h>

#define printf_with_time(format, ...)                                        \
{                                                                            \
    struct timespec spec;                                                    \
    clock_gettime(CLOCK_REALTIME, &spec);                                    \
    printf("[%lu:%lu]%s\n", spec.tv_sec, spec.tv_nsec, format, ##__VA_ARGS__);\
}

volatile uint64_t count = 0;
void timer_notify_cb(union sigval val)
{
	++count;
}
#include <list>
#include <iostream>
using namespace std;
int main(void)
{
	list<int> mylist;
	mylist.push_back(4);
	mylist.push_back(4);
	mylist.push_back(2);
	int ele = 3;
	for (list<int>::iterator it = mylist.begin(); it != mylist.end(); ++it)
	{
		if (*it < ele)
		{
			mylist.insert(it, ele);
			break;
		}
	}
	for (list<int>::iterator it = mylist.begin(); it != mylist.end(); ++it)
	{
		cout << *it << " ";
	}
	
// 	/* Variable Definition */
// 	timer_t id;
// 	struct timespec spec;
// 	struct sigevent ent;
// 	struct itimerspec value;
// 
// 	/* Init */
// 	memset(&ent, 0x00, sizeof(struct sigevent));
// 	memset(&value, 0x00, sizeof(struct itimerspec));
// 
// 	/* create a timer */
// 	ent.sigev_notify = SIGEV_THREAD;
// 	ent.sigev_notify_function = timer_notify_cb;
// 	printf_with_time("create timer");
// 	timer_create(CLOCK_REALTIME, &ent, &id);        /* CLOCK_REALTIME */
// 
// 	/* start a timer */
// 	clock_gettime(CLOCK_REALTIME, &spec);            /* CLOCK_REALTIME */
// 	value.it_value.tv_sec = spec.tv_sec + 0;
// 	value.it_value.tv_nsec = spec.tv_nsec + 0;
// 	value.it_interval.tv_sec = 0;    /* per second */
// 	value.it_interval.tv_nsec = 100000;
// 	printf_with_time("start timer");
// 	timer_settime(id, TIMER_ABSTIME, &value, NULL); /* TIMER_ABSTIME */
// 
// 	sleep(5);
// 	printf_with_time("delete timer");
// 	timer_delete(id);
// 	sleep(5);
// 	printf("count: %d, overrun: %d\n", count, timer_getoverrun(id));
// 	return 0;
}

