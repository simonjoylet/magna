#include "../AdminServer/admin_client.h"
#include <iostream>
using namespace std;

AdminClient * g_adminProxy;

int main()
{
	AdminClient::Init("../AdminServer/admin_client.conf");
	g_adminProxy = new AdminClient;
	google::protobuf::StringValue req;
	google::protobuf::StringValue resp;
	req.set_value("Access AdminServer Success");
	int ret = g_adminProxy->PhxEcho(req, &resp);
	printf("AdminServer.PhxEcho return %d\n", ret);
	printf("resp: {\n%s}\n", resp.DebugString().c_str());

	return 0;
}