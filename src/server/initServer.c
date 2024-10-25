#include "server.h"
#include "common.h"

// 初始化服务器套接字
int initServer(socklen_t len, int max)
{
    // 创建用于监听的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字
    if (fd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 设置端口复用
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        exit(0);
    }

    // 设置地址信息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;         // 地址族：IPv4
    addr.sin_port = htons(Port);       // 监听的端口号，使用宏 Port 定义
    addr.sin_addr.s_addr = INADDR_ANY; // 接受任意IP地址的连接

    // 绑定地址信息
    int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind");
        exit(0);
    }

    // 开始监听
    ret = listen(fd, 100); // 最多允许100个客户端排队
    if (ret == -1)
    {
        perror("listen");
        exit(0);
    }

    // 初始化客户端信息数组
    for (int i = 0; i < max; ++i)
    {
        bzero(&infos[i], sizeof(infos[i])); // 清空结构体
        infos[i].fd = -1;                   // 初始化文件描述符为-1，表示未使用
    }

    // 初始化互斥锁
    pthread_mutex_init(&logo_mutex, NULL);

    return fd;
}