#include "mainmenu.h"
#include "ui_mainmenu.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include "listfile.h"
#include "utility.h"


MainMenu::MainMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    setWindowTitle("主界面");

    // 初始化
    ui->upload_progressBar->setRange(0, 100);      // 进度条范围0~100
    ui->upload_progressBar->setValue(0);           // 进度条初始化为 0
    ui->download_progressBar->setRange(0, 100);
    ui->download_progressBar->setValue(0);
    ui->upload_label->setAlignment(Qt::AlignCenter);  // 设置为居中对齐
    ui->download_label_2->setAlignment(Qt::AlignCenter);
    ui->filename_label->setAlignment(Qt::AlignCenter);
    ui->files_label->setAlignment(Qt::AlignCenter);

    // 按钮点击的初始状态
    ui->upload->setEnabled(false);      // 上传路径没选择，不能点击上传文件
    ui->download->setEnabled(false);    // 文件名和下载路径没选择，不能点击下载文件

    // 初始化输入框高度
    ui->upload_edit->setFixedHeight(40);
    ui->location_edit->setFixedHeight(40);
    ui->filename_edit->setFixedHeight(40);
    ui->upload_progressBar->setFixedHeight(40);
    ui->download_progressBar->setFixedHeight(40);
    ui->listFile->setFixedHeight(40);

    // 上传文件部分----------------------------------------------------------------------------------
    upload = new QThread;          // 创建上传文件的子线程对象
    upload_worker = new UpLoad;    // 创建上传文件的任务对象

    // 任务对象移动到子线程
    upload_worker->moveToThread(upload);

    // 接收主线程发送文件的信号，首先进行连接服务器，然后发送文件
    connect(this, &MainMenu::startUpload, upload_worker, &UpLoad::connectServer);

    // 接收子线程发来的进度条信号，更新进度条
    connect(upload_worker, &UpLoad::curPercent, ui->upload_progressBar, &QProgressBar::setValue);

    // 处理文件发送完成信号
    connect(upload_worker, &UpLoad::uploadFinished, this, [ = ]()
    {
        ui->upload_progressBar->setValue(0);  // 重置进度条
        ui->upload_edit->clear();            // 清空文件路径输入框
        QMessageBox::information(this, "文件发送", "文件发送完毕，可以选择新文件发送");
    });

    // 处理上传文件是否覆盖
    connect(upload_worker, &UpLoad::requestRecover, this, [ = ]()
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "文件覆盖确认", "服务器上已有同名文件，是否覆盖？",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            emit upload_worker->recover(true); // 发送信号表示用户选择覆盖
        }
        else
        {
            emit upload_worker->recover(false); // 发送信号表示用户选择取消
        }
    });

    // 上传的子线程开始工作
    upload->start();



    // 下载文件部分------------------------------------------------------------------------------
    download = new QThread;                // 创建下载文件的子线程对象
    download_worker = new DownloadFile;    // 创建下载文件的任务对象

    // 任务对象移动到子线程
    download_worker->moveToThread(download);

    // 接收主线程下载文件的信号，首先进行连接服务器，然后下载文件
    connect(this, &MainMenu::startDownload, download_worker, &DownloadFile::connectServer);

    // 接收子线程发来的进度条信号，更新进度条
    connect(download_worker, &DownloadFile::curPercent, ui->download_progressBar, &QProgressBar::setValue);

    // 处理文件发送完成信号
    connect(download_worker, &DownloadFile::downloadFinished, this, [ = ](QString path)
    {
        ui->download_progressBar->setValue(0);  // 重置进度条
        ui->location_edit->clear();             // 清空文件名输入框
        ui->filename_edit->clear();             // 清空文件位置输入框
        QMessageBox::information(this, "下载文件", "文件下载成功，文件在: " + path);
    });

    // 下载的子线程开始工作
    download->start();


    // 列出服务器文件部分-------------------------------------------------------------
    list = new QThread;               // 创建子线程对象
    list_worker = new ListFile;       // 创建任务对象

    // 任务对象移动到子线程
    list_worker->moveToThread(list);

    // 接收主线程列出文件的信号，首先进行连接服务器，然后列出文件
    connect(this, &MainMenu::startList, list_worker, &ListFile::connectServer);

    // 接收子线程发送的文件清单，然后将其显示在表格中
    connect(list_worker, &ListFile::dataList, this, &MainMenu::showFiles);

    // 列出文件的子线程开始工作
    list->start();
}


// 上传文件按钮的槽函数
void MainMenu::on_upload_clicked()
{
    emit startUpload(m_username, ui->upload_edit->text(), 8091, "192.168.208.128");
}

// 下载文件按钮的槽函数
void MainMenu::on_download_clicked()
{
    emit startDownload(ui->location_edit->text(), 8091, "192.168.208.128");
}

// 查看服务器文件按钮的槽函数
void MainMenu::on_listFile_clicked()
{
    emit startList(8091, "192.168.208.128");
}

// 选择文件上传的按钮槽函数
void MainMenu::on_chooseFile_clicked()
{
    // 获取选择的文件路径
    QString path = QFileDialog::getOpenFileName();

    if (path.isEmpty())
    {
        QMessageBox::warning(this, "打开文件", "选择的文件路径不能为空!");
        return;
    }

    // 将上传文件的文本框赋值
    ui->upload_edit->setText(path);

    // 此时允许点击上传文件按钮
    ui->upload->setEnabled(true);
}

// 选择下载位置的槽函数
void MainMenu::on_chooseLoaction_clicked()
{
    // 获取选择的目录路径
    QString path = QFileDialog::getExistingDirectory(this, "选择目录");

    if (path.isEmpty())
    {
        QMessageBox::warning(this, "打开目录", "选择的目录路径不能为空!");
        return;
    }

    // 将选择的目录路径设置到 location_edit
    ui->location_edit->setText(path);

    // 获取 filename_edit 和 location_edit 中的内容
    QString filename = ui->filename_edit->text();
    QString location = ui->location_edit->text();

    // 拼接内容
    QString combinedPath = location + "/" + filename; // 添加 "/" 以构建有效路径

    // 将拼接结果设置回 location_edit
    ui->location_edit->setText(combinedPath);

    // 此时能够点击下载文件了
    ui->download->setEnabled(true);
}

// 将服务器文件显示在窗口
void MainMenu::showFiles(QString fileList)
{
    // 清空表格
    ui->tableWidget->setRowCount(0);    // 清空行
    ui->tableWidget->setColumnCount(3); // 设置列数

    // 设置表头
    QStringList headers;
    headers << "文件名" << "大小(KB)" << "修改日期";
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // 按行显示文件信息
    QStringList fileRecords = fileList.split("\n", QString::SkipEmptyParts); // 分割每条记录

    foreach (const QString &record, fileRecords)
    {
        QStringList fileInfo = record.split("\t"); // 按制表符分割文件名、大小和修改日期

        if (fileInfo.size() == 3) // 确保格式正确
        {
            int rowCount = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(rowCount); // 插入新行

            // 文件名（不需要对齐设置，使用默认左对齐）
            ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(fileInfo[0]));

            // 文件大小转换为KB
            QString sizeString = fileInfo[1];
            sizeString.replace(" bytes", ""); // 去掉 " bytes" 字符串

            bool ok;
            qint64 fileSize = sizeString.toLongLong(&ok); // 将字符串转换为长整型

            if (ok)
            {
                QString sizeInKB = QString::number(fileSize / 1024.0, 'f', 2); // 转换为KB并保留两位小数
                QTableWidgetItem *sizeItem = new QTableWidgetItem(sizeInKB + " KB");
                sizeItem->setTextAlignment(Qt::AlignCenter); // 居中对齐
                ui->tableWidget->setItem(rowCount, 1, sizeItem);
            }
            else
            {
                ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem("N/A")); // 如果转换失败，显示"N/A"
            }

            // 修改日期（居中对齐）
            QTableWidgetItem *dateItem = new QTableWidgetItem(fileInfo[2]);
            dateItem->setTextAlignment(Qt::AlignCenter); // 居中对齐
            ui->tableWidget->setItem(rowCount, 2, dateItem);

            // 设置行高度
            ui->tableWidget->setRowHeight(rowCount, 30); // 设置每行高度为30
        }
    }

    // 自适应列宽
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true); // 拉伸最后一列以填满空间
}

// 析构函数，释放资源
MainMenu::~MainMenu()
{
    upload->quit();
    upload->wait();
    download->quit();
    download->wait();
    list->quit();
    list->wait();

    delete upload_worker;
    delete upload;
    delete download;
    delete download_worker;
    delete list;
    delete list_worker;

    delete ui;
}
