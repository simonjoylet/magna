#include "SocketOps.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Endian.h"
#include "Logging.h"
#include "Types.h"


int base::connect(int sockfd, const struct sockaddr* addr)
{
	return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

void base::bindOrDie(int sockfd, const struct sockaddr* addr)
{
	int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
	if (ret < 0)
	{
		LOG_SYSFATAL << "sockets::bindOrDie";
	}
}

void base::listenOrDie(int sockfd)
{
	int ret = ::listen(sockfd, SOMAXCONN);
	if (ret < 0)
	{
		LOG_SYSFATAL << "sockets::listenOrDie";
	}
}

int base::accept(int sockfd, struct sockaddr_in* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

	int connfd = ::accept4(sockfd, sockaddr_cast(addr),
		&addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (connfd < 0)
	{
		int savedErrno = errno;
		LOG_SYSERR << "Socket::accept";
		switch (savedErrno)
		{
		case EAGAIN:
		case ECONNABORTED:
		case EINTR:
		case EPROTO: // ???
		case EPERM:
		case EMFILE: // per-process lmit of open file desctiptor ???
			// expected errors
			errno = savedErrno;
			break;
		case EBADF:
		case EFAULT:
		case EINVAL:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case ENOTSOCK:
		case EOPNOTSUPP:
			// unexpected errors
			LOG_FATAL << "unexpected error of ::accept " << savedErrno;
			break;
		default:
			LOG_FATAL << "unknown error of ::accept " << savedErrno;
			break;
		}
	}
	return connfd;
}

void base::close(int sockfd)
{
	if (::close(sockfd) < 0)
	{
		LOG_SYSERR << "sockets::close";
	}
}

void base::shutdownWrite(int sockfd)
{
	if (::shutdown(sockfd, SHUT_WR) < 0)
	{
		LOG_SYSERR << "sockets::shutdownWrite";
	}
}

int base::getSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);

	if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
}

ssize_t base::read(int sockfd, void *buf, size_t count)
{
	return ::read(sockfd, buf, count);
}

ssize_t base::write(int sockfd, const void *buf, size_t count)
{
	return ::write(sockfd, buf, count);
}

void base::toIpPort(char* buf, size_t size, const struct sockaddr* addr)
{
	toIp(buf, size, addr);
	size_t end = ::strlen(buf);
	const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
	uint16_t port = base::networkToHost16(addr4->sin_port);
	assert(size > end);
	snprintf(buf + end, size - end, ":%u", port);
}

void base::toIp(char* buf, size_t size, const struct sockaddr* addr)
{
	if (addr->sa_family == AF_INET)
	{
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
	}
}

void base::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
	{
		LOG_SYSERR << "sockets::fromIpPort";
	}
}

const struct sockaddr* base::sockaddr_cast(const struct sockaddr_in* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* base::sockaddr_cast(struct sockaddr_in* addr)
{
	return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* base::sockaddr_in_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}
