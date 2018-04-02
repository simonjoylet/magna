/* admin_main.cpp

 Generated by phxrpc_pb2server from admin.proto

*/

#include <iostream>
#include <memory>
#include <unistd.h>
#include <signal.h>

#include "phxrpc_admin_dispatcher.h"
#include "admin_service_impl.h"
#include "admin_server_config.h"

#include "phxrpc/rpc.h"
#include "phxrpc/msg.h"
#include "phxrpc/file.h"
#include "../NodeAgent/node_client.h"
#include <mutex>
#include <thread>
#include "AdminData.h"
#include "ResourceCost.h"

using namespace std;


void Dispatch(const phxrpc::BaseRequest *request,
              phxrpc::BaseResponse *response,
              phxrpc::DispatcherArgs_t *args) {
    ServiceArgs_t *service_args = (ServiceArgs_t *)(args->service_args);

    AdminServiceImpl service(*service_args);
    AdminDispatcher dispatcher(service, args);

    phxrpc::BaseDispatcher<AdminDispatcher> base_dispatcher(
            dispatcher, AdminDispatcher::GetMqttFuncMap(),
            AdminDispatcher::GetURIFuncMap());
    if (!base_dispatcher.Dispatch(request, response)) {
        response->DispatchErr();
    }
}

void ShowUsage(const char *program) {
    printf("\n");
    printf("Usage: %s [-c <config>] [-d] [-l <log level>] [-v]\n", program);
    printf("\n");

    exit(0);
}


bool g_hbShouldRun = true;
extern phxrpc::ClientConfig global_nodeclient_config_;

void AdminHbFunc()
{
	AdminData * adminData = AdminData::GetInstance();

	printf("\nAdmin HbThread start running...\n");

	// 时钟轮盘
	while (g_hbShouldRun)
	{
		static uint32_t count = 0;
		sleep(1);
		++count;

		// 每2秒心跳计数加1，移除心跳超时的节点。
		if (count % 2 == 0)
		{
			adminData->lock();
			for (auto it = adminData->m_nodeList.begin(); it != adminData->m_nodeList.end(); ++it)
			{
				it->second.heatbeat += 1;
				if (it->second.heatbeat > 5)
				{
					phxrpc::Endpoint_t ep;
					snprintf(ep.ip, sizeof(ep.ip), "%s", it->second.addr.ip.c_str());
					ep.port = it->second.addr.port;
					global_nodeclient_config_.Remove(ep);
					adminData->m_nodeList.erase(it++);
					printf("\nnode: %s: %d removed\n", ep.ip, ep.port);
					continue;
				}
			}
			adminData->unlock();
		}

	}
	printf("\nAdmin HbThread stopped...\n");
	return;
}

int main(int argc, char **argv) {
    const char *config_file{nullptr};
    bool daemonize{false};
    int log_level{-1};
    extern char *optarg;
    int c;
    while (EOF != (c = getopt(argc, argv, "c:vl:d"))) {
        switch (c) {
            case 'c' : config_file = optarg; break;
            case 'd' : daemonize = true; break;
            case 'l' : log_level = atoi(optarg); break;

            case 'v' :
            default: ShowUsage(argv[0]); break;
        }
    }

    if (daemonize) phxrpc::ServerUtils::Daemonize();

    assert(signal(SIGPIPE, SIG_IGN) != SIG_ERR);

    //set customize log/monitor
    //phxrpc::setlog(openlog, closelog, vlog);
    //phxrpc::MonitorFactory::SetFactory(new YourSelfsMonitorFactory());

    if (nullptr == config_file) ShowUsage(argv[0]);

    AdminServerConfig config;
	if (!config.Read(config_file)) ShowUsage(argv[0]);

    if (log_level > 0) config.GetHshaServerConfig().SetLogLevel(log_level);

    phxrpc::openlog(argv[0], config.GetHshaServerConfig().GetLogDir(),
            config.GetHshaServerConfig().GetLogLevel());


	// 初始化全局变量
	extern phxrpc::ClientConfig global_nodeclient_config_;
	global_nodeclient_config_.Init(1000, 2000, "magna");

	// 读取各个组件的压测数据
	AdminData * ad = AdminData::GetInstance();
	ad->ReadStressData("Comp_1", "../SimuClient/Comp_1_223.3.87.60.stress");
	ad->ReadStressData("Comp_2", "../SimuClient/Comp_2_223.3.87.60.stress");
	ad->ReadStressData("Comp_3", "../SimuClient/Comp_3_223.3.87.60.stress");
// 	//ad->SaveCompFeature(featureFile);
// 
// 	string featureFile = "test.feature";
// 	ad->ReadCompFeature(featureFile);
	// 初始化路由表
	auto initRouterFunc = [&ad]()
	{
		while (true)
		{
			sleep(1);
			if (ad->m_nodeList.size() > 2)
			{
				ad->InitServiceTable();
				break;
			}
		}
		
	};
	auto updateRouterFunc = [&ad]()
	{
		while (true)
		{
			sleep(1);
			ad->UpdateServiceTable();
		}
	};
	
	std::thread initRouter(initRouterFunc);
	std::thread updateRouter(updateRouterFunc);

	// 启动心跳线程
	std::thread hb(AdminHbFunc);
	
    ServiceArgs_t service_args;
    service_args.config = &config;

    phxrpc::HshaServer server(config.GetHshaServerConfig(), Dispatch, &service_args);
    server.RunForever();

    phxrpc::closelog();

    return 0;
}

