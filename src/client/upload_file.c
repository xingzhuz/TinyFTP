#include "client.h"
#include "common.h"

void upload_file(int fd, char *username)
{
    char temp_filename[128], filename[128];
    char response[128];              // 接收服务器的反馈
    unsigned int total_checksum = 0; // 总体校验和
    char status[128], message[256];  // 解析j son
    FILE *fp = NULL;

    // 循环检查文件是否存在
    while (fp == NULL)
    {
        printf("输入要上传的文件名或者('q'退出当前操作): ");
        scanf("%s", temp_filename);

        if (strcmp(temp_filename, "q") == 0)
            return;

        // 尝试打开文件
        fp = fopen(temp_filename, "rb");
        if (fp == NULL)
        {
            perror("fopen");
        }

        // 获取路径中的实际上传的文件名
        // temp_filename 是包括了路径的文件名, filename是只有文件名
        get_filename(temp_filename, filename);

        // 拼接用户名和临时文件名
        strcpy(temp_filename, filename);
        // 查找最后一个点
        char *dot = strrchr(temp_filename, '.');
        if (dot)
        {
            // 保存后缀
            char extension[10];     // 假设后缀不超过 10 个字符
            strcpy(extension, dot); // 复制后缀

            // 截断到文件名
            *dot = '\0';

            // 拼接用户名，括号和后缀
            strcat(temp_filename, "(");       // 加上左括号
            strcat(temp_filename, username);  // 拼接用户名
            strcat(temp_filename, ")");       // 加上右括号
            strcat(temp_filename, extension); // 加上后缀
        }
    }

    // 文件存在，继续执行上传流程
    write(fd, "upload", strlen("upload") + 1);
    usleep(1000);
    write(fd, temp_filename, strlen(temp_filename) + 1);

    // 读取服务器的文件是否存在响应
    read(fd, response, sizeof(response));
    // 解析 JSON 响应
    parse_json_message(response, status, message);

    // 如果文件存在，处理文件替换问题
    if (strcmp(status, "success") == 0)
    {
        char choice;
        printf("服务器上已有同名文件，是否覆盖？(y/n): ");
        scanf(" %c", &choice);

        if (choice == 'y' || choice == 'Y')
        {
            write(fd, "1", 2); // 发送"1"表示覆盖文件
        }
        else
        {
            write(fd, "0", 2); // 发送"0"表示取消上传
            fclose(fp);
            printf("上传取消\n");
            return;
        }
    }
    else if (strcmp(status, "error") == 0)
    {
        printf("开始上传文件...\n");
    }

    // 动态申请内存，用于接收数据
    char *filebuf = (char *)malloc(BUFFER_SIZE);
    if (filebuf == NULL)
    {
        perror("Memory allocation failed");
        return;
    }
    int bytes;
    while ((bytes = fread(filebuf, 1, BUFFER_SIZE, fp)) > 0)
    {
        // 发送数据块
        write(fd, filebuf, bytes);

        usleep(1000); // 发送得慢一点

        total_checksum += simple_checksum(filebuf, bytes); // 累加校验和
    }

    // 读取服务器准备接收数据的响应
    read(fd, response, sizeof(response));

    // 传输结束后，将总体校验和通过管道发送给写进程
    unsigned char final_checksum = total_checksum & 0xFF; // 取低8位
    write(fd, &final_checksum, 1);                        // 管道传输总校验和

    // 读取服务器端，接收完毕反馈
    read(fd, response, sizeof(response));
    parse_json_message(response, status, message); // 解析 json 数据
    if (strcmp(status, "success") == 0)
    {
        printf("服务器数据校验成功!\n");
    }
    else if (strcmp(status, "error") == 0)
    {
        printf("服务器数据校验错误!\n");
    }

    printf("文件上传成功!\n\n");

    fclose(fp);
    free(filebuf);
}
