#include "common.h"

// 客户端部分----------------------------------------
// 连接到服务器的函数
int connect_to_server(const char *ip, int port);

// 初始化登录服务器，根据返回值判断用户名和密码匹配
int init_connectServer(char *username, char *password, int fd);

// 获取文件名部分
void get_filename(const char *path, char *filename);

// 获取带编号的文件名
void generate_unique_filename(const char *save_dir, char *filename, size_t size);

// 上传文件
void upload_file(int fd, char *username);

// 下载文件的操作函数
void download_file(int fd, char *username);

// 列出服务器文件
void list_files(int fd);