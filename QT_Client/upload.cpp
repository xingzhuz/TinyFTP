#include "upload.h"
#include <QFile>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkProxy>
#include <QThread>
#include "utility.h"

UpLoad::UpLoad(QObject *parent) : QObject(parent)
{
}


//在 recover 信号的处理函数中设置用户选择并唤醒等待线程
void UpLoad::recover(bool is_recover)
{
    m_mutex.lock();
    m_overwriteChoice = is_recover; // 存储用户选择
    m_waitCondition.wakeOne();      // 唤醒等待的线程
    m_mutex.unlock();
}

// 连接服务器
void UpLoad::connectServer(QString username, QString path, unsigned short port, QString ip)
{
    m_tcp = new QTcpSocket;
    m_tcp->setProxy(QNetworkProxy::NoProxy);

    qDebug() << "上传文件的子线程正在连接...";
    m_tcp->connectToHost(QHostAddress(ip), port);

    // 检测是否连接成功，成功发送信号给主线程
    connect(m_tcp, &QTcpSocket::connected, this, [ = ]()
    {
        qDebug() << "连接成功";

        m_tcp->write("toupload");
        m_tcp->flush();

        QThread::msleep(10); // 休眠10毫秒

        uploadFile(username, path);
    });
}

// 上传文件的具体函数
void UpLoad::uploadFile(QString username, QString path)
{
    unsigned int total_checksum = 0; // 总体校验和
    QString status, message;
    QByteArray response;

    QFileInfo info(path);

    // 将上传的文件加上用户名
    QString newFileName = Utility::modifyFileName(info.fileName(), username);

    // 首先发送是上传请求
    qDebug() << "开始上传了";
    m_tcp->write("upload\n");
    m_tcp->flush();

    QThread::msleep(10); // 休眠10毫秒

    // 然后发送文件名
    m_tcp->write(newFileName.toUtf8());
    m_tcp->flush();

    // 等待服务器响应
    m_tcp->waitForReadyRead();
    response = m_tcp->readAll();

    if(!Utility::parse_json_message(response, status, message))  // 解析服务器响应, 解析 json 数据
    {
        qDebug() << "上传文件: 解析服务器文件名是否存在的JSON数据失败";
    }

    if (status == "success")
    {
        // 处理文件覆盖逻辑
        emit requestRecover(); // 发送信号请求覆盖

        // 阻塞等待用户选择文件是否覆盖
        m_mutex.lock();
        m_waitCondition.wait(&m_mutex); // 阻塞等待用户选择
        m_mutex.unlock();

        if (!m_overwriteChoice)
        {
            m_tcp->write("2\n");

            if (!m_tcp->waitForBytesWritten())
            {
                qDebug() << "发送选择不上传: 失败";
                return;
            }

            // 用户选择取消上传
            return;
        }
        else
        {
            m_tcp->write("1\n");

            if (!m_tcp->waitForBytesWritten())
            {
                qDebug() << "发送上传覆盖: 失败";
                return;
            }
        }
    }
    else if (status == "error")
    {
        qDebug() << "开始上传文件...";
    }

    // 上传文件部分----------------------------------------------------------------------------
    QFile file(path);

    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "上传文件: 打开文件失败";
        return;
    }

    long long totalSent = 0;               // 用于跟踪已发送字节数
    char* buffer = new char[BUFFER_SIZE];  // 使用动态内存分配创建缓冲区

    // 获取文件大小
    long long fileSize = info.size();

    while (true)
    {
        qint64 bytesRead = file.read(buffer, BUFFER_SIZE); // 读取数据到缓冲区

        if (bytesRead <= 0) // 如果没有读取到数据，结束循环
            break;

        QThread::msleep(10); // 休眠10毫秒

        // 发送数据给服务器
        m_tcp->write(buffer, bytesRead); // 发送实际读取的字节数
        totalSent += bytesRead;  // 更新已发送字节数

        // 等待数据写入完成
        m_tcp->waitForBytesWritten();

        // 计算并发送给主线程百分比
        int percent = static_cast<int>((totalSent * 100) / fileSize);
        emit curPercent(percent);

        total_checksum += Utility::simple_checksum(buffer, static_cast<unsigned int>(bytesRead)); // 累加校验和
    }

    // 释放动态分配的内存
    delete[] buffer;
    file.close();

    // 读取服务器准备接收数据的响应
    m_tcp->waitForReadyRead();
    response = m_tcp->readAll(); // 读取响应数据

    // 传输结束后，将总体校验和发送给服务器
    unsigned char final_checksum = total_checksum & 0xFF; // 取低8位
    m_tcp->write(reinterpret_cast<char*>(&final_checksum), 1);

    qDebug() << final_checksum;

    if(!m_tcp->waitForBytesWritten())
    {
        qDebug() << "上传文件: 传输总体校验和失败";
    }

    // 读取服务器端，接收完毕反馈
    m_tcp->waitForReadyRead();
    response = m_tcp->readAll(); // 读取服务器反馈
    Utility::parse_json_message(response, status, message); // 解析 json 数据

    // 根据解析结果进行处理
    if (status == "success")
    {
        qDebug() << "服务器数据校验成功，文件上传成功!";
    }
    else if (status == "error")
    {
        qDebug() << "上传文件: 服务器数据校验有误!";
    }

    // 发送给主线程，当前文件上传完成
    emit uploadFinished();

    if (m_tcp)
    {
        m_tcp->close(); // 关闭连接
    }
}


