#ifndef LISTFILE_H
#define LISTFILE_H

#include <QObject>
#include <QFileInfo>
#include <QThread>
#include <QTcpSocket>

class ListFile : public QObject
{
    Q_OBJECT
public:
    explicit ListFile(QObject *parent = nullptr);

    // 列出服务器文件的具体实现
    void listFile();

signals:

    // 发送给主线程的文件清单
    void dataList(QString fileList);

public slots:

    // 连接服务器
    void connectServer(unsigned short port, QString ip);

private:
    QTcpSocket *m_tcp; // 子线程中用于通信的套接字
};

#endif // LISTFILE_H
