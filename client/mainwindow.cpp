#include "mainwindow.h"
#include "../message.h"
#include <QInputDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QColor>

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
    if (result == QDialog::Accepted)
    {
        username = inputDialog.textValue();
        if (username.isEmpty())
            username = "Anonymous";
    }
    else
    {
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

    // interesant ca connected este un slot, in continuare al folosit o functie lambda pentru a trimite un mesaj de inregistrare
    connect(socket, &QTcpSocket::connected, this, [this]()
            {
        qDebug() << "[Client] Socket connected, sending registration message";
        Message regMsg(username, "", Message::Registration);
        QByteArray regData = regMsg.toBinary();
        QByteArray regPacket;
        QDataStream regOut(&regPacket, QIODevice::WriteOnly);
        regOut << (quint32)regData.size();
        regPacket.append(regData);
        socket->write(regPacket);
        socket->flush();
        qDebug() << "[Client] Registration message sent, bytes:" << regPacket.size(); });

    // Connect signals and slots (only once)
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(clearChatButton, &QPushButton::clicked, chatBox, &QTextEdit::clear);

    // Handle typing indication
    connect(messageInput, &QLineEdit::textEdited, this, [this](const QString &text)
            {
        if (text.startsWith('/')) return;
        if (socket->state() == QAbstractSocket::ConnectedState) {
            Message typingMsg(username, "", Message::Typing);
            QByteArray data = typingMsg.toBinary();
            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);
            out << (quint32)data.size();
            packet.append(data);
            socket->write(packet);
            socket->flush();
        }
        typingResetTimer->start(2000); // 2 seconds after last keypress
        afkTimer->start(60000); // 60 seconds after last activity
    });

    typingLabel = new QLabel(this);
    leftLayout->addWidget(typingLabel);

    typingResetTimer = new QTimer(this);
    typingResetTimer->setSingleShot(true);
    connect(typingResetTimer, &QTimer::timeout, this, [this]() {
        // Send "online" status if not typing anymore
        if (socket->state() == QAbstractSocket::ConnectedState) {
            Message onlineMsg(username, "", Message::Status);
            onlineMsg.text = "online";
            QByteArray data = onlineMsg.toBinary();
            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);
            out << (quint32)data.size();
            packet.append(data);
            socket->write(packet);
            socket->flush();
        }
    });

    afkTimer = new QTimer(this);
    afkTimer->setSingleShot(true);
    connect(afkTimer, &QTimer::timeout, this, [this]() {
        // Send "afk" status
        if (socket->state() == QAbstractSocket::ConnectedState) {
            Message afkMsg(username, "", Message::Status);
            afkMsg.text = "afk";
            QByteArray data = afkMsg.toBinary();
            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);
            out << (quint32)data.size();
            packet.append(data);
            socket->write(packet);
            socket->flush();
        }
    });
}

MainWindow::~MainWindow() {}

void MainWindow::onSendClicked()
{
    QString messageText = messageInput->text();
    if (messageText.isEmpty() || socket->state() != QAbstractSocket::ConnectedState)
        return;

    // Handle client-side commands
    if (messageText == "/help")
    {
        chatBox->append("<i>Available commands:</i>");
        chatBox->append("<i>/help - Show this help message (client-side only)</i>");
        chatBox->append("<i>/clear - Clear the chat window (client-side only)</i>");
        chatBox->append("<i>Other commands will be sent to the server.</i>");
        messageInput->clear();
        return;
    }
    if (messageText == "/clear")
    {
        chatBox->clear();
        messageInput->clear();
        return;
    }
    // Unknown client-side command (starts with / but not recognized)
    if (messageText.startsWith('/'))
    {
        chatBox->append(QString("<i>Unknown command: %1</i>").arg(messageText.toHtmlEscaped()));
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

    afkTimer->start(60000); // Reset AFK timer on send
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
            for (const QString &entry : users) {
                QStringList parts = entry.split("|");
                QString uname = parts.value(0);
                QString status = parts.value(1, "online"); // default to online

                QColor color = Qt::green;
                if (status == "afk") color = QColor("#FFD600"); // yellow
                else if (status == "typing") color = QColor("#2196F3"); // blue

                QListWidgetItem *item = new QListWidgetItem(statusIcon(color), uname);
                userList->addItem(item);
            }
        }
        else if (msg.type == Message::System)
        {
            chatBox->append(QString("<i>[%1] %2</i>")
                                .arg(msg.timestamp.toString("hh:mm:ss"))
                                .arg(msg.text.toHtmlEscaped()));
        }
        else if (msg.type == Message::Typing)
        {
            // Show typing indicator (e.g., in the window title or a label)
            typingLabel->setText(QString("%1 is typing...").arg(msg.username));
            // Hide after a short delay
            QTimer::singleShot(1500, this, [this]()
                               { typingLabel->clear(); });
        }
        else
        {
            chatBox->append("[" + msg.timestamp.toString("hh:mm:ss") + "] " + msg.username + ": " + sanitizeHtml(msg.text));
        }
        buffer = buffer.mid(4 + msgSize);
    }
}

// Allow only <b>, <i>, <u>, <br> tags
QString MainWindow::sanitizeHtml(const QString& input) {
    QString safe = input.toHtmlEscaped();

    // Unescape allowed tags
    safe.replace("&lt;b&gt;", "<b>", Qt::CaseInsensitive);
    safe.replace("&lt;/b&gt;", "</b>", Qt::CaseInsensitive);
    safe.replace("&lt;i&gt;", "<i>", Qt::CaseInsensitive);
    safe.replace("&lt;/i&gt;", "</i>", Qt::CaseInsensitive);
    safe.replace("&lt;u&gt;", "<u>", Qt::CaseInsensitive);
    safe.replace("&lt;/u&gt;", "</u>", Qt::CaseInsensitive);
    safe.replace("&lt;br&gt;", "<br>", Qt::CaseInsensitive);

    return safe;
}

// Helper to create a colored circle icon for status
QIcon MainWindow::statusIcon(const QColor &color) {
    QPixmap pix(12, 12);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, 12, 12);
    return QIcon(pix);
}
