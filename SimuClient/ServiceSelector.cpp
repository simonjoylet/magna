#include "ServiceSelector.h"
#include <random>
#include <iostream>

phxrpc::Endpoint_t ServiceSelector::GetService() const
{
// 	double randnumber = rand() % 1000 / (double)1000;
// 
// 	for (size_t i = 0; i < m_serviceList.size(); ++i)
// 	{
// 		if (randnumber <= m_serviceList[i].cp)
// 		{
// 			return m_serviceList[i].ep;
// 		}
// 	}
// 	std::cout << "Service probability not full\n";
// 	return m_serviceList[0].ep;
	uint32_t index = rand() % m_serviceList.size();
	return m_serviceList[index].ep;
}

bool ServiceSelector::AddService(const phxrpc::Endpoint_t & ep, double percentage)
{
	if (m_serviceList.empty())
	{
		m_serviceList.push_back(ServiceCP{ ep, percentage });
		return true;
	}

	double cp = m_serviceList.back().cp + percentage;
	m_serviceList.push_back(ServiceCP{ ep, cp });
	return true;
}
