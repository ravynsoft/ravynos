#ifndef DBUSADDRESSABLE_H
#define DBUSADDRESSABLE_H

#include <QString>
#ifdef HAVE_QDBUS
#include <QtDBus/QtDBus>
#include <QUuid>
#endif

class DBusAddressable
{
    #ifdef HAVE_QDBUS
    private:
        QString m_path;
    #endif
    public:
    #ifdef HAVE_QDBUS
        QDBusObjectPath getDbusPath();
        QString getDbusPathString();
    #endif
        DBusAddressable(const QString& prefix);
};

#ifdef HAVE_QDBUS
template <class AClass, class WClass> void registerAdapter(WClass *obj)
{
    new AClass(obj);
    QString path = dynamic_cast<DBusAddressable*>(obj)->getDbusPathString();
    QDBusConnection::sessionBus().registerObject(path, obj);
}
#endif


#endif