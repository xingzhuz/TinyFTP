#include "server.h"
#include "common.h"
#include <termios.h>

struct SockInfo infos[128];          // 存储最多 128 个客户端的信息，也就是最多只能连接的数量
char *FILES_DIR = "./server_files/"; // 文件存放的目录

// 定义互斥锁
pthread_mutex_t logo_mutex; // 日志写入的锁

// 检查文件是否存在的辅助函数
int file_exists(const char *filename)
{
    return access(filename, F_OK) == 0; // 文件存在返回 0，不存在返回 -1
}

// 线程处理函数，负责处理每个客户端的请求
void *working(void *arg)
{
    struct SockInfo *info = (struct SockInfo *)arg; // 获取客户端信息

    // 接收客户端发送的 JSON 数据
    char buf[1024];
    int ret = read(info->fd, buf, sizeof(buf));
    buf[ret] = '\0';

    if (ret < 0)
    {
        perror("read");
        close(info->fd);
        return NULL; // 退出线程
    }
    else if (ret == 0)
    {
        // 客户端关闭连接
        printf("客户端 fd 为 %d 的已经关闭连接...\n", info->fd);
        close(info->fd);
        return NULL;
    }

    // 这是用于QT客户端的跳过密码验证
    if (strcmp(buf, "toupload") != 0 && strcmp(buf, "todownload") != 0 && strcmp(buf, "tolist") != 0)
    {
        printf("进入了密码验证\n");

        // 解析客户端发送的 JSON 请求
        cJSON *loginData = cJSON_Parse(buf);
        if (!loginData)
        {
            send_json_message(info->fd, "error", "Invalid JSON format");
            close(info->fd);
            return NULL;
        }

        // 提取用户名和密码
        const cJSON *username = cJSON_GetObjectItem(loginData, "username");
        const cJSON *password = cJSON_GetObjectItem(loginData, "password");

        // 验证用户登录密码
        checkoutUser(username, password, info);

        cJSON_Delete(loginData); // 释放解析的 JSON 对象
    }

    // 处理客户端的请求
    while (1)
    {
        char buf[1024];                             // 用于存储客户端的指令
        int ret = read(info->fd, buf, sizeof(buf)); // 接收客户端指令

        if (ret == 0)
        {
            // 客户端关闭连接
            printf("客户端 fd 为 %d 的已经关闭连接...\n", info->fd);
            info->fd = -1; // 将文件描述符置为无效
            break;
        }
        else if (ret == -1)
        {
            // 数据接收失败
            printf("接收数据失败...\n");
            info->fd = -1;
            break;
        }

        // 如果客户端发送的是"upload"指令
        if (strncmp(buf, "upload", 6) == 0)
        {
            printf("进入上传文件了\n");
            int ret = copeUpload(info->fd, info->username);
            if (ret == -1)
                continue;
        }
        // 如果客户端发送的是"download"指令
        else if (strncmp(buf, "download", 8) == 0)
        {
            printf("进入下载文件了\n");
            int ret = copeDownload(info->fd, info->username);
            if (ret == -1)
                continue;
        }
        // 如果客户端发送的是"list"指令
        else if (strncmp(buf, "list", 4) == 0)
        {
            printf("进入列出服务器文件了\n");
            copeList(info->fd);
        }
    }
    return NULL; // 结束线程
}

int main()
{
    // 创建文件存放目录
    mkdir(FILES_DIR, 0777);

    // 初始化服务器套接字
    socklen_t len = sizeof(struct sockaddr);
    int max = sizeof(infos) / sizeof(infos[0]);
    int fd = initServer(len, max);

    // 主循环，处理客户端连接
    while (1)
    {
        struct SockInfo *pinfo;
        for (int i = 0; i < max; ++i)
        {
            if (infos[i].fd == -1) // 找到一个空闲的客户端槽
            {
                pinfo = &infos[i]; // 分配给新的客户端
                break;
            }
        }
        int connfd = accept(fd, (struct sockaddr *)&pinfo->addr, &len); // 等待客户端连接
        printf("parent thread, connfd: %d\n", connfd);
        if (connfd == -1)
        {
            perror("accept");
            exit(0);
        }
        pinfo->fd = connfd;                                // 记录新连接的文件描述符
        pthread_create(&pinfo->tid, NULL, working, pinfo); // 创建线程处理客户端请求
        pthread_detach(pinfo->tid);                        // 线程分离，自动回收资源
    }

    close(fd); // 关闭监听套接字
    return 0;
}
