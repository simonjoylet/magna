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
#include "HbThread.h"

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
NodeClient * g_nodeProxy;
std::mutex * g_nodelist_mutex;
void testNodeEcho()
{
	google::protobuf::StringValue req;
	google::protobuf::StringValue resp;
	req.set_value("Access NodeAgent Success");
	int ret = g_nodeProxy->PhxEcho(req, &resp);
	printf("NodeClient.PhxEcho return %d\n", ret);
	printf("resp: {\n%s}\n", resp.DebugString().c_str());
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
	g_nodeProxy = new NodeClient;
	g_nodelist_mutex = new std::mutex;

	// 启动心跳线程
	HbThread::GetInstance()->Start();
	
    ServiceArgs_t service_args;
    service_args.config = &config;

    phxrpc::HshaServer server(config.GetHshaServerConfig(), Dispatch, &service_args);
    server.RunForever();

    phxrpc::closelog();

    return 0;
}

