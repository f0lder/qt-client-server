#include "server.h"
#include <QDebug>

ChatServer::ChatServer(QObject *parent) : QTcpServer(parent)
{
    if (!listen(QHostAddress::Any, 12345))
    {
        qDebug() << "Server could not start!";
    }
    else
    {
        qDebug() << "Server started on port" << serverPort();
    }
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);
    clients.append(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead, this, &ChatServer::readClient);
    connect(clientSocket, &QTcpSocket::disconnected, [=]()
            {
        clients.removeAll(clientSocket);
        clientSocket->deleteLater(); });

    qDebug() << "New client connected:" << socketDescriptor;
}

void ChatServer::readClient()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    QByteArray data = client->readAll();

    for (QTcpSocket *c : clients)
    {
        if (c != client)
        {
            c->write(data);
        }
    }
}
