#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include "../message.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    ChatServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readClient();
    void broadcastUserList();

private:
    void sendToAllClients(const QByteArray& packet);

    QList<QTcpSocket *> clients;
};
