#ifndef QTERMINALAPP_H
#define QTERMINALAPP_H

#include <QApplication>
#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
#endif


#include "mainwindow.h"


class QTerminalApp : public QApplication
{
Q_OBJECT

public:
    MainWindow *newWindow(bool dropMode, TerminalConfig &cfg);
    QList<MainWindow*> getWindowList();
    void addWindow(MainWindow *window);
    void removeWindow(MainWindow *window);
    static QTerminalApp *Instance(int &argc, char **argv);
    static QTerminalApp *Instance();
    QString &getWorkingDirectory();
    void setWorkingDirectory(const QString &wd);

    #ifdef HAVE_QDBUS
    void registerOnDbus();
    QList<QDBusObjectPath> getWindows();
    QDBusObjectPath newWindow(const QHash<QString,QVariant> &termArgs);
    QDBusObjectPath getActiveWindow();
    bool isDropMode();
    bool toggleDropdown();
    #endif

    static void cleanup();

private:
    QString m_workDir;
    QList<MainWindow *> m_windowList;
    static QTerminalApp *m_instance;
    QTerminalApp(int &argc, char **argv);
    ~QTerminalApp() override{};
};

template <class T> T* findParent(QObject *child)
{
    QObject *maybeT = child;
    while (true)
    {
        if (maybeT == nullptr)
        {
            return nullptr;
        }
        T *holder = qobject_cast<T*>(maybeT);
        if (holder)
            return holder;
        maybeT = maybeT->parent();
    }
}

#endif