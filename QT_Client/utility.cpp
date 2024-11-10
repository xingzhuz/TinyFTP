#include "utility.h"

Utility::Utility(QObject *parent) : QObject(parent)
{

}


// 发送JSON数据
void Utility::sendJsonMessage(QTcpSocket *socket, const QString &status, const QString &message)
{
    if (!socket || !socket->isOpen())
    {
        qDebug() << "Socket is not open";
        return;
    }

    // 创建 JSON 对象
    QJsonObject jsonObj;
    jsonObj["status"] = status;
    jsonObj["message"] = message;

    // 将 JSON 对象转换为 JSON 文档
    QJsonDocument jsonDoc(jsonObj);

    // 转换为 QByteArray 格式，便于传输
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact);

    // 将 JSON 数据发送到套接字
    socket->write(jsonData);

    // 刷新套接字（确保数据立即发送）
    socket->flush();
}



// 解析 JSON 数据的函数
bool Utility::parse_json_message(QByteArray &jsonData, QString &status, QString &message)
{
    // 去掉 null 字符
    jsonData.remove(jsonData.indexOf('\0'), jsonData.size() - jsonData.indexOf('\0'));

    // 解析 JSON 文档
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    // 检查是否解析成功
    if (doc.isNull())
    {
        qDebug() << "解析 JSON 失败";
        status = "error";
        message = "解析失败";
        return false;
    }

    // 检查 JSON 是否是对象
    if (!doc.isObject())
    {
        qDebug() << "JSON 数据不是对象";
        status = "error";
        message = "数据格式错误";
        return false;
    }

    QJsonObject jsonObj = doc.object();

    // 提取状态和消息
    if (jsonObj.contains("status"))
    {
        QJsonValue statusValue = jsonObj.value("status");

        if (statusValue.isString())
        {
            status = statusValue.toString();
        }
        else
        {
            status = "error";
            message = "状态字段格式错误";
            return false;
        }
    }
    else
    {
        status = "error";
        message = "缺少状态字段";
        return false;
    }

    if (jsonObj.contains("message"))
    {
        QJsonValue messageValue = jsonObj.value("message");

        if (messageValue.isString())
        {
            message = messageValue.toString();
        }
        else
        {
            message = "消息字段格式错误";
        }
    }
    else
    {
        message = "缺少消息字段";
    }

    return true;
}



// 简单校验和函数
unsigned char Utility::simple_checksum(const char *data, unsigned int length)
{
    unsigned int checksum = 0;

    for (unsigned int i = 0; i < length; i++)
    {
        checksum += data[i];
    }

    return checksum & 0xFF; // 返回低8位
}

unsigned char Utility::simple_checksum(const QByteArray &data)
{
    unsigned int checksum = 0;

    for (char byte : data)
    {
        checksum += static_cast<unsigned char>(byte);
    }

    return checksum & 0xFF; // 返回低8位
}


// 获取唯一编号的文件名，解决文件同名问题
void Utility::getUniqueFileName(QString &path)
{
    // 分离路径和文件名
    QString directory = QFileInfo(path).absolutePath();  // 获取路径
    QString filename = QFileInfo(path).fileName();       // 获取文件名

    QString baseName = filename;
    QString extension;
    int index = filename.lastIndexOf('.');

    if (index != -1)  // 分离文件名和扩展名
    {
        baseName = filename.left(index);
        extension = filename.mid(index);
    }

    QString fullPath = path;  // 初始完整路径
    int counter = 1;

    // 如果文件已存在，则按 (1), (2), (3) 等格式生成唯一文件名
    while (QFile::exists(fullPath))
    {
        fullPath = directory + "/" + baseName + "(" + QString::number(counter) + ")" + extension;
        counter++;
    }

    path = fullPath;  // 更新原路径为唯一文件名
}

// 去除文件名中的编号的函数
QString Utility::removeFileNumbering(const QString &filename)
{
    QString baseName = filename;
    QString extension;
    int index = filename.lastIndexOf('.');

    if (index != -1)    // 分离文件名和扩展名
    {
        baseName = filename.left(index);
        extension = filename.mid(index);
    }

    // 使用正则表达式匹配形如 "(1)", "(2)" 的编号
    QRegularExpression numberingPattern(R"(\(\d+\)$)");
    baseName = baseName.remove(numberingPattern);

    return baseName + extension;
}

// 去除文件中用户名的函数
void Utility::removeUsernameFromPath(QString &path)
{
    // 查找扩展名的起始位置
    int dotPos = path.lastIndexOf('.');

    if (dotPos == -1)
    {
        // 如果没有找到扩展名，直接返回
        return;
    }

    // 在扩展名前查找第一个 '(' 的位置
    int openBracketPos = path.lastIndexOf('(', dotPos);

    if (openBracketPos == -1)
    {
        // 如果没有找到 '('，直接返回
        return;
    }

    // 查找对应的 ')' 的位置
    int closeBracketPos = path.indexOf(')', openBracketPos);

    if (closeBracketPos == -1)
    {
        // 如果没有找到对应的 ')'，直接返回
        return;
    }

    // 删除括号及其中的内容
    path.remove(openBracketPos, closeBracketPos - openBracketPos + 1);
}

// 将上传的文件加上用户名
QString Utility::modifyFileName(const QString &originalFileName, const QString &username)
{
    QString baseName = originalFileName;
    QString extension;
    int index = originalFileName.lastIndexOf('.');

    if (index != -1)
    {
        baseName = originalFileName.left(index);  // 获取不带扩展名的文件名部分
        extension = originalFileName.mid(index);  // 获取扩展名部分
    }

    // 生成新的文件名
    QString newFileName = baseName + "(" + username + ")" + extension;
    return newFileName;
}
