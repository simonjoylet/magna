#include "comp_service_impl_0.cpp"

int CompServiceImpl::Handle(const magna::AppRequest &req, magna::AppResponse *resp) {
	//usleep(5000);
	resp->set_id(req.id());
	resp->set_ack(true);
    return 0;
}

