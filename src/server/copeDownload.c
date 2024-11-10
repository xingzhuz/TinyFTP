#include "server.h"
#include "common.h"

// 服务器处理下载文件
int copeDownload(int fd, char *username)
{
    // 下载文件
    char filename[128];
    char response[256], status[256], message[256]; // 接收接收方的数据校验反馈
    unsigned int total_checksum = 0;               // 总体校验和

    // 接收文件名
    size_t size = read(fd, filename, sizeof(filename));
    filename[size] = '\0';

    // 构建文件路径
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", FILES_DIR, filename);

    // 打开文件准备读取
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        // 文件不存在，向客户端发送不存在指令
        send_json_message(fd, "error", "FILE_NOTEXISTS");
        perror("fopen");
        return -1;
    }

    // 文件存在，发送 "FILE_EXISTS" 指令
    send_json_message(fd, "success", "FILE_EXISTS");
    usleep(100);

    // 申请内存，服务器传输数据给客户端的缓冲区
    char *filebuf = (char *)malloc(BUFFER_SIZE);
    if (filebuf == NULL)
    {
        perror("Memory allocation failed");
        return 0;
    }

    // 发送文件大小是新增的-------------------------------------------------------
    // 获取文件大小
    uint32_t file_size = 0; // 修改为 uint32_t 类型
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 发送文件大小
    write(fd, &file_size, sizeof(file_size)); // 发送文件大小

    usleep(1000);

    // 一次性读取一个字节，一共读取 BUFFER_SIZE 字节
    // 循环的原因是可能文件大于超过 BUFFER_SIZE
    int bytes;
    while ((bytes = fread(filebuf, 1, BUFFER_SIZE, fp)) > 0)
    {
        write(fd, filebuf, bytes); // 发送数据

        usleep(1000); // 发送慢一点，确保客户端接收完毕

        total_checksum += simple_checksum(filebuf, bytes);
    }

    // 读取客户端准备接收数据的响应
    read(fd, response, sizeof(response));

    // 传输结束后，将总体校验和通过管道发送给写进程
    unsigned char final_checksum = total_checksum & 0xFF; // 取低8位
    write(fd, &final_checksum, 1);                        // 管道传输总体校验和

    // 读取客户端，接收完毕反馈
    read(fd, response, sizeof(response));
    parse_json_message(response, status, message); // 解析 json 数据
    if (strcmp(status, "success") == 0)
    {
        printf("接收方数据校验成功!\n");
        printf("当前文件所有数据块 (用户:%s) 接收完毕, 等待客户端继续操作!\n", username);
    }
    else if (strcmp(status, "error") == 0)
    {
        printf("接收方数据校验错误!\n");
    }

    // 加锁，保证只有一个进程写这个日志
    pthread_mutex_lock(&logo_mutex);
    // 记录下载操作
    log_operation(username, filename, "download");
    // 解锁
    pthread_mutex_unlock(&logo_mutex);

    fclose(fp); // 关闭文件
    free(filebuf);
    return 0;
}