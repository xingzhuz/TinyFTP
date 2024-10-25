// 这个 usleep 在这里面太重要了
#include "client.h"
#include "common.h"

// 检查文件是否存在的辅助函数
int file_exists(const char *filename)
{
    return access(filename, F_OK) == 0; // 文件存在返回 0，不存在返回 -1
}

int main()
{
    // 服务器 IP 和端口
    const char *server_ip = "127.0.0.1";
    int server_port = Port;
    int fd;

    // 验证用户登录
    char username[32], password[32];

    while (1)
    {
        // 连接服务器
        fd = connect_to_server(server_ip, server_port);

        // 提示用户输入用户名
        printf("请输入用户名 ('q' 退出): ");
        scanf("%s", username);

        // 如果用户输入 'q'，则退出程序
        if (strcmp(username, "q") == 0)
        {
            close(fd);
            exit(EXIT_SUCCESS);
        }

        // 提示用户输入密码
        printf("请输入密码: ");
        scanf("%s", password);

        // 初始化登录服务器，根据返回值判断用户名和密码匹配
        int flag = init_connectServer(username, password, fd);

        if (flag == 0)
            continue; // 匹配失败，让客户端重新输入
        else
            break;
    }

    while (1)
    {
        printf("1. 上传文件\n");
        printf("2. 下载文件\n");
        printf("3. 查看文件列表\n");
        printf("4. 退出\n");
        printf("请输入数字选择当前操作: ");

        int choice;
        scanf("%d", &choice);
        getchar();
        switch (choice)
        {
        case 1:
            upload_file(fd, username);
            break;
        case 2:
            download_file(fd, username);
            break;
        case 3:
            list_files(fd);
            break;
        case 4:
            close(fd);
            return 0;
        }
    }

    return 0;
}