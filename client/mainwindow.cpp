#include "mainwindow.h"
#include "../message.h"
#include <QInputDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this)), isConnectedToChat(false)
{
    centralWidget = new QWidget(this);

    chatBox = new QTextEdit(this);
    chatBox->setReadOnly(true);

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message or /help for available commands");
    sendButton = new QPushButton("Send", this);

    userList = new QListWidget(this);

    // Prompt for username
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("Username");
    inputDialog.setLabelText("Enter your name:");
    inputDialog.setTextValue("");
    inputDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, false);
    inputDialog.setOkButtonText("OK");
    inputDialog.setCancelButtonText("Continue with Anonymous");

    int result = inputDialog.exec();
    if (result == QDialog::Accepted) {
        username = inputDialog.textValue();
        if (username.isEmpty())
            username = "Anonymous";
    } else {
        username = "Anonymous";
    }

    // Welcome label
    QLabel *welcomeLabel = new QLabel(QString("Welcome %1!").arg(username), this);

    clearChatButton = new QPushButton("Clear Chat", this);
    clearChatButton->setToolTip("Clear Chat");

    // Row: welcome label + clear button
    QHBoxLayout *topRowLayout = new QHBoxLayout;
    topRowLayout->addWidget(welcomeLabel);
    topRowLayout->addWidget(clearChatButton, 0, Qt::AlignRight);

    // Input row: message input + send button
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    // Left column: top row + chat box + input row
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topRowLayout);
    leftLayout->addWidget(chatBox, 1);
    leftLayout->addLayout(inputLayout, 0);

    // Main layout: left (chat+input) and right (user list)
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addWidget(userList, 1);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    socket->connectToHost("127.0.0.1", 12345);

    connect(socket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "[Client] Socket connected, sending registration message";
        Message regMsg(username, "", Message::Registration);
        QByteArray regData = regMsg.toBinary();
        QByteArray regPacket;
        QDataStream regOut(&regPacket, QIODevice::WriteOnly);
        regOut << (quint32)regData.size();
        regPacket.append(regData);
        socket->write(regPacket);
        socket->flush();
        qDebug() << "[Client] Registration message sent, bytes:" << regPacket.size();
    });

    // Connect signals and slots (only once)
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(clearChatButton, &QPushButton::clicked, chatBox, &QTextEdit::clear);
    connect(socket, &QTcpSocket::connected, this, [](){ qDebug() << "[Client] Socket connected"; });
    connect(socket, &QTcpSocket::disconnected, this, [](){ qDebug() << "[Client] Socket disconnected"; });
    connect(socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError err){
        qDebug() << "[Client] Socket error:" << err;
    });
}

MainWindow::~MainWindow() {}

void MainWindow::onSendClicked()
{
    QString messageText = messageInput->text();
    if (messageText.isEmpty() || socket->state() != QAbstractSocket::ConnectedState)
        return;

    // Handle client-side commands
    if (messageText == "/help") {
        chatBox->append("<i>Available commands:</i>");
        chatBox->append("<i>/help - Show this help message (client-side only)</i>");
        chatBox->append("<i>/clear - Clear the chat window (client-side only)</i>");
        chatBox->append("<i>Other commands will be sent to the server.</i>");
        messageInput->clear();
        return;
    }
    if (messageText == "/clear") {
        chatBox->clear();
        messageInput->clear();
        return;
    }

    // All other messages (including /something) go to the server
    Message msg(username, messageText, Message::Text);
    QByteArray data = msg.toBinary();
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out << (quint32)data.size();
    packet.append(data);
    socket->write(packet);
    socket->flush();
    messageInput->clear();
}

void MainWindow::onReadyRead()
{
    static QByteArray buffer;
    buffer.append(socket->readAll());

    // Read messages with size prefix
    while (buffer.size() >= 4)
    {
        QDataStream in(buffer);
        quint32 msgSize;
        in >> msgSize;
        if (buffer.size() < 4 + msgSize)
            break;
        QByteArray msgData = buffer.mid(4, msgSize);
        Message msg = Message::fromBinary(msgData);

        // Check for user list update
        if (msg.username == "System" && msg.text.startsWith("USERLIST|"))
        {
            QStringList users = msg.text.mid(QString("USERLIST|").length()).split(",", Qt::SkipEmptyParts);
            userList->clear();
            if (!users.isEmpty() && !(users.size() == 1 && users[0].isEmpty()))
                userList->addItems(users);
        }
        else if (msg.type == Message::System)
        {
            chatBox->append(QString("<i>[%1] %2</i>")
                .arg(msg.timestamp.toString("hh:mm:ss"))
                .arg(msg.text.toHtmlEscaped()));
        }
        else
        {
            chatBox->append("[" + msg.timestamp.toString("hh:mm:ss") + "] " + msg.username + ": " + msg.text);
        }
        buffer = buffer.mid(4 + msgSize);
    }
}
