#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QTcpSocket>
#include <QLabel>
#include "upload.h"
#include <QCloseEvent>
#include "downloadfile.h"
#include "listfile.h"


namespace Ui
{
class MainMenu;
}

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);
    ~MainMenu();
    QString m_username;


signals:

    void startUpload(QString username, QString path, unsigned short port, QString ip);
    void startDownload(QString path, unsigned short port, QString ip);
    void startList(unsigned short port, QString ip);

    void send(bool flag);


private slots:
    void on_upload_clicked();

    void on_download_clicked();

    void on_listFile_clicked();

    void on_chooseFile_clicked();

    void on_chooseLoaction_clicked();

    void showFiles(QString fileList);


private:
    Ui::MainMenu *ui;
    QThread *upload;         // 上传文件的子线程
    UpLoad *upload_worker;   // 上传文件的任务函数对象

    QThread *download;              // 下载文件的子线程
    DownloadFile *download_worker;  // 下载文件的任务函数对象

    QThread *list;          // 列出服务器文件的子线程
    ListFile *list_worker;  // 列出服务器文件的任务函数对象


};

#endif // MAINMENU_H
