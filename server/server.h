#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    ChatServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readClient();

private:
    QList<QTcpSocket *> clients;
};
