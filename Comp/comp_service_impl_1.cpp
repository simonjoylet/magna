#include "comp_service_impl_0.cpp"
#include <iostream>
int CompServiceImpl::Handle(const magna::AppRequest &req, magna::AppResponse *resp) {
	//usleep(5000);
	static uint32_t count = 0;
	std::cout << "received: " << ++count << "\n";
	resp->set_id(req.id());
	resp->set_ack(true);
    return 0;
}

