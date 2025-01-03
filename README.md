
## 目录结构

```plaintext
netSocket/
├── Makefile            # 根目录的 Makefile
├── operation.log       # 根目录的 ftp 服务器操作日志
├── dbconf.json         # 根目录的 ftp 服务器数据库连接信息
├── server_files/       # ftp 服务器中存在的资源文件
│   ├── cal.c      
│   └── test.txt        # 客户端可执行文件
├── bin/                # 存放生成的可执行文件
│   ├── server_ftp      # 服务器端可执行文件
│   └── client_ftp      # 客户端可执行文件
├── obj/                # 存放生成的对象文件(.o)
├── src/                # 所有源文件的根目录
│   ├── server/         # 服务器端源文件
│   │   ├── server.c
│   │   ├── initServer.c
│   │   ├── checkoutUser.c
│   │   ├── log_operation.c
│   │   ├── copeUpload.c
│   │   ├── copeDownload.c
│   │   └── copeList.c
│   ├── client/         # 客户端源文件
│   │   ├── client.c
│   │   ├── connect_to_server.c
│   │   ├── init_connectServer.c
│   │   ├── upload_file.c
│   │   ├── download_file.c
│   │   └── list_files.c
│   └── common/         # 服务器和客户端共享的公共模块
│       ├── data_checkout.c
│       ├── get_filename.c
│       └── sendjson.c
└── include/            # 头文件存放目录
    ├── server.h        # 服务器端头文件
    ├── client.h        # 客户端头文件
    └── common.h        # 公共头文件        
```

## 环境配置

- `mysql`配置：
    - CSDN：[https://blog.csdn.net/2303_76953932/article/details/142703683?spm=1001.2014.3001.5501](https://blog.csdn.net/2303_76953932/article/details/142703683?spm=1001.2014.3001.5501)
    - 我的自建博客：[https://xingzhu.top/archives/shu-ju-ku-lian-jie-chi-huan-jing-pei-zhi#ubuntu-%E5%AE%89%E8%A3%85-mysql](https://xingzhu.top/archives/shu-ju-ku-lian-jie-chi-huan-jing-pei-zhi#ubuntu-%E5%AE%89%E8%A3%85-mysql)


- `csjon`配置（没有`git`就去安装一个）

```shell
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON/
mkdir build
cd build/
cmake ..
make
make install
```

- 修改`dbconf.json` 文件，`db_ip` 修改为数据库服务器，即所在的电脑 `ip`

```sql
-- 首先需要先创建一个数据库
CREATE DATABASE ftp_linux;
USE ftp_linux;

-- 创建用户表
CREATE TABLE user (
    username CHAR(50) DEFAULT NULL,
    password CHAR(50) DEFAULT NULL
);

# 新建用户和密码，自己更改
INSERT INTO user (username, password) VALUES ('user', 'password');
```

- 在`client`文件目录中的`client.c`中`main`函数中的这个修改为自己的服务器`ip`
- 查询方式查看`ifconfig -a`

```c	
const char *server_ip = "xxxx";
```

 

## 编译运行

```shell
# 编译
make

# 清除可执行文件
make clean
```

```shell
# 运行
# 服务器端
./bin/server_ftp

# 客户端
./bin/client_ftp
```


## 简介

- 实现的是一个`ftp`服务器功能，包含了上传、下载和查看服务器中文件的功能
- 加入了数据库功能，用户验证登录`ftp`服务器的用户名和密码是否匹配
- 使用了`CJSON`进行数据的封装，用于实现`ftp`服务器和客户端之间的信息反馈，统一数据格式
- 可以上传图片、`mp3`文件、文本文件等多种形式文件，速率也能保证
- 处理了上传文件和下载文件的文件同名问题，处理手段是加编号，如`test(1).c`形式，而服务器端同名文件供给客户端选择是否覆盖文件
    - 由于多个客户端可能上传同一个文件，所以上传文件的文件命名使用了加用户名的方式，如`test(xingzhu).c`这种形式
    - 这样保证客户端选择覆盖是覆盖自己上传的文件，而不是其他客户端上传的文件被覆盖
- 进行了数据的完整性检验，这里使用的是弱校验方式 (校验和)，是为了保证速率
- 2024/11/10 增加了QT客户端的实现
- 客户端实现登录界面，然后进入主界面，选择对应文件和位置后，才能点击上传和下载等

**不足之处**:
- 客户端是使用的本地网络下实现的，只有命令行
- 后续会使用图形化界面，使用 Qt 结合做个客户端
- 2024/11/10 已经实现
- 2024/11/10 录制图片时候又发现一个小BUG，不知道位置文件过小(1.5K以下)，QT客户端会下载出问题，但是上传没问题，后续有思路解决


## 开发日志


- 2024/9/11  
    - [x] 新增处理中文乱码问题
- 2024/9/18  
    - [x] 新增处理大文件一次性无法完全传输问题，使用分块传输思想解决
-  2024/9/25  
    - [x] 新增数据完整性检验部分，使用的`MD5`码检验
- 2024/10/10 
    - [x] 新增`JSON`数据统一格式，用于客户端和服务器端信息相互之间的反馈
    - [x] 新增数据库`API`，用于判断用户名和密码是否匹配
    - [x] 整理所有的源文件，整理成模块，可读性提升
- 2024/10/11 
    - [x] 修复成功上传和下载图片、视频等BUG
    - [x] 新增从服务器下载文件可以指定路径，或默认为当前工作目录
- 2024/10/12 
    - [x] 修复大容量慢问题，提高了速率
- 2024/10/16 
    - [x] 修复了之前大容量因网络原因，会出现频繁丢包行为，减少了传输的数据包大小，这个需要注意，因为网络通信中数据包大小有限制，不然会出现频繁丢包，这里呈现的就是频繁重传数据
    - [x] 修复了只有文件所有者才具有覆盖文件的权限，用户不能覆盖非本用户上传的文件，这里修改的是文件名加用户作为服务器端文件名，如`cal(xingzhu).c`
- 2024/10/23 
    - [x] 修复了完整性代码，删除了`MD5`校验，数据传输部分不需要这么强的校验，使用若校验: 校验和校验，而且不用每一部分都去校验，最终总文件校验即可，虽然若校验可能`30%~40%`部分校验不到，但是数据传输的高速很重要，发送数据错误几率低，`MD5`检验是应用层用户自己手动需要校验的变校验
    - [x] 增加了发送校验和之间的反馈，需要先收到对方的准备好接收数据，才发送数据
    - [x] 增加了服务器和客户端之间信息完整性，比如服务器端知道客户端接收完毕等之类信息，这之中解决了很多隐含的BUG
- 2024/11/10
     -  [x] 根据QT客户端，做了一些改动，但是还是能够实现当前命令行下客户端的通信




## QT客户端开发日志

- 2024/11/2 
    - [x] 解决了Qt客户端和linux下服务器端之间的连接问题，问题是需要关闭QT中的代理，`m_tcp->setProxy(QNetworkProxy::NoProxy);`
    - [x] 解决了接收用户名和密码`json`数据时问题，由于服务器端发送了`'\0'`结束符，所以客户端需要去掉
    - [x] 解决了子线程接收主线程的信号反馈实现，原因是不小心把槽函数的头文件定义，写在了信号的声明位置去了，但是报错很离谱：`multiple definition of UpLoad::recover(bool) first defined here`，一直误导我，一直没解决，结果思想错误，实则应该是放在子线程的槽函数位置声明
    - [x] 解决了第一个窗口还存在connect的连接，会检测服务器是否有数据到来，导致后续窗口接收数据出现问题，解决办法是disconnect即可
- 2024/11/3 
    - [x] 发现`connecttoHost` 是异步的，并不阻塞进行
    - [x] 基本实现上传文件的功能，由于套接字不能在子线程中进行，解决办法是在子线程中创建套接字，重新再次连接，这次的连接跳过密码验证
    - [x] 由于上述的措施，导致释放需要关闭两个套接字，但是关闭窗口只能关闭`MainWindow`的套接字(服务器线程的提示)，不理解，一直尝试未果，初步考虑的是`show mainmenu`窗口的时候，发送信号给`main`函数，然后创建`mainmenu`对象，然后进行`show`展示，而在`emit`信号给`Main`函数的部分，关闭`m_tcp`，但是这样之后，虽然服务器显示了关闭，但是后续的客户端向服务器端发送信号的时候，总是携带了第一次套接字通信时发送的`json`数据，一直没解决
    - [x] 为了能正常关闭套接字，目前只能先断开连接，然后关闭窗口，这样两个套接字都能实现关闭，服务器就不会崩掉
- 2024/11/4 
    - [x] 实现了下载功能，根据上述的BUG，很快实现了
    - [x] 目前还是和服务器断开连接问题，一直未果，还是在`show mainmenu`的时候`m_tcp->close()`关闭连接，然后发现了前几天没找到的`BUG`，就是数据部分为何会多出来问题，也就是我发送`toupload`，但是`buf = toupload+第一个套接字发送的json部分数据`，呈现出`touploadpassword:123}`这种形式，然后我发现是感觉替换了的感觉，之前一直以为接收到就是这样，直到我打印服务器收到的字节数，发现是正确的，然后`buf[ret]='\0'`就正确了，但是还是没想通为啥会这样
    - [x] 由于每次发送都要连接服务器，所以干脆发送完就关闭连接，这样BUG都解决了，之前的信号槽一直有问题，现在还没理解为啥关闭`Mainmenu`窗口，不调用自己的析构，反而调用`mainwindow`的析构...
    - [x] 虽然每次发送文件和下载都需要连接服务器，效率低，但是也解决了问题，主要是不想放弃子线程，不然就不能下载和上传同时进行了
- 2024/11/5 
    - [x] 增加了一些工具类，实用性，比如文件同名情况处理，下载的文件名处理
    - [x] 优化了代码，增加了注释





## 运行演示



### 服务器端

<img src="https://bu.dusays.com/2024/10/25/671bb5df798e1.png" alt="服务器端" width="570px" height="180px"></img>







### 客户端

<img src="https://bu.dusays.com/2024/10/25/671bb5dc0a70c.png" alt="客户端" width="450px" height="200px"></img>

<img src="https://bu.dusays.com/2024/10/25/671bb5dd467c8.png" alt="客户端" width="450px" height="300px"></img>

<img src="https://bu.dusays.com/2024/11/10/6730c873aceb8.png" alt="客户端" width="450px" height="250px"></img>





### QT客户端

<img src="https://bu.dusays.com/2024/11/10/6730c8aa194ef.png" alt="QT客户端" width="500px" height="360px"></img>

<img src="https://bu.dusays.com/2024/11/10/6730c8accdb1f.png" alt="QT客户端" width="500px" height="450px"></img>


<img src="https://bu.dusays.com/2024/11/10/6730c8af39fb1.png" alt="QT客户端" width="500px" height="450px"></img>










