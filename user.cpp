#include "user.h"

User::User(QTcpSocket *_socket){
    socket = _socket;
    stream.setDevice(socket);
    connect(socket, SIGNAL(readyRead()), this, SLOT(read()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater())); //При рассоединении отключается
}

void User::read(){
    enum com{AUTH, BLNC, OPRTN};
    int command;
    stream >> command;
    switch (command) {
    case AUTH:
    {
        QString *password = new QString;
        stream >> login;
        stream >> *password;
        //qDebug() << "authorization" << login;
        emit auth(this, password);
    }
        break;

    case OPRTN:
    {
        QString to;
        int sum;
        stream >> to;
        stream >> sum;
        emit operation(this, to, sum);
    }
        break;

    default:
        //qDebug() << "Неизвестная комманда от" << login << command;
        break;
    }

}
