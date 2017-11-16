#ifndef SOCKET_OPS_H
#define SOCKET_OPS_H

#include <arpa/inet.h>
namespace base
{ 
	
int  connect(int sockfd, const struct sockaddr* addr);
void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
int  accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);
int getSocketError(int sockfd);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t write(int sockfd, const void *buf, size_t count);

void toIpPort(char* buf, size_t size, const struct sockaddr* addr);
void toIp(char* buf, size_t size, const struct sockaddr* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

} // namespace base
#endif//SOCKET_OPS_H
