#include "remoteterm.h"
#include <QTcpSocket>
#include <QDebug>
#include <unistd.h>

RemoteTerm::RemoteTerm(const QString &ipaddr, quint16 port, QWidget *parent)
    : QTermWidget(0,parent)
{
    socket = new QTcpSocket(this);
    
    // Write what we input to remote terminal via socket
    connect(this, &RemoteTerm::sendData,[this](const char *data, int size){
        this->socket->write(data, size);
    });

    // Read anything from remote terminal via socket and show it on widget.
    connect(socket,&QTcpSocket::readyRead,[this](){
        QByteArray data = socket->readAll();
        write(this->getPtySlaveFd(), data.data(), data.size());
    });
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(atError()));

    // Here we start an empty pty.
    this->startTerminalTeletype();

    socket->connectToHost(ipaddr, port);
}

void RemoteTerm::atError()
{
    qDebug() << socket->errorString();
}
