#include "InetAddress.h"

#include <memory.h>
#include <assert.h>

#include "Endian.h"
#include "SocketOps.h"

using namespace base;

InetAddress::InetAddress(uint16_t port, bool loopback)
{
	::bzero(&m_addr, sizeof(m_addr));
	in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = hostToNetwork32(ip);
	m_addr.sin_port = hostToNetwork16(port);
}

InetAddress::InetAddress(string ip, uint16_t port)
{
	bzero(&m_addr, sizeof(m_addr));
	base::fromIpPort(ip.c_str(), port, &m_addr);
}

const struct sockaddr * InetAddress::getSockAddr() const
{
	return base::sockaddr_cast(&m_addr);
}

sa_family_t InetAddress::family() const
{
	return m_addr.sin_family;
}

string InetAddress::toIp() const
{
	char buf[64] = "";
	base::toIp(buf, sizeof buf, getSockAddr());
	return buf;
}

string InetAddress::toIpPort() const
{
	char buf[64] = "";
	base::toIpPort(buf, sizeof buf, getSockAddr());
	return buf;
}

uint16_t InetAddress::toPort() const
{
	return base::networkToHost16(portNetEndian());
}

uint32_t InetAddress::ipNetEndian() const
{
	assert(family() == AF_INET);
	return m_addr.sin_addr.s_addr;
}

uint16_t InetAddress::portNetEndian() const
{
	return m_addr.sin_port;
}

