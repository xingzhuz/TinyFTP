#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLabel>
#include <QJsonObject>
#include <QJsonDocument>

#include "mainmenu.h"
#include "ui_mainmenu.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

    // 给 main 函数发出当前窗口关闭信号，用于打开另一个窗口
    void windowClosed(QString username);

private slots:

    // 登录按钮的槽函数
    void on_pushButton_clicked();

    // 连接服务器按钮的槽函数
    void on_connectServer_clicked();

    // 接收服务器登录验证的反馈
    void LoginFeedback();


private:
    Ui::MainWindow *ui;
    QTcpSocket *m_tcp;     // 和服务器通信的套接字
    QLabel *m_status;      // 任务栏状态
    QString m_ip;          // 服务器IP地址
    unsigned short m_port; // 服务器端口
    QString m_username;    // 存储用户名
};

#endif // MAINWINDOW_H
