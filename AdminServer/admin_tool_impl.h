/* admin_tool_impl.h

 Generated by phxrpc_pb2tool from admin.proto

*/

#pragma once

#include <stdio.h>

#include "phxrpc_admin_tool.h"
namespace phxrpc {
    class OptMap;
}

class AdminToolImpl : public AdminTool
{
public:
    AdminToolImpl();
    virtual ~AdminToolImpl();

    virtual int PHXEcho( phxrpc::OptMap & opt_map );

    virtual int RegisterNode( phxrpc::OptMap & opt_map );

    virtual int NodeHeatbeat( phxrpc::OptMap & opt_map );

    virtual int RegisterService( phxrpc::OptMap & opt_map );

    virtual int ServiceHeatbeat( phxrpc::OptMap & opt_map );

};

