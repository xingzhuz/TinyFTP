#include "downloadfile.h"

#include <QNetworkProxy>
#include "utility.h"

DownloadFile::DownloadFile(QObject *parent) : QObject(parent)
{

}



void DownloadFile::connectServer(QString path, unsigned short port, QString ip)
{
    // 连接服务器
    m_tcp = new QTcpSocket;
    m_tcp->setProxy(QNetworkProxy::NoProxy);

    qDebug() << "下载文件的子线程正在连接...";
    m_tcp->connectToHost(QHostAddress(ip), port);

    // 检测是否连接成功，成功发送信号给主线程
    connect(m_tcp, &QTcpSocket::connected, this, [ = ]()
    {
        qDebug() << "连接成功";

        m_tcp->write("todownload");
        m_tcp->flush();

        QThread::msleep(10); // 休眠10毫秒

        downloadFile(path);
    });
}

void DownloadFile::downloadFile(QString path)
{
    unsigned int total_checksum = 0; // 总体校验和
    int total_bytes_received = 0; // 用于跟踪接收的字节数
    QString status, message;
    QByteArray response;

    QFileInfo info(path);

    qDebug() << "开始下载了";

    // 首先发送是下载请求
    m_tcp->write("download\n");
    m_tcp->flush();

    QThread::msleep(10); // 休眠10毫秒

    // 然后发送文件名;
    m_tcp->write(info.fileName().toUtf8());
    m_tcp->flush();

    // 等待服务器响应
    m_tcp->waitForReadyRead(-1);
    response = m_tcp->readAll();

    // 解析服务器响应, 解析 json 数据
    if(!Utility::parse_json_message(response, status, message))
    {
        qDebug() << "下载文件: 解析服务器文件名是否存在的JSON数据失败";
    }

    if(status == "error")
    {
        qDebug() << "下载文件: 文件不存在";
    }
    else if(status == "success")
    {
        // 如果有用户名存在, 将文件名中的用户名删除
        Utility::removeUsernameFromPath(path);

        // 处理文件名重复，使用加编号解决
        Utility::getUniqueFileName(path);

        // 打开文件
        QFile file(path);

        if (!file.open(QIODevice::WriteOnly))
        {
            qWarning() << "下载路径位置File open failed:" << file.errorString();
            return;
        }

        if (!m_tcp->waitForReadyRead(5000))   // 等待最多5秒
        {
            qDebug() << "下载文件:等待数据超时";
            return;
        }


        // 下载文件部分----------------------------------------------------------------------------
        int expected_file_size = 0;
        m_tcp->read(reinterpret_cast<char*>(&expected_file_size), sizeof(expected_file_size)); // 读取文件大小

        while (m_tcp->waitForReadyRead(-1))
        {
            qDebug() << expected_file_size;
            QByteArray data = m_tcp->readAll();
            int bytes = data.size();

            if (bytes > 0)
            {
                file.write(data);
                total_checksum += Utility::simple_checksum(data); // 计算当前块数据的校验和
                total_bytes_received += bytes; // 更新接收字节数

                // 计算并发送给主线程百分比
                int percent = static_cast<int>((total_bytes_received * 100) / expected_file_size);
                emit curPercent(percent);
            }

            // 根据文件大小判断是否接收完毕
            if (total_bytes_received >= expected_file_size)
            {
                break; // 文件接收完毕
            }
        }

        // 关闭文件
        file.close();

        // 通知服务器端，客户端准备好接收了
        QByteArray startMessage = "start";
        m_tcp->write(startMessage);       // 发送 "start" 消息
        m_tcp->waitForBytesWritten();     // 等待数据写入完成

        // 接收客户端传来的总体校验和
        unsigned char received_final_checksum = 0;

        if (m_tcp->waitForReadyRead()) // 等待数据可读
        {
            m_tcp->read(reinterpret_cast<char*>(&received_final_checksum), sizeof(received_final_checksum));
        }

        // 计算最终校验和的低8位
        unsigned char calculated_final_checksum = total_checksum & 0xFF;

        // 验证校验和
        if (calculated_final_checksum == received_final_checksum)
        {
            qDebug() << "下载文件:校验和匹配，文件下载成功!";
            Utility::sendJsonMessage(m_tcp, "success", "校验成功");

            // 向主线程发送当前文件下载完成信号
            emit downloadFinished(path);
        }
        else
        {
            qDebug() << "下载文件:校验和不匹配，下载的文件数据部分有误";
            Utility::sendJsonMessage(m_tcp, "error", "校验失败");
        }
    }
    else
    {
        qDebug() << "下载文件-接收文件名反馈:未知错误";
    }

    if (m_tcp)
    {
        m_tcp->close(); // 关闭连接
    }

}

