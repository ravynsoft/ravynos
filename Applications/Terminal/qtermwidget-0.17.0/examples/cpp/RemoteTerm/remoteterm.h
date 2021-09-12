#ifndef WIDGET_H
#define WIDGET_H

#include <qtermwidget5/qtermwidget.h>

class QTcpSocket;

class RemoteTerm : public QTermWidget
{
    Q_OBJECT
public:
    RemoteTerm(const QString &ipaddr, quint16 port, QWidget *parent = 0);
public slots:
     void atError();
private:
    QTcpSocket *socket;
};

#endif // WIDGET_H
