#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this))
{
    centralWidget = new QWidget(this);
    layout = new QVBoxLayout(centralWidget);

    chatBox = new QTextEdit(this);
    chatBox->setReadOnly(true);

    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);

    layout->addWidget(chatBox);
    layout->addWidget(messageInput);
    layout->addWidget(sendButton);

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    socket->connectToHost("127.0.0.1", 12345);

    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
}

MainWindow::~MainWindow() {}

void MainWindow::onSendClicked()
{
    QString message = messageInput->text();
    if (!message.isEmpty())
    {
        socket->write(message.toUtf8());
        socket->flush();
        messageInput->clear();
    }
}

void MainWindow::onReadyRead()
{
    QString received = QString::fromUtf8(socket->readAll());
    chatBox->append("Server: " + received);
}
