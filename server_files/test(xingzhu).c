#include "server.h"
#include "common.h"

int copeDownload(int fd, char *username)
{
    // 下载文件
    char filename[128];
    read(fd, filename, sizeof(filename)); // 接收文件名

    // 构建文件路径
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", files_dir, filename);

    // 打开文件准备读取
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        // 文件不存在，向客户端发送不存在指令
        send_json_message(fd, "error", "FILE_NOTEXISTS");
        perror("fopen");
        return -1;
    }

    // 加锁，防止多个客户端同时读取同一个文件
    pthread_mutex_lock(&file_mutex);

    // 文件存在，发送 "FILE_EXISTS" 指令
    send_json_message(fd, "success", "FILE_EXISTS");
    sleep(1);

    // 读取文件内容并发送给客户端
    int bytes;
    char *filebuf = (char *)malloc(BUFFER_SIZE);
    if (filebuf == NULL)
    {
        perror("Memory allocation failed");
        return 0;
    }
    char response_md5[1024];

    // 一次性读取一个字节，一共读取 BUFFER_SIZE 字节
    // 循环的原因是可能文件大于超过 BUFFER_SIZE
    while ((bytes = fread(filebuf, 1, BUFFER_SIZE, fp)) > 0)
    {
        write(fd, filebuf, bytes); // 发送数据

        // 计算并发送 MD5 校验值
        unsigned char md5_digest[MD5_DIGEST_LENGTH];
        compute_md5((unsigned char *)filebuf, bytes, md5_digest);
        char md5_hex[MD5_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
            sprintf(&md5_hex[i * 2], "%02x", md5_digest[i]);

        sleep(1); // 发送慢一点

        // 发送 md5码
        write(fd, md5_hex, sizeof(md5_hex));

        while (1)
        {
            // 读取客户端的 JSON 校验反馈
            read(fd, response_md5, sizeof(response_md5));

            // 解析 JSON 校验反馈
            char status[10], message[256];
            parse_json_message(response_md5, status, message);

            // 判断反馈结果
            if (strcmp(status, "success") == 0)
            {
                printf("接收方接收到正确的当前块数据!\n");
                break; // 正确接收，继续发送下一个块
            }
            else if (strcmp(status, "error") == 0)
            {
                printf("接收方接收到错误的当前块数据，重新发送该块。\n");
                fseek(fp, -bytes, SEEK_CUR); // 回退到当前块的开始位置
                break;                       // 跳回重发逻辑
            }
        }
    }
    fclose(fp); // 关闭文件

    free(filebuf);

    // 解锁
    pthread_mutex_unlock(&file_mutex);

    // 加锁，保证只有一个进程写这个日志
    pthread_mutex_lock(&logo_mutex);
    // 记录下载操作
    log_operation(username, filename, "download");
    // 解锁
    pthread_mutex_unlock(&logo_mutex);

    return 0;
}