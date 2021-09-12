#include "remoteterm.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(a.arguments().size() != 3){
        qDebug() << "Example(client-side) for remote terminal of QTermWidget.";
        qDebug() << QString("Usage: %1 ipaddr port").arg(a.arguments()[0]);
        return 1;
    }
    QString ipaddr = a.arguments().at(1);
    quint16 port = a.arguments().at(2).toUShort();
    RemoteTerm w(ipaddr,port);
    w.show();

    return a.exec();
}
