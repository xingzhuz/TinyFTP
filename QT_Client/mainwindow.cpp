#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QHostAddress>
#include <QDebug>
#include <QNetworkProxy>
#include <QTimer>
#include "mainmenu.h"
#include "utility.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tcp(new QTcpSocket(this))  // 初始化 m_tcp
{
    ui->setupUi(this);
    setWindowTitle("登录");

    // 初始化状态
    ui->ip->setText("192.168.208.128");    // 服务器IP
    ui->port->setText("8091");             // 绑定的端口
    ui->username->setText("xingzhu");      // 用户名
    ui->password->setText("1");            // 密码
    ui->pushButton->setEnabled(false);     // 设置登录按钮不可用
    ui->connectServer->setEnabled(true);   // 设置连接服务器按钮可用
    ui->ip->setFixedHeight(40);            // 设置固定高度为 40 像素
    ui->port->setFixedHeight(40);
    ui->username->setFixedHeight(40);
    ui->password->setFixedHeight(40);
    ui->connectServer->setFixedHeight(50);
    ui->connectServer->setFixedWidth(130);  // 设置固定宽度为 130 像素
    ui->pushButton->setFixedHeight(50);


    // 状态栏
    m_status = new QLabel;
    m_status->setText("未连接");
    ui->statusbar->addWidget(new QLabel("连接状态: "));
    ui->statusbar->addWidget(m_status);

    // 检测是否连接成功
    connect(m_tcp, &QTcpSocket::connected, this, [ = ]()
    {
        ui->pushButton->setEnabled(true);      // 设置登录按钮可用
        ui->connectServer->setEnabled(false);  // 设置连接服务器按钮不可用
        m_status->setText("连接成功");
    });

    // 检测服务器是否断开连接
    connect(m_tcp, &QTcpSocket::disconnected, this, [ = ]()
    {
        m_status->setText("未连接");
        m_tcp->close();
    });

    // 接收服务器消息
    connect(m_tcp, &QTcpSocket::readyRead, this, &MainWindow::LoginFeedback);
}

// 登录按钮的槽函数
void MainWindow::on_pushButton_clicked()
{
    // 获取用户输入的用户名和密码
    QString username = ui->username->text();
    QString password = ui->password->text();
    m_username = username;

    // 创建 JSON 对象
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    // 将 JSON 对象转换为字节数组
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    // 发送 JSON 数据到服务器
    if (m_tcp->state() == QAbstractSocket::ConnectedState)
    {
        m_tcp->write(jsonData);
        m_tcp->flush();  // 确保数据被发送
    }
    else
    {
        QMessageBox::warning(this, "连接错误", "未能连接到服务器");
    }
}

// 连接服务器按钮的槽函数
void MainWindow::on_connectServer_clicked()
{
    // 获取 IP 地址和端口
    QString ip = ui->ip->text();
    unsigned short port = ui->port->text().toUShort();

    // 更新状态显示
    m_status->setText("正在连接...");

    // 连接到服务器
    m_tcp->setProxy(QNetworkProxy::NoProxy);
    m_tcp->connectToHost(QHostAddress(ip), port);
}

// 接收服务器登录验证的反馈
void MainWindow::LoginFeedback()
{
    QByteArray responseData = m_tcp->readAll(); // 读取服务器的响应

    // 定义用于存储解析结果的变量
    QString status;
    QString message;

    // 使用封装的 parseJsonMessage 函数解析 JSON 数据
    Utility::parse_json_message(responseData, status, message);

    if (status == "success")
    {
        // 在关闭窗口之前断开所有与 m_tcp 的连接
        disconnect(m_tcp, nullptr, this, nullptr);

        // 关闭当前窗口
        this->close();
        m_tcp->close();

        // 给 main 函数发出当前窗口关闭信号，用于打开另一个窗口
        emit windowClosed(m_username);
    }
    else if (status == "error")
    {
        QMessageBox::warning(this, "登录失败", message); // 登录失败提示
    }
    else
    {
        QMessageBox::warning(this, "错误", "登录验证反馈接收到无效的 JSON 响应");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
