#include "common.h"

// 简单校验和函数
unsigned char simple_checksum(const char *data, size_t length)
{
    unsigned int checksum = 0;
    for (size_t i = 0; i < length; i++)
    {
        checksum += data[i];
    }
    return checksum & 0xFF; // 返回低8位
}