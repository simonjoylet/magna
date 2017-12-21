/* node_server_config.h

 Generated by phxrpc_pb2server from node.proto

*/

#pragma once

#include "phxrpc/rpc.h"

class NodeServerConfig
{
public:
    NodeServerConfig();

    ~NodeServerConfig();

    bool Read( const char * config_file );

    phxrpc::HshaServerConfig & GetHshaServerConfig();

private:
    phxrpc::HshaServerConfig ep_server_config_;
};
