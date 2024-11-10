#ifndef UPLOAD_H
#define UPLOAD_H

#include <QMutex>
#include <QObject>
#include <QTcpSocket>
#include <QWaitCondition>
#include <QFileInfo>

class UpLoad : public QObject
{
    Q_OBJECT
public:
    explicit UpLoad(QObject *parent = nullptr);

    // 上传文件的具体实现
    void uploadFile(QString username, QString path);

signals:

    // 向主线程发送当前上传进度的信号
    void curPercent(int num);

    // 向主线程发送上传文件完成的信号
    void uploadFinished();

    // 请求覆盖信号，用于处理客户端服务器文件是否覆盖
    void requestRecover();

public slots:

    // 处理文件覆盖
    void recover(bool is_recover);

    // 连接服务器
    void connectServer(QString username, QString path, unsigned short port, QString ip);

private:
    QTcpSocket *m_tcp;            // 子线程中用于通信的套接字
    QWaitCondition m_waitCondition;
    QMutex m_mutex;
    bool m_overwriteChoice;      // 存储用户选择，是否覆盖服务器文件
};

#endif // UPLOAD_H
