#ifndef USER_H
#define USER_H
#include <QObject>
#include <QTcpSocket>
#include <QDataStream>

class User : public QObject{
    Q_OBJECT
    QString login;
    QTcpSocket *socket;
    QDataStream stream;

public:
    explicit User(QTcpSocket *_socket);
    QString getLogin() const { return login;}
    QTcpSocket *getSocket(){ return socket;}
    QDataStream *getStream(){ return &stream;}


public slots:
    void read();
    /* Не знаю как переделать destroyed(QObject*) */
    void deleteLater(){
        emit deleteMi(this);
    }
signals:
    //Сигнал о проверке аутентификации
    void auth(User *, QString*);
    void operation(User* from, QString to, int sum);
    void deleteMi(User *);
};
#endif // USER_H
