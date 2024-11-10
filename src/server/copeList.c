#include "server.h"
#include "common.h"

// 服务器处理查看文件
void copeList(int fd)
{
    DIR *dir = opendir(FILES_DIR); // 打开文件目录
    struct dirent *entry;
    char *file_info = malloc(4096); // 用于存储所有文件信息
    size_t current_length = 0;      // 当前已存储的长度
    file_info[0] = '\0';            // 初始化字符串

    if (dir == NULL)
    {
        perror("opendir");
        free(file_info); // 确保在出错时释放内存
        return;
    }

    // 读取目录中的文件
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG) // 只处理普通文件
        {
            // 构建文件路径
            char filePath[512]; // 增加缓冲区大小
            if (snprintf(filePath, sizeof(filePath), "%s/%s", FILES_DIR, entry->d_name) >= sizeof(filePath))
            {
                fprintf(stderr, "File path truncated: %s/%s\n", FILES_DIR, entry->d_name);
                continue; // 跳过处理
            }

            // 获取文件状态信息
            struct stat fileStat;
            if (stat(filePath, &fileStat) == 0) // 检查是否获取成功
            {
                // 获取文件大小和修改时间
                off_t size = fileStat.st_size;      // 文件大小
                time_t modTime = fileStat.st_mtime; // 文件修改时间
                char modTimeStr[20];                // 用于存储格式化的时间字符串

                // 将时间格式化为字符串
                strftime(modTimeStr, sizeof(modTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&modTime));

                // 准备要添加的文件信息字符串
                char buffer[512]; // 增加缓冲区大小
                if (snprintf(buffer, sizeof(buffer), "%s\t%ld bytes\t%s\n", entry->d_name, size, modTimeStr) >= sizeof(buffer))
                {
                    fprintf(stderr, "Buffer truncated for file: %s\n", entry->d_name);
                    continue; // 跳过处理
                }

                // 确保有足够的空间
                if (current_length + strlen(buffer) >= 4096) // 如果超出范围，重新分配内存
                {
                    char *new_file_info = realloc(file_info, current_length + strlen(buffer) + 4096);
                    if (new_file_info == NULL)
                    {
                        perror("realloc");
                        closedir(dir);
                        free(file_info); // 释放内存
                        return;
                    }
                    file_info = new_file_info;
                }

                // 将文件信息添加到 file_info 中
                strcat(file_info, buffer);
                current_length += strlen(buffer); // 更新当前长度
            }
        }
    }
    closedir(dir); // 关闭目录

    // 发送所有文件信息给客户端
    if (write(fd, file_info, current_length) < 0)
    {
        perror("write");
    }

    free(file_info); // 释放分配的内存
}