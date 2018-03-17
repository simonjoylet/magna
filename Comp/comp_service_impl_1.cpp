#include "comp_service_impl.cpp"
#include <iostream>
#include <math.h>
int CompServiceImpl::Handle(const magna::AppRequest &req, magna::AppResponse *resp) {
	//usleep(5000);
	static uint32_t count = 0;
	uint64_t ts = phxrpc::Timer::GetSteadyClockMS();
	uint32_t calNum = rand() % 10000 * 1000;
	for (uint32_t i = 0; i < calNum; ++i)
	{
		pow(rand(), i);
	}

	resp->set_id(req.id());
	resp->set_ack(true);
	printf("received: %d, time used: %dms\n", ++count, phxrpc::Timer::GetSteadyClockMS()-ts);
    return 0;
}

