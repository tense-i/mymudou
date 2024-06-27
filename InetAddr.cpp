#include "InetAddr.h"
#include <string.h>

InetAddr::InetAddr(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);                  // 主机字节序转网络字节序
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 点分十进制ip地址转换为网络字节序
}

InetAddr::InetAddr(const sockaddr_in &addr) : addr_(addr)
{
}

InetAddr::~InetAddr()
{
}

std::string InetAddr::toIp() const
{
    char buf[64] = "";
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddr::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

uint16_t InetAddr::toPort() const
{
    return addr_.sin_port;
}

const sockaddr *InetAddr::getSockaddr() const
{
    return (sockaddr *)&addr_;
}

void InetAddr::setSockaddr(const sockaddr_in &addr)
{
    addr_ = addr;
}
