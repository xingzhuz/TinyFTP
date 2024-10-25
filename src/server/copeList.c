#include "server.h"
#include "common.h"

// 服务器处理查看文件
void copeList(int fd)
{
    // 列出文件
    DIR *dir = opendir(files_dir); // 打开文件目录
    struct dirent *entry;
    char file_list[1024] = ""; // 存储文件列表

    // 读取目录中的文件
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG) // 只处理普通文件
        {
            strcat(file_list, entry->d_name); // 将文件名添加到列表
            strcat(file_list, "\n");          // 添加换行符
        }
    }
    closedir(dir);                               // 关闭目录
    write(fd, file_list, strlen(file_list) + 1); // 将文件列表发送给客户端
}