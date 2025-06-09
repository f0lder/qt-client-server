#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.setWindowTitle("Chat Client");
    window.resize(400, 300);
    window.show();
    return app.exec();
}
