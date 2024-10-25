#include "client.h"
#include "common.h"

// 函数：连接到服务器
// 参数：
//    ip   - 服务器的 IP 地址
//    port - 服务器的端口号
// 返回值：
//    成功时返回套接字描述符 fd，失败时退出程序

// 连接到服务器的函数
int connect_to_server(const char *ip, int port)
{
    // 创建套接字，使用 IPv4 (AF_INET) 和 TCP 协议 (SOCK_STREAM)
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        // 创建套接字失败，输出错误信息并退出
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址信息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;              // 地址族：IPv4
    addr.sin_port = htons(port);            // 将端口号转换为网络字节序
    inet_pton(AF_INET, ip, &addr.sin_addr); // 将 IP 地址从点分十进制转换为网络字节序

    // 连接到服务器
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        // 连接失败，输出错误信息，关闭套接字并退出
        perror("connect");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 连接成功，返回套接字描述符
    return fd;
}
