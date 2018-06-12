#include "server.h"

Server::Server() : QTcpServer(){
    //log = QTextStream("log.txt", QIODevice::WriteOnly);
    log.setDevice(new QFile("log.txt"));
    //log.

    //Подключение к серверу MySQL
    database = QSqlDatabase::addDatabase("QMYSQL");
    database.setHostName("localhost");
    database.setDatabaseName("debt");
    database.setUserName("serverDebt");
    database.setPassword("132435");
    bool ok = database.open();
    log << "connection to database" << ok << endl;
    if(!ok) deleteLater();

    //Начинаем слушать на порту входящие подключения
    log << "Start listen on port" << PORT << endl;
    listen(QHostAddress::Any, PORT);
    connect(this, SIGNAL(newConnection()), this, SLOT(newUser()));
}

Server::~Server()
{
    log << "exit" << endl;

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
    log << user->getLogin() << "connect" << (bool) auth.size() << endl;
   getBalance(user);

}

void Server::remove(User *user)
{
    //Удаляется запись в списке подключений
    users.removeOne(user);
    log << "disconnect" << user->getLogin() << endl;

}

void Server::getBalance(User *user){
    //log<< "balanced" << user->getLogin();
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
    log << user->getLogin()<< "command:" << command << endl;
    *out << balance.size();
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

    log << "new operation " << user->getLogin() << ">>" << to << sum << endl;
    getBalance(user);

}

void Server::newUser(){
    log << "new connection" << endl;
    User *user = new User(nextPendingConnection());
    users << user;
    connect(user, SIGNAL(deleteMi(User*)), this, SLOT(remove(User*)));
    connect(user, SIGNAL(auth(User*, QString*)), this, SLOT(authorization(User*,QString*)));
    connect(user, SIGNAL(operation(User*,QString,int)), this, SLOT(newOperation(User*,QString,int)));
}
