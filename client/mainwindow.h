#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QIcon>
#include <QTimer>
#include "../message.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onReadyRead();
    QString sanitizeHtml(const QString& input);

private:
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QTextEdit *chatBox;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QTcpSocket *socket;
    QPushButton *clearChatButton;
    QString username;
    QListWidget* userList;
    QLabel *typingLabel;
    bool isConnectedToChat = false;
    QIcon statusIcon(const QColor &color);
    QTimer *typingResetTimer;
    QTimer *afkTimer;
};
