#pragma once
#include <QString>
#include <QDateTime>
#include <QDataStream>
#include <QIODevice>

class Message {
public:
    enum Type {
        Text = 0,
        Registration = 1,
        Command = 2,
        System = 3,
        Typing = 4
    };

    QString username;
    QString text;
    QDateTime timestamp;
    Type type = Text;

    Message() = default;
    Message(const QString& user, const QString& msg, Type t = Text)
        : username(user), text(msg), timestamp(QDateTime::currentDateTime()), type(t) {}

    QString serialize() const {
        // Format: type|username|timestamp|text
        return QString::number(type) + "|" + username + "|" + timestamp.toString(Qt::ISODate) + "|" + text + "\n";
    }

    // Binary serialization
    QByteArray toBinary() const {
        QByteArray arr;
        QDataStream out(&arr, QIODevice::WriteOnly);
        out << static_cast<qint32>(type) << username << timestamp << text;
        return arr;
    }

    static Message fromBinary(const QByteArray& arr) {
        Message msg;
        QDataStream in(arr);
        qint32 t;
        in >> t >> msg.username >> msg.timestamp >> msg.text;
        msg.type = static_cast<Type>(t);
        return msg;
    }
};