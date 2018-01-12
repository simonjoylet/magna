#include "AdminData.h"

AdminData * AdminData::m_instance = NULL;

AdminData * AdminData::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new AdminData();
	}

	return m_instance;
}

localdata::InetAddress::InetAddress()
{
	ip = "";
	port = 0;
}

localdata::InetAddress::InetAddress(string _ip, int32 _port)
{
	ip = _ip;
	port = _port;
}

bool localdata::InetAddress::operator<(const InetAddress & param)
{
	if (ip < param.ip)
	{
		return true;
	}
	else if (ip == param.ip)
	{
		return port < param.port;
	}
	else
	{
		return false;
	}
}
