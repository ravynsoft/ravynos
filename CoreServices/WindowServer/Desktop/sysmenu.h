/*
 * Copyright (C) 2021-2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <Plasma/Applet>
#include <QMessageBox>
#include <QDBusConnection>
#include <QProcess>
#include <QMenu>
#include <QAction>
#ifdef __MACH__
#include <mach/mach.h>
#include <servers/bootstrap.h>
#endif

/* A simple wrapper around QMessageBox that allows closing it
 * with the titlebar button */
class AXMessageBox: public QMessageBox
{
public:
    explicit AXMessageBox();
protected:
    void closeEvent(QCloseEvent *event) override;
};

class ravynOSMenu: public Plasma::Applet
{
    Q_OBJECT

public:
    ravynOSMenu(QObject *parent, const QVariantList &args);
    ~ravynOSMenu();

    Q_INVOKABLE void openMenu(int x, int y);

signals:

private slots:
    void aboutFinished();
    void pciconfOutputReady();
    void dmiOutputReady();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void menuHovered(QAction *action);
    void menuTriggered(QAction *action);
    void suspend();
    void requestRestart();
    void requestShutDown();
    void requestLockScreen();
    void requestLogout();

private:
    void requestKSMLogout(int a, int b, int c);
    void aboutThisComputer();
    void systemPreferences();
    void refreshRecentItems();
    unsigned int numCPUs();
    QString CPUModel();
    unsigned long realMemory();
    QString formatAsGB(unsigned long bytes);
    QStringList graphicsAdaptors();
    QString hostUUID();
    void productName();
    void openRecentItemsEntry();

    AXMessageBox *m_about;
    QDBusConnection m_dbus;
    QStringList m_adaptorsFound;
    QString m_productName;
    QMenu m_menu;
    QMenu m_recentItems;
    QAction *m_recentItemsAction;
    bool m_refreshedRecent;
#ifdef __MACH__
    mach_port_t m_bportDock;
    mach_port_t m_bportFiler;
#endif
};
