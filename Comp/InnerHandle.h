#ifndef INNER_HANDLE_H
#define INNER_HANDLE_H
#include "comp.pb.h"
#include <stdint.h>
#include "phxrpc/network.h"
#include "../SimuClient/simu_client.h"
extern SimuClient * g_simuProxy;
int32_t InnerHandle(const magna::AppRequest &req, magna::AppResponse *resp);

#endif//INNER_HANDLE_H
