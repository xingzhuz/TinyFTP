#ifndef DOWNLOADFILE_H
#define DOWNLOADFILE_H

#include <QMutex>
#include <QObject>
#include <QTcpSocket>
#include <QWaitCondition>
#include <QFileInfo>
#include <QThread>
#include <QMessageBox>


class DownloadFile : public QObject
{
    Q_OBJECT
public:
    explicit DownloadFile(QObject *parent = nullptr);

    // 下载文件的具体实现函数
    void downloadFile(QString path);

signals:

    // 向主线程发送当前下载进度的信号
    void curPercent(int num);

    // 向主线程发送下载文件完成的信号
    void downloadFinished(QString path);

public slots:

    // 连接服务器
    void connectServer(QString path, unsigned short port, QString ip);

private:
    QTcpSocket *m_tcp;

};

#endif // DOWNLOADFILE_H
