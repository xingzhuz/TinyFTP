#ifndef UTILITY_H
#define UTILITY_H

#include <QObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#define BUFFER_SIZE (1024 * 8)

class Utility : public QObject
{
    Q_OBJECT
public:
    explicit Utility(QObject *parent = nullptr);

    // 发送 JSON 数据
    static void sendJsonMessage(QTcpSocket *socket, const QString &status, const QString &message);

    // 解析 JSON 数据
    static bool parse_json_message(QByteArray &jsonData, QString &status, QString &message);

    // 简单的校验和实现
    static unsigned char simple_checksum(const char *data, unsigned int length);
    static unsigned char simple_checksum(const QByteArray &data);

    // 获取唯一编号的文件名，解决文件同名问题
    static void getUniqueFileName(QString &path);

    // 去除文件名中的编号的函数
    static QString removeFileNumbering(const QString &filename);

    // 去除文件中用户名的函数
    static void removeUsernameFromPath(QString &path);

    // 将上传的文件加上用户名
    static QString modifyFileName(const QString &originalFileName, const QString &username);
};

#endif // UTILITY_H
