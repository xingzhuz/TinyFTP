#include "client.h"
#include "common.h"

// 下载文件的操作函数
void download_file(int fd, char *username)
{
    char filename[128], temp_filename[128];
    char save_dir[256];              // 输入的目录路径
    char filepath[384];              // 文件完整路径
    char response[128];              // 接收服务器的响应
    unsigned int total_checksum = 0; // 总体校验和
    char status[10], message[256];   // json的解析

    while (1)
    {
        printf("输入要下载的文件名或者('q'退出当前操作): ");
        scanf("%s", filename);

        // 读取回车符，会影响下面默认回车符操作
        getchar();

        // 将服务器上的文件中用户名去掉
        strcpy(temp_filename, filename);
        char *left_paren = strchr(filename, '(');  // 查找左括号
        char *right_paren = strchr(filename, ')'); // 查找右括号
        char *dot = strrchr(filename, '.');        // 查找最后一个点
        if (left_paren && right_paren && dot && left_paren < right_paren && right_paren < dot)
        {
            // 截断到左括号
            *left_paren = '\0'; // 截断字符串

            // 重新拼接后缀
            strcat(filename, dot); // 加上后缀
        }

        // 客户端选择退出当前操作
        if (strcmp(filename, "q") == 0)
            return;

        // 循环直到用户输入有效的目录
        while (1)
        {
            printf("请输入保存文件的目录(回车默认为当前目录)(或按'q'退出当前操作): ");
            fgets(save_dir, sizeof(save_dir), stdin);
            save_dir[strcspn(save_dir, "\n")] = '\0'; // 移除末尾的换行符

            // 如果用户未输入路径，则使用当前目录
            if (strlen(save_dir) == 0)
            {
                getcwd(save_dir, sizeof(save_dir)); // 获取当前目录
                printf("当前工作目录: %s\n", save_dir);
            }
            else if (strcmp(save_dir, "q") == 0)
            {
                // 退出当前操作
                return;
            }
            else
            {
                // 检查目录是否存在
                struct stat st;
                if (stat(save_dir, &st) == -1)
                {
                    if (errno == ENOENT)
                    {
                        printf("目录不存在，请重新输入!\n");
                        continue; // 重新提示用户输入路径
                    }
                    else
                    {
                        perror("stat");
                        return;
                    }
                }
                else if (!S_ISDIR(st.st_mode))
                {
                    printf("输入的路径不是一个有效的目录，请重新输入!\n");
                    continue; // 重新提示用户输入路径
                }
            }

            // 拼接文件路径
            snprintf(filepath, sizeof(filepath), "%s/%s", save_dir, filename);
            break; // 退出路径输入循环
        }

        // 发送下载请求
        write(fd, "download", strlen("download") + 1);
        usleep(100);
        write(fd, temp_filename, strlen(temp_filename) + 1);

        // 读取服务器的响应
        read(fd, response, sizeof(response));
        parse_json_message(response, status, message); // 解析 JSON 响应

        if (strcmp(status, "error") == 0)
        {
            printf("文件不存在，请重新输入文件名!\n");
            continue; // 重新提示用户输入文件名
        }
        else if (strcmp(status, "success") == 0)
        {
            // 使用下载的文件名存在的处理过程
            if (file_exists(filepath))
            {
                printf("当前文件已经存在, 为您创建副本: \n");
                filename[sizeof(filename) - 1] = '\0';
                generate_unique_filename(save_dir, filename, sizeof(filepath));

                // 拼接正确的路径
                snprintf(filepath, sizeof(filepath), "%s/%s", save_dir, filename);
                printf("新文件名 %s\n", filename);
            }

            // 打开文件
            FILE *fp = fopen(filepath, "wb");
            if (fp == NULL)
            {
                perror("fopen");
                return;
            }

            // 动态申请内存，用于接收数据
            char *filebuf = (char *)malloc(BUFFER_SIZE);
            if (filebuf == NULL)
            {
                perror("Memory allocation failed");
                return;
            }
            int bytes;
            while ((bytes = read(fd, filebuf, BUFFER_SIZE)) > 0)
            {
                // **不要自动添加字符串终止符，处理二进制数据**
                // 在处理文件时，我们不应假设它是文本数据。保留原始的二进制数据。

                fwrite(filebuf, 1, bytes, fp); // 写入文件

                total_checksum += simple_checksum(filebuf, bytes);
                if (bytes < BUFFER_SIZE)
                    break; // 文件已全部接收
            }

            // 通知客户端，服务器准备好接收了
            write(fd, "start", 6);

            // 接收传来的总体校验和
            unsigned char received_final_checksum;
            read(fd, &received_final_checksum, 1);

            // 计算最终校验和的低8位
            unsigned char calculated_final_checksum = total_checksum & 0xFF;

            // 验证校验和
            if (calculated_final_checksum == received_final_checksum)
            {
                printf("校验和匹配，数据传输成功，等待继续传输\n");
            }
            else
            {
                printf("校验和不匹配，数据部分有误\n");
            }

            // 发送校验成功的发聩
            send_json_message(fd, "success", "校验成功");
            printf("文件下载成功!\n");

            fclose(fp);
            free(filebuf);
            break;
        }
        else
        {
            printf("文件不存在或文件名写错\n");
            break; // 退出当前操作
        }
    }
}
