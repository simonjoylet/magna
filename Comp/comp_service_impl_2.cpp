#include "comp_service_impl.cpp"
#include "InnerHandle.h"
#include <iostream>
std::string g_compName = "Comp_2";
extern map<uint32_t, ReqWaitInfo> g_waitInfoMap;

int InnerHandle(const magna::AppRequest &req, magna::AppResponse *resp) {
	static uint32_t count = 0;
	uint64_t ts = phxrpc::Timer::GetSteadyClockMS();
	printf("%d req received. id = %d, servicename = %s, clienttype = %d, ", ++count, req.id(), req.servicename().c_str(), req.clienttype());
	const char * fileName = "tmp.dat";
	FILE * f = fopen(fileName, "wb");
	if (f == NULL)
	{
		cout << "File open failed\n";
		return -1;
	}
	const int BLOCK_SIZE = 1024 * 50;
	char * tmp = new char[BLOCK_SIZE];
	for (int i = 0; i < 1024; ++i)
	{
		fwrite(tmp, BLOCK_SIZE, 1, f);
	}
	delete[] tmp;
	fclose(f);
	remove(fileName);
	
	magna::RetRequest retReq;
	magna::RetResponse retRsp;

	ReqWaitInfo & waitInfo = g_waitInfoMap[req.id()];
	retReq.set_id(req.id());
	retReq.set_servicename(req.servicename());
	retReq.set_clientweight(req.clienttype());
	retReq.set_complamda(waitInfo.compLamda);
	retReq.set_queuelength(waitInfo.queueLength);
	retReq.set_queuetime(waitInfo.queueEnd - waitInfo.queueBegin);
	uint32_t processTime = phxrpc::Timer::GetSteadyClockMS() - ts;
	retReq.set_processtime(processTime);

	int ret = g_simuProxy->GetRet(retReq, &retRsp);
	printf("time used: %dms, return %s\n", processTime, ret == 0 ? "succ" : "fail");

	return 0;
}
