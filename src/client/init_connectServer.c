#include "common.h"
#include "client.h"

int init_connectServer(char *username, char *password, int fd)
{
    char response_login[128]; // 接收服务器反馈

    // 创建 JSON 对象并填充用户名和密码
    cJSON *loginData = cJSON_CreateObject();
    cJSON_AddStringToObject(loginData, "action", "login");
    cJSON_AddStringToObject(loginData, "username", username);
    cJSON_AddStringToObject(loginData, "password", password);

    // 将 JSON 对象转换为字符串
    char *jsonString = cJSON_Print(loginData);

    // 发送 JSON 字符串到服务器
    write(fd, jsonString, strlen(jsonString) + 1);

    // 接收服务器的验证结果
    if (read(fd, response_login, sizeof(response_login)) <= 0)
    {
        perror("read");
        close(fd);
        return 0; // 重新连接和验证
    }

    // 解析服务器的 JSON 响应
    cJSON *response = cJSON_Parse(response_login);
    if (response)
    {
        // 检查登录状态
        const cJSON *status = cJSON_GetObjectItem(response, "status");
        if (status && strcmp(status->valuestring, "success") == 0)
        {
            printf("登录成功！\n");
            // 处理后续操作
            cJSON_Delete(response);
            return 1; // 登录成功，返回 1
        }
        else
        {
            const cJSON *message = cJSON_GetObjectItem(response, "message");
            printf("登录失败: %s\n\n", message ? message->valuestring : "未知错误");
            close(fd); // 登录失败，关闭套接字
        }
        cJSON_Delete(response); // 释放解析的 JSON 对象
    }

    return 0;
}