#include <QCoreApplication>
#include <server.h>

#include <QTime>

#include <sys/types.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //qDebug() << QTime::currentTime().toString("h:m:s");

    auto pid = fork();
    //auto pid = 0; //Для отладки
    if(pid > 0){
        //qDebug() << "PID дочернего процесса" << pid;
        a.exit();
    } else if(pid == 0){
        Server server;
        return a.exec();
    } else {
        qCritical() << "Что-то не так, дочерний процесс не запущен";
        a.exit(1);
    }

}
