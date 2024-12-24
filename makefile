# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -g

# 源文件目录
SERVER_DIR = src/server
CLIENT_DIR = src/client
COMMON_DIR = src/common

# 头文件目录
INCLUDE_DIR = include

# 对象文件目录
OBJ_DIR = obj

# 可执行文件目录
BIN_DIR = bin

# 可执行文件名
SERVER_TARGET = $(BIN_DIR)/server_ftp
CLIENT_TARGET = $(BIN_DIR)/client_ftp

# 自动获取各目录下的 .c 文件
SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.c)
CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.c)
COMMON_SRCS = $(wildcard $(COMMON_DIR)/*.c)

# 将 .c 文件转换为 .o 文件，存放在 obj/ 目录中
SERVER_OBJS = $(patsubst $(SERVER_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(CLIENT_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))
COMMON_OBJS = $(patsubst $(COMMON_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))

# 包含头文件
INCLUDES = -I$(INCLUDE_DIR)

# 伪目标：同时生成 server 和 client 的可执行文件
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# 规则：生成服务器可执行文件
$(SERVER_TARGET): $(SERVER_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto -lcjson -lmysqlclient -lpthread

# 规则：生成客户端可执行文件
$(CLIENT_TARGET): $(CLIENT_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lssl -lcrypto -lcjson -lmysqlclient -lpthread

# 规则：编译 server/ 目录下的 .c 文件生成 .o 文件
$(OBJ_DIR)/%.o: $(SERVER_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 规则：编译 client/ 目录下的 .c 文件生成 .o 文件
$(OBJ_DIR)/%.o: $(CLIENT_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 规则：编译 common/ 目录下的 .c 文件生成 .o 文件
$(OBJ_DIR)/%.o: $(COMMON_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 创建 bin 和 obj 目录
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 清理生成的文件
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
