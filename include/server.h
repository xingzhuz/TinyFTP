#ifndef SERVER_H
#define SERVER_H

#include "common.h"

// 服务器部分------------------------------------------
// 初始化服务器套接字
int initServer(socklen_t len, int max);

// 服务器处理上传文件
int copeUpload(int fd, char *username);

// 服务器处理下载文件
int copeDownload(int fd, char *username);

// 服务器处理查看文件
void copeList(int fd);

// 日志记录函数，记录每个文件操作
void log_operation(const char *username, const char *filename, const char *operation);

// 验证用户名和密码是否匹配
void checkoutUser(const cJSON *username, const cJSON *password, struct SockInfo *info);

#endif // SERVER_H