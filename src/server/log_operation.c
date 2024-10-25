#include "server.h"
#include "common.h"

// 日志记录函数，记录每个文件操作
void log_operation(const char *username, const char *filename, const char *operation)
{
    // 打开日志文件（以追加方式打开，若文件不存在则创建）
    FILE *log_fp = fopen("operation.log", "a");
    if (log_fp == NULL)
    {
        perror("fopen");
        return;
    }
    // 将操作记录写入文件
    fprintf(log_fp, "User: %s, File: %s, Operation: %s\n", username, filename, operation);
    // 关闭日志文件
    fclose(log_fp);
}
