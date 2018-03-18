#include "comp_service_impl.cpp"
#include "InnerHandle.h"
#include <iostream>
std::string g_compName = "Comp_3";
int InnerHandle(const magna::AppRequest &req, magna::AppResponse *resp) {
	static uint32_t count = 0;
	uint64_t ts = phxrpc::Timer::GetSteadyClockMS();
	printf("%d req received. id = %d, servicename = %s, clienttype = %d, ", ++count, req.id(), req.servicename().c_str(), req.clienttype());
	usleep(10 * 1000);

	magna::RetRequest retReq;
	magna::RetResponse retRsp;
	retReq.set_id(req.id());
	int ret = g_simuProxy->GetRet(retReq, &retRsp);
	printf("time used: %dms, return %s\n", phxrpc::Timer::GetSteadyClockMS() - ts, ret == 0 ? "succ" : "fail");

	return 0;
}
