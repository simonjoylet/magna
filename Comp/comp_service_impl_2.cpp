#include "comp_service_impl.cpp"
#include <iostream>
int CompServiceImpl::Handle(const magna::AppRequest &req, magna::AppResponse *resp) {
	static uint32_t count = 0;
	printf("Req received. id = %d, servicename = %s, clienttype = %d\n", req.id(), req.servicename(), req.clienttype());
	std::cout << "received: " << ++count << "\n";
	resp->set_id(req.id());
	resp->set_ack(true);
	return 0;
}