#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H

#include <netinet/in.h>
#include <string>

using std::string;
namespace base
{

class InetAddress
{
	explicit InetAddress(uint16_t port = 0, bool loopback = false);
	InetAddress(string ip, uint16_t port);
	InetAddress(const struct sockaddr_in & addr)
		: m_addr(addr)
	{ }
	const struct sockaddr * getSockAddr() const;
	sa_family_t family() const;
	string toIp() const;
	string toIpPort() const;
	uint16_t toPort() const;
	uint32_t ipNetEndian() const;
	uint16_t portNetEndian() const;

	struct sockaddr_in m_addr;
};


} // namespace base

#endif//INET_ADDRESS_H

