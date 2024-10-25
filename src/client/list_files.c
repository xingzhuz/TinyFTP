#include "client.h"
#include "common.h"

// 列出服务器文件
void list_files(int fd)
{
    write(fd, "list", strlen("list") + 1);
    char buf[1024];
    read(fd, buf, sizeof(buf));
    printf("服务器文件列表:\n%s", buf);
}