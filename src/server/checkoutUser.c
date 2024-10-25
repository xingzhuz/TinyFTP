#include "common.h"
#include "server.h"
#include <mysql/mysql.h>

// 读取 JSON 配置文件并获取数据库连接信息
cJSON *read_db_config(const char *filename)
{
    // 尝试打开指定的 JSON 配置文件
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        // 打开文件失败
        fprintf(stderr, "Failed to open config file: %s\n", filename);
        return NULL;
    }

    // 移动文件指针到文件末尾以确定文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); // 获取文件的字节大小
    fseek(file, 0, SEEK_SET);     // 将文件指针重新移动到文件的开头

    // 为文件内容分配内存，+1 是为了容纳结尾的空字符 '\0'
    char *json_data = (char *)malloc(file_size + 1);
    if (json_data == NULL)
    {
        // 内存分配失败时关闭文件并返回 NULL
        fclose(file);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // 读取整个文件内容到分配的内存中
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0'; // 添加字符串结束符以确保数据是有效的字符串
    fclose(file);                // 关闭文件

    // 解析 JSON 数据
    cJSON *json = cJSON_Parse(json_data);
    free(json_data); // 释放之前分配的内存，因为内容已被解析

    // 如果 JSON 解析失败，打印错误消息
    if (json == NULL)
    {
        fprintf(stderr, "Error parsing config file\n");
    }

    // 返回解析后的 JSON 对象，或者在解析失败时返回 NULL
    return json;
}

void checkoutUser(const cJSON *username, const cJSON *password, struct SockInfo *info)
{
    if (username && password)
    {
        // 从 JSON 文件中读取数据库连接配置
        cJSON *db_config = read_db_config("dbconf.json");
        if (db_config == NULL)
        {
            send_json_message(info->fd, "error", "Server configuration error");
            close(info->fd);
            return;
        }

        // 获取 JSON 中的数据库连接信息
        const char *db_ip = cJSON_GetObjectItem(db_config, "db_ip")->valuestring;
        int db_port = cJSON_GetObjectItem(db_config, "db_port")->valueint;
        const char *db_user = cJSON_GetObjectItem(db_config, "db_user")->valuestring;
        const char *db_password = cJSON_GetObjectItem(db_config, "db_password")->valuestring;
        const char *db_name = cJSON_GetObjectItem(db_config, "db_name")->valuestring;

        MYSQL *conn;    // MySQL 连接对象
        MYSQL_RES *res; // 查询结果
        MYSQL_ROW row;  // 行数据

        // 初始化 MySQL 连接
        conn = mysql_init(NULL);
        if (conn == NULL)
        {
            fprintf(stderr, "mysql_init() failed\n");
            cJSON_Delete(db_config);
            return;
        }

        // 连接到数据库
        if (mysql_real_connect(conn, db_ip, db_user, db_password, db_name, db_port, NULL, 0) == NULL)
        {
            fprintf(stderr, "mysql_real_connect() failed\n");
            mysql_close(conn);
            cJSON_Delete(db_config);
            return;
        }

        // 创建查询语句
        char sql[256];
        snprintf(sql, sizeof(sql), "SELECT password FROM User WHERE username='%s'", username->valuestring);

        // 执行查询
        if (mysql_query(conn, sql))
        {
            fprintf(stderr, "mysql_query() failed: %s\n", mysql_error(conn));
            mysql_close(conn);
            cJSON_Delete(db_config);
            return;
        }

        // 获取查询结果
        res = mysql_store_result(conn);
        if (res == NULL)
        {
            fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
            mysql_close(conn);
            cJSON_Delete(db_config);
            return;
        }

        int user_found = 0;         // 用于标记用户是否找到
        char fetched_password[128]; // 用于存储从数据库中获取的密码

        // 遍历查询结果
        while ((row = mysql_fetch_row(res)))
        {
            user_found = 1; // 用户存在
            strncpy(fetched_password, row[0], sizeof(fetched_password) - 1);
            fetched_password[sizeof(fetched_password) - 1] = '\0'; // 确保字符串结束
            break;                                                 // 找到匹配的用户，退出循环
        }

        // 释放结果集
        mysql_free_result(res);

        if (user_found)
        {
            // 验证密码
            if (strcmp(fetched_password, password->valuestring) == 0)
            {
                // 验证成功，存储用户名
                strncpy(info->username, username->valuestring, sizeof(info->username) - 1);
                info->username[sizeof(info->username) - 1] = '\0';

                // 发送成功消息
                send_json_message(info->fd, "success", "Login successful");
            }
            else
            {
                // 密码错误，发送失败消息
                send_json_message(info->fd, "error", "Invalid password");
                close(info->fd);
            }
        }
        else
        {
            // 用户名不存在，发送失败消息
            send_json_message(info->fd, "error", "User does not exist");
            close(info->fd);
        }

        // 关闭数据库连接
        mysql_close(conn);
        cJSON_Delete(db_config); // 释放 JSON 对象
    }
    else
    {
        // 发送失败消息，缺少用户名或密码
        send_json_message(info->fd, "error", "Missing username or password");
        close(info->fd);
    }
}