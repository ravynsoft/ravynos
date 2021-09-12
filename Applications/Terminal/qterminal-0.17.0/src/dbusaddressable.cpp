

#include "dbusaddressable.h"

#ifdef HAVE_QDBUS
Q_DECLARE_METATYPE(QList<QDBusObjectPath>)

QString DBusAddressable::getDbusPathString()
{
    return m_path;
}

QDBusObjectPath DBusAddressable::getDbusPath()
{
    return QDBusObjectPath(m_path);
}
#endif

DBusAddressable::DBusAddressable(const QString& prefix)
{
    #ifdef HAVE_QDBUS
    QString uuidString = QUuid::createUuid().toString();
    m_path = prefix + QLatin1Char('/') + uuidString.replace(QRegExp(QStringLiteral("[\\{\\}\\-]")), QString());
    #endif
}
