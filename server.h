#ifndef SERVER_H
#define SERVER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>

#include <user.h>

class Server : public QTcpServer
{
    Q_OBJECT
    int const PORT = 48654;
    QSqlDatabase database;
    QList <User *> users;
    QTextStream log;

public:
    explicit Server();
    ~Server();

public:

    /* Запрос баланса пользователя */
    void getBalance(User *user);

public slots:
    /* Создание нового юзера */
    void newUser();
    /* Запрос авторизации пользователя*/
    void authorization(User *user, QString *password);
    /*Удаление юзера из списка при рассоединении*/
    void remove(User *user);
    /* Проведение новой операции */
    void newOperation(User *user, QString to, int sum);

};


#endif // SERVER_H
