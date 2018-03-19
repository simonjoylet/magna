#ifndef SIMU_FUNC_H
#define SIMU_FUNC_H

#include "../AdminServer/admin_client.h"
#include "../Comp/comp_client.h"

#include "ReqAnalytics.h"
#include "ServiceSelector.h"
#include "phxrpc/network/timer.h"
#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include <random>
#include <signal.h>    /* union sigval / struct sigevent */
#include <unistd.h> /* sleep */
#include <mutex>
using namespace std;

extern AdminClient * g_adminProxy;
extern map<string, ServiceSelector> * g_serviceTable;

extern std::mutex g_rstDataMutex;
extern map<uint32_t, ReqLog> g_rstData;

extern std::mutex g_loadLogDataMutex;
extern vector<LoadLog> g_loadLogList;
extern uint32_t g_sendCount;
extern uint32_t g_sendLamda;

bool TestAccessAdminServer();

bool ReadTrafficFile(string filePath, vector<AppReq> & traffic);

int UpdateServiceTable(map<string, ServiceSelector> & table);

int StartSimu(const vector<AppReq> & traffic, map<uint32_t, ReqLog> & rstData, map<int32_t, int32_t> & retMap);

int SimuAll();

int Stress(string compName, const phxrpc::Endpoint_t & ep, map<int, string> & trafficFiles);




#endif//SIMU_FUNC_H
