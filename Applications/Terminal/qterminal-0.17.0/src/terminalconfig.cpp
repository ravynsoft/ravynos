

#include <QHash>
#include <QString>

#include "qterminalapp.h"
#include "terminalconfig.h"
#include "properties.h"
#include "termwidget.h"

TerminalConfig::TerminalConfig(const QString & wdir, const QString & shell)
{
    m_workingDirectory = wdir;
    m_shell = shell;
}

TerminalConfig::TerminalConfig()
{
}

TerminalConfig::TerminalConfig(const TerminalConfig &cfg)
    : m_currentDirectory(cfg.m_currentDirectory),
      m_workingDirectory(cfg.m_workingDirectory),
      m_shell(cfg.m_shell) {}

QString TerminalConfig::getWorkingDirectory()
{
    if (!m_workingDirectory.isEmpty())
        return m_workingDirectory;
    if (Properties::Instance()->useCWD && !m_currentDirectory.isEmpty())
        return m_currentDirectory;
    return QTerminalApp::Instance()->getWorkingDirectory();
}

QString TerminalConfig::getShell()
{
    if (!m_shell.isEmpty())
        return m_shell;
    if (!Properties::Instance()->shell.isEmpty())
        return Properties::Instance()->shell;
    QByteArray envShell = qgetenv("SHELL");
    if (envShell.constData() != nullptr)
    {
        QString shellString = QString::fromLocal8Bit(envShell);
        if (!shellString.isEmpty())
            return shellString;
    }
    return QString();
}

void TerminalConfig::setWorkingDirectory(const QString &val)
{
    m_workingDirectory = val;
}

void TerminalConfig::setShell(const QString &val)
{
    m_shell = val;
}

void TerminalConfig::provideCurrentDirectory(const QString &val)
{
    m_currentDirectory = val;
}



#if HAVE_QDBUS

#define DBUS_ARG_WORKDIR "workingDirectory"
#define DBUS_ARG_SHELL "shell"

TerminalConfig TerminalConfig::fromDbus(const QHash<QString,QVariant> &termArgsConst, TermWidget *toSplit)
{
    QHash<QString,QVariant> termArgs(termArgsConst);
    if (toSplit != nullptr && !termArgs.contains(QLatin1String(DBUS_ARG_WORKDIR)))
    {
        termArgs[QLatin1String(DBUS_ARG_WORKDIR)] = QVariant(toSplit->impl()->workingDirectory());
    }
    return TerminalConfig::fromDbus(termArgs);
}

static QString variantToString(const QVariant& variant, QString &defaultVal)
{
    if (variant.type() == QVariant::String)
        return qvariant_cast<QString>(variant);
    return defaultVal;
}

TerminalConfig TerminalConfig::fromDbus(const QHash<QString,QVariant> &termArgs)
{
    QString wdir = QString();
    QString shell(Properties::Instance()->shell);
    if (termArgs.contains(QLatin1String(DBUS_ARG_WORKDIR)))
    {
        wdir = variantToString(termArgs[QLatin1String(DBUS_ARG_WORKDIR)], wdir);
    }
    if (termArgs.contains(QLatin1String(DBUS_ARG_SHELL))) {
        shell = variantToString(termArgs[QLatin1String(DBUS_ARG_SHELL)], shell);
    }
    return TerminalConfig(wdir, shell);
}

#endif

