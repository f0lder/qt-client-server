#include "server.h"
#include "../message.h"
#include <QDebug>
#include <QDataStream>
#include <QMap>

ChatServer::ChatServer(QObject *parent) : QTcpServer(parent)
{
    if (!listen(QHostAddress::Any, 12345))
        qDebug() << "Server could not start!";
    else
        qDebug() << "Server started on port" << serverPort();
}

// Track usernames for each client
static QMap<QTcpSocket *, QString> usernames;
static QMap<QTcpSocket *, QString> userStatus; // "online", "afk", "typing"

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);
    clients.append(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead, this, &ChatServer::readClient);
    connect(clientSocket, &QTcpSocket::disconnected, [this, clientSocket]()
            {
        clients.removeAll(clientSocket);
        bool wasRegistered = usernames.contains(clientSocket);
        usernames.remove(clientSocket);
        userStatus.remove(clientSocket);

        // Send "user disconnects" system message to all clients if registered
        if (wasRegistered) {
            Message sysMsg;
            sysMsg.type = Message::System;
            sysMsg.username = "System";
            sysMsg.text = QString("A user has disconnected. Total users: %1").arg(usernames.size());
            sysMsg.timestamp = QDateTime::currentDateTime();

            QByteArray data = sysMsg.toBinary();
            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);
            out << (quint32)data.size();
            packet.append(data);
            sendToAllClients(packet);

            broadcastUserList();
        }

        clientSocket->deleteLater();
        qDebug() << "[Server] Client disconnected"; });

    qDebug() << "[Server] New client connected:" << socketDescriptor;
}

// Helper to broadcast the user list to all clients
void ChatServer::broadcastUserList()
{
    QStringList userEntries;
    for (QTcpSocket *c : clients)
    {
        if (usernames.contains(c)) {
            QString status = userStatus.value(c, "online");
            userEntries << QString("%1|%2").arg(usernames.value(c, "Anonymous"), status);
        }
    }
    Message userListMsg;
    userListMsg.type = Message::System;
    userListMsg.username = "System";
    userListMsg.text = "USERLIST|" + userEntries.join(",");
    userListMsg.timestamp = QDateTime::currentDateTime();

    QByteArray data = userListMsg.toBinary();
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out << (quint32)data.size();
    packet.append(data);
    sendToAllClients(packet);
    qDebug() << "[Server] Broadcasted user list:" << userEntries;
}

void ChatServer::readClient()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    static QMap<QTcpSocket *, QByteArray> buffers;

    QByteArray incoming = client->readAll();
    qDebug() << "[Server] Read" << incoming.size() << "bytes from client" << client;
    if (incoming.isEmpty())
    {
        qDebug() << "[Server] No data received, client may have disconnected.";
    }
    buffers[client].append(incoming);
    qDebug() << "[Server] Buffer size for client" << client << ":" << buffers[client].size();

    while (true)
    {
        if (buffers[client].size() < 4)
        {
            qDebug() << "[Server] Not enough data for size prefix, waiting for more.";
            break;
        }
        QDataStream in(buffers[client]);
        quint32 msgSize;
        in >> msgSize;
        qDebug() << "[Server] Next message size:" << msgSize << "Buffer size:" << buffers[client].size();
        if (buffers[client].size() < 4 + msgSize)
        {
            qDebug() << "[Server] Not enough data for full message, waiting for more.";
            break;
        }
        QByteArray msgData = buffers[client].mid(4, msgSize);
        Message msg = Message::fromBinary(msgData);

        qDebug() << "[Server] Parsed message:"
                 << "type:" << msg.type
                 << "username:" << msg.username
                 << "text:" << msg.text;

        // Registration logic
        if (!usernames.contains(client))
        {
            if (msg.type == Message::Registration)
            {
                usernames[client] = msg.username;
                userStatus[client] = "online";
                qDebug() << "[Server] Registered new user:" << msg.username;

                // Send "user connects" system message to all clients
                Message sysMsg;
                sysMsg.type = Message::System;
                sysMsg.username = "System";
                sysMsg.text = QString("A user has connected. Total users: %1").arg(usernames.size());
                sysMsg.timestamp = QDateTime::currentDateTime();

                QByteArray data = sysMsg.toBinary();
                QByteArray packet;
                QDataStream out(&packet, QIODevice::WriteOnly);
                out << (quint32)data.size();
                packet.append(data);
                sendToAllClients(packet);

                broadcastUserList();
            }
            else
            {
                qDebug() << "[Server] Ignored non-registration message from unregistered client. Type:" << msg.type;
            }
            // Remove processed bytes
            buffers[client] = buffers[client].mid(4 + msgSize);
            continue;
        }

        // Only process chat messages from registered users
        if (msg.type == Message::Text)
        {
            qDebug() << "[Server] Received message from" << msg.username << ":" << msg.text;
            userStatus[client] = "online";
            broadcastUserList();

            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);
            out << (quint32)msgData.size();
            packet.append(msgData);
            sendToAllClients(packet);
        }
        else if (msg.type == Message::Typing)
        {
            qDebug() << "[Server] Typing notification from" << msg.username;
            userStatus[client] = "typing";
            broadcastUserList();

            // Prepare the typing message packet
            QByteArray typingData = msgData;
            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);
            out << (quint32)typingData.size();
            packet.append(typingData);

            // Broadcast to all except the sender
            for (QTcpSocket *c : clients)
            {
                if (c != client && usernames.contains(c))
                {
                    c->write(packet);
                }
            }
            buffers[client] = buffers[client].mid(4 + msgSize);
            continue;
        }
        else if (msg.type == Message::Status)
        {
            qDebug() << "[Server] Status update from" << msg.username << ":" << msg.text;
            userStatus[client] = msg.text; // "online", "afk", etc.
            broadcastUserList();
            // No need to forward this message to other clients directly
            buffers[client] = buffers[client].mid(4 + msgSize);
            continue;
        }
        else
        {
            qDebug() << "[Server] Ignored non-text message from" << msg.username << " of type " << msg.type;
        }

        // Remove processed bytes
        buffers[client] = buffers[client].mid(4 + msgSize);
        qDebug() << "[Server] Buffer size after processing:" << buffers[client].size();
    }
}

void ChatServer::sendToAllClients(const QByteArray &packet)
{
    for (QTcpSocket *c : clients)
    {
        if (usernames.contains(c))
        { // Only send to registered clients
            c->write(packet);
            qDebug() << "[Server] Sent packet to client" << c;
        }
        else
        {
            qDebug() << "[Server] Skipped sending to unregistered client" << c;
        }
    }
}
