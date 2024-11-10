#include "client.h"
#include "common.h"

// 列出服务器文件
void list_files(int fd)
{
    write(fd, "list", strlen("list") + 1);

    char buf[4096]; // 增加缓冲区大小，确保能接收更多的文件信息
    ssize_t bytesRead = read(fd, buf, sizeof(buf));
    if (bytesRead < 0)
    {
        perror("read");
        return;
    }

    printf("+------------------------------------------------------------------------------------------------------------+\n");
    printf("| %-63s | %-22s | %-24s |\n", "文件名", "大小", "修改时间"); // 表头
    printf("+------------------------------------------------------------------------------------------------------------+\n");

    // 输出从服务器读取的文件信息
    char *line = buf;
    while (line && *line)
    {
        char file_name[256], file_size[50], file_time[50];

        // 假设每行格式是 "文件名\t大小\t修改时间"
        if (sscanf(line, "%255[^\t]\t%49[^\t]\t%49[^\n]", file_name, file_size, file_time) == 3)
        {
            printf("| %-60s | %-20s | %-20s |\n", file_name, file_size, file_time);
        }

        // 跳到下一行
        while (*line && *line != '\n')
            line++;
        if (*line)
            line++; // 跳过换行符
    }

    printf("+------------------------------------------------------------------------------------------------------------+\n");
}
