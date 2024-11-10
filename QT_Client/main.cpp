#include "mainwindow.h"
#include "mainmenu.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // 创建 MainMenu 窗口并显示
    MainMenu *mainMenu = new MainMenu(nullptr);

    // 连接 MainWindow 的 windowClosed 信号到 MainMenu 的 show 槽
    QObject::connect(&w, &MainWindow::windowClosed, [ = ](QString username)
    {
        mainMenu->m_username = username;
        mainMenu->show();
    });

    return a.exec();
}
