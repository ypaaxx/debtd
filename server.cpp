#include "server.h"

#include <QFile>
#include <QDebug>

Server::Server() : QTcpServer(){
    //Подключение к серверу MySQL
    database = QSqlDatabase::addDatabase("QMYSQL");
    database.setHostName("localhost");
    database.setDatabaseName("debt");
    database.setUserName("serverDebt");
    database.setPassword("132435");
    bool ok = database.open();
    qDebug()<< "connection to database" << ok;
    if(!ok) deleteLater();

    //Начинаем слушать на порту входящие подключения
    qDebug()<< "Start listen on port" << PORT;
    listen(QHostAddress::Any, PORT);
    connect(this, SIGNAL(newConnection()), this, SLOT(newUser()));
}

Server::~Server()
{
    qDebug() << "exit";

}

void Server::authorization(User* user, QString *password){
    //Если в таблице юзеров есть с нужным именем и паролём метод возвращает сокет с юзером
    QSqlQuery auth;
    auth.prepare("SELECT * FROM debt.users WHERE login=:login AND password=:password");
    auth.bindValue(":login", user->getLogin() );
    auth.bindValue(":password", *password);
    auth.exec();
    delete password;
    if (!auth.size()) {
        remove(user);
        return;
    }
    qDebug() << user->getLogin() << "connect" << (bool) auth.size();
   getBalance(user);

}

void Server::remove(User *user)
{
    //Удаляется запись в списке подключений
    users.removeOne(user);
    qDebug()<< "disconnect" << user->getLogin();

}

void Server::getBalance(User *user){
    //qDebug()<< "balanced" << user->getLogin();
    QSqlQuery balance;
    balance.prepare("SELECT name, (plus - minus) AS balance FROM "
                    "(SELECT requester AS name, sum(sum) AS minus FROM debt.operations WHERE confirmatory=:user GROUP BY name) AS t1 "
                    "JOIN "
                    "(SELECT confirmatory AS name, sum(sum) AS plus FROM debt.operations WHERE requester=:user GROUP BY name) AS t2 "
                    "USING (name)");
    balance.bindValue(":user", user->getLogin());
    balance.exec();

    //Передача балланса это 1
    qint8  command = 1;
    QDataStream *out = user->getStream();
    *out << command;
    qDebug()<< user->getLogin()<< "Комманда:" << command;
    *out << balance.size();
    //qDebug()<< balance.size();
    while (balance.next()){
        QString name = balance.value("name").toString();
        int sum = balance.value("balance").toInt();
        *out << name << sum; 
    }

}

void Server::newOperation(User *user, QString to, int sum){
    QSqlQuery operation;
    operation.prepare("INSERT INTO debt.operations(requester, confirmatory, sum) VALUE(:from, :to, :howMuch)");
    operation.bindValue(":from", user->getLogin());
    operation.bindValue(":to", to);
    operation.bindValue(":howMuch", sum);
    operation.exec();

    qDebug()<< "new operation" << user->getLogin() << ">>" << to << sum;
    getBalance(user);

}

void Server::newUser(){
    qDebug()<< "new connection";
    User *user = new User(nextPendingConnection());
    users << user;
    connect(user, SIGNAL(deleteMi(User*)), this, SLOT(remove(User*)));
    connect(user, SIGNAL(auth(User*, QString*)), this, SLOT(authorization(User*,QString*)));
    connect(user, SIGNAL(operation(User*,QString,int)), this, SLOT(newOperation(User*,QString,int)));

}
