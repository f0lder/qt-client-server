#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onReadyRead();

private:
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QTextEdit *chatBox;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QTcpSocket *socket;
};
