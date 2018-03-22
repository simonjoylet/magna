#ifndef SERVICE_SELECTOR_H
#define SERVICE_SELECTOR_H
#include <string>
#include <vector>
#include "phxrpc/rpc.h"

using std::string;
using std::vector;

class ServiceSelector
{
public:
// 	ServiceSelector(){}
// 	~ServiceSelector(){}
	phxrpc::Endpoint_t GetService() const;
	bool AddService(const phxrpc::Endpoint_t & ep, double percentage);
	

	struct ServiceCP
	{
		phxrpc::Endpoint_t ep;
		double cp;// cumulative probability ¿€º∆∏≈¬ 
	};
	
	vector<ServiceCP> m_serviceList;
};

#endif//SERVICE_SELECTOR_H

