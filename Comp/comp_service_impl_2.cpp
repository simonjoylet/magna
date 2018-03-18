#include "comp_service_impl.cpp"
#include "InnerHandle.h"
#include <iostream>

int InnerHandle(const magna::AppRequest &req, magna::AppResponse *resp) {
// 	static uint32_t count = 0;
// 	uint64_t ts = phxrpc::Timer::GetSteadyClockMS();
// 	printf("Req received. id = %d, servicename = %s, clienttype = %d\n", req.id(), req.servicename(), req.clienttype());
// 	std::cout << "received: " << ++count << "\n";
// 	resp->set_id(req.id());
// 	resp->set_ack(true);
// 
// 	magna::RetRequest retReq;
// 	magna::RetResponse retRsp;
// 	retReq.set_id(req.id());
// 	int ret = g_simuProxy->GetRet(retReq, &retRsp);
// 	printf("time used: %dms, return %s\n", phxrpc::Timer::GetSteadyClockMS() - ts, ret == 0 ? "succ" : "fail");

	return 0;
}
