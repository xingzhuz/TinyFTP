#include "common.h"

// 发送登录验证的 JSON 格式的消息
// 参数:
// fd: 客户端的文件描述符，用于通信
// status: 表示操作的状态，如 "success" 或 "error"
// message: 具体的消息内容，如 "Login successful" 或 "Login failed"
void send_json_message(int fd, const char *status, const char *message)
{
    // 创建一个 JSON 对象
    cJSON *json = cJSON_CreateObject();

    // 将状态字段（status）添加到 JSON 对象中，格式为 {"status": status}
    cJSON_AddStringToObject(json, "status", status);

    // 将消息字段（message）添加到 JSON 对象中，格式为 {"message": message}
    cJSON_AddStringToObject(json, "message", message);

    // 将 JSON 对象转换为字符串格式，便于传输
    char *json_string = cJSON_Print(json);

    // 通过文件描述符将 JSON 字符串发送给客户端
    // 使用 write 系统调用，发送的内容是 json_string 字符串及其末尾的 '\0'
    write(fd, json_string, strlen(json_string) + 1);

    // 删除创建的 JSON 对象，释放内存
    cJSON_Delete(json);

    // 释放 JSON 字符串所占用的内存
    free(json_string);
}

// 解析 JSON 格式的消息
void parse_json_message(char *buffer, char *status, char *message)
{
    cJSON *json = cJSON_Parse(buffer);
    if (json != NULL)
    {
        cJSON *status_item = cJSON_GetObjectItem(json, "status");
        cJSON *message_item = cJSON_GetObjectItem(json, "message");
        if (cJSON_IsString(status_item) && status_item->valuestring)
        {
            strcpy(status, status_item->valuestring);
        }
        if (cJSON_IsString(message_item) && message_item->valuestring)
        {
            strcpy(message, message_item->valuestring);
        }
        cJSON_Delete(json);
    }
}