#include "server.h"
#include "common.h"

// 服务器处理上传文件
int copeUpload(int fd, char *username)
{
    char filename[128];              // 文件名
    char filepath[256];              // 完整的文件路径
    unsigned int total_checksum = 0; // 总体校验和

    // 接收文件名
    read(fd, filename, sizeof(filename));

    printf("客户端发送来的文件名:%s\n", filename);

    // 构建文件存储路径
    snprintf(filepath, sizeof(filepath), "%s%s", FILES_DIR, filename);

    // 使用文件名存在的处理过程
    if (file_exists(filepath))
    {
        // 文件已存在，发送提示给客户端
        send_json_message(fd, "success", "FILE_EXISTS");

        // 接收客户端的选择 (1 = 覆盖, 0 = 取消)
        char choice[2];
        read(fd, choice, sizeof(choice));

        if (choice[0] == '0')
        {
            printf("服务器上存在同名文件，客户端选择不覆盖文件，取消上传\n");
            return -1; // 取消上传
        }
        else if (choice[0] == '1')
        {
            printf("服务器上存在同名文件，客户端选择覆盖文件，继续上传\n");
        }
    }
    else
    {
        // 如果文件不存在，继续上传流程
        send_json_message(fd, "error", "FILE_NOTEXISTS");
    }

    // 打开文件准备写入
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    // 申请内存，服务器接收数据的缓冲区
    char *filebuf = (char *)malloc(BUFFER_SIZE);
    if (filebuf == NULL)
    {
        perror("Memory allocation failed");
        return 0;
    }

    int bytes;
    while ((bytes = read(fd, filebuf, BUFFER_SIZE)) > 0)
    {
        fwrite(filebuf, 1, bytes, fp); // 以二进制写入文件

        total_checksum += simple_checksum(filebuf, bytes); // 累加校验和

        if (bytes < BUFFER_SIZE)
        {
            printf("当前文件所有数据块接收完毕, 即将进行校验!\n");
            break;
        }
    }

    // 通知客户端，服务器准备好接收了
    write(fd, "start", 6);

    // 接收客户端传来的总校验和
    unsigned char received_final_checksum;
    read(fd, &received_final_checksum, 1);

    // 计算最终校验和的低8位
    unsigned char calculated_final_checksum = total_checksum & 0xFF;

    // 验证校验和
    if (calculated_final_checksum == received_final_checksum)
    {
        printf("校验和匹配，数据传输成功\n");
    }
    else
    {
        printf("校验和不匹配，数据部分有误\n");
    }

    // 发送校验成功
    send_json_message(fd, "success", "校验成功");

    // 加锁，保证只有一个进程写这个日志
    pthread_mutex_lock(&logo_mutex);
    // 记录上传操作
    log_operation(username, filename, "upload");
    // 解锁
    pthread_mutex_unlock(&logo_mutex);

    fclose(fp); // 关闭文件
    free(filebuf);

    return 0;
}