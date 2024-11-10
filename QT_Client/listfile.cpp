#include "listfile.h"

#include <QNetworkProxy>

ListFile::ListFile(QObject *parent) : QObject(parent)
{

}

// 连接服务器
void ListFile::connectServer(unsigned short port, QString ip)
{
    m_tcp = new QTcpSocket;
    m_tcp->setProxy(QNetworkProxy::NoProxy);

    qDebug() << "列出文件的子线程正在连接...";
    m_tcp->connectToHost(QHostAddress(ip), port);

    // 检测是否连接成功，成功发送信号给主线程
    connect(m_tcp, &QTcpSocket::connected, this, [ = ]()
    {
        qDebug() << "列出文件的子线程连接成功";

        m_tcp->write("tolist");
        m_tcp->flush();

        QThread::msleep(10); // 休眠10毫秒

        listFile();
    });
}

void ListFile::listFile()
{
    qDebug() << "开始列出了...";
    // 首先发送是上传请求
    m_tcp->write("list\n");
    m_tcp->flush();

    QByteArray data;

    // 等待服务器发送数据
    if (m_tcp->waitForReadyRead(5000))   // 等待最多5秒
    {
        data = m_tcp->readAll(); // 读取所有数据
    }
    else
    {
        qDebug() << "列出服务器文件:等待数据超时";
        return;
    }

    // 将接收到的数据转换为字符串
    QString fileList = QString::fromUtf8(data);

    // 向主线程发送文件清单
    emit dataList(fileList);

    if (m_tcp)
    {
        m_tcp->close(); // 关闭连接
    }
}

