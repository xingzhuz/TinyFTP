#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <iconv.h>
#include <cjson/cJSON.h>
#include <errno.h>

#define Port 8091
#define BUFFER_SIZE (1024 * 32)

// 定义通信结构体
struct SockInfo
{
    int fd;                  // 客户端的通信文件描述符
    pthread_t tid;           // 线程ID
    struct sockaddr_in addr; // 客户端的地址信息
    char username[32];       // 用户名
};

// 全局变量声明
extern struct SockInfo infos[128]; // 用 extern 声明全局变量
extern char *files_dir;
extern pthread_mutex_t logo_mutex;

// 文件操作记录结构体
struct FileRecord
{
    char filename[128]; // 文件名
    char username[32];  // 操作用户
    char operation[16]; // 操作类型："upload" 或 "download"
};

// json -----------------------
// 发送 Json 数据
void send_json_message(int fd, const char *status, const char *message);

// 解析 json 数据
void parse_json_message(char *buffer, char *status, char *message);

// 检测文件是否存在
int file_exists(const char *filename);

// 计算校验和
unsigned char simple_checksum(const char *data, size_t length);

#endif // COMMON_H