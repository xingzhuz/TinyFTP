#include "common.h"

// 获取文件名部分
void get_filename(const char *path, char *filename)
{
    const char *last_slash = strrchr(path, '/'); // 反向搜索
    if (last_slash != NULL)
    {
        strcpy(filename, last_slash + 1); // 文件名在最后一个斜杠之后
    }
    else
    {
        strcpy(filename, path); // 没有路径部分，直接复制
    }
}

// 获取带编号的文件名
void generate_unique_filename(const char *save_dir, char *filename, size_t size)
{
    char filepath[384]; // 存储完整路径
    char original_name[256];
    char extension[50] = ""; // 用于存储文件的后缀名
    int number = 0;

    // 复制原始文件名，防止修改源文件名
    strncpy(original_name, filename, sizeof(original_name) - 1);
    original_name[sizeof(original_name) - 1] = '\0';

    // 分离文件名和扩展名
    char *dot = strrchr(original_name, '.');
    if (dot != NULL)
    {
        // 如果有扩展名，复制扩展名
        strncpy(extension, dot, sizeof(extension) - 1);
        extension[sizeof(extension) - 1] = '\0';
        *dot = '\0'; // 去掉原文件名中的扩展名部分
    }

    // 初始文件名
    snprintf(filepath, sizeof(filepath), "%s/%s%s", save_dir, original_name, extension);

    // 不断尝试新的文件名，直到不存在为止
    while (file_exists(filepath))
    {
        snprintf(filepath, sizeof(filepath), "%s/%s(%d)%s", save_dir, original_name, ++number, extension);
    }

    // 生成唯一文件名
    snprintf(filename, size, "%s(%d)%s", original_name, number, extension);
}
