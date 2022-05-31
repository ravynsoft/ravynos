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


#include "sysmenu.h"
#include "version.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QUrl>
#include <KRecentDocument>
#include <KDesktopFile>
#import <CoreFoundation/CoreFoundation.h>
#import <LaunchServices/LaunchServices.h>

AXMessageBox::AXMessageBox()
    : QMessageBox()
{
}

void AXMessageBox::closeEvent(QCloseEvent *event)
{
    this->done(0);
}

ravynOSMenu::ravynOSMenu(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , m_dbus(QDBusConnection::sessionBus())
    , m_refreshedRecent(false)
{
    m_about = NULL;
    graphicsAdaptors();
    productName();

    QAction *a;
    a = m_menu.addAction("About This Computer");
    connect(a, &QAction::triggered, this, &ravynOSMenu::aboutThisComputer);
    m_menu.addSeparator();
    a = m_menu.addAction("System Preferences...");
    connect(a, &QAction::triggered, this, &ravynOSMenu::systemPreferences);
    a = m_menu.addAction("Software Store...");
    a->setEnabled(false);
    m_menu.addSeparator();
    m_recentItemsAction = m_menu.addAction("Recent Items");
    m_recentItemsAction->setMenu(&m_recentItems);
    connect(m_recentItemsAction, &QAction::hovered, this, &ravynOSMenu::refreshRecentItems);
    m_menu.addSeparator();
    a = m_menu.addAction("Force Quit...");
    a->setEnabled(false);
    m_menu.addSeparator();
    a = m_menu.addAction("Sleep");
    connect(a, &QAction::triggered, this, &ravynOSMenu::suspend);
    a = m_menu.addAction("Restart...");
    connect(a, &QAction::triggered, this, &ravynOSMenu::requestRestart);
    a = m_menu.addAction("Shut Down...");
    connect(a, &QAction::triggered, this, &ravynOSMenu::requestShutDown);
    m_menu.addSeparator();
    a = m_menu.addAction("Lock Screen");
    connect(a, &QAction::triggered, this, &ravynOSMenu::requestLockScreen);
    a = m_menu.addAction("Log Out"); // FIXME: add full name here
    connect(a, &QAction::triggered, this, &ravynOSMenu::requestLogout);

    connect(&m_menu, &QMenu::hovered, this, &ravynOSMenu::menuHovered);
    connect(&m_menu, &QMenu::triggered, this, &ravynOSMenu::menuTriggered);

#ifdef __MACH__
    /* Kickstart the Mach subsystem to trigger launchd. Save the ports
     * of our core services in case we want them later.
     */
    kern_return_t kr = bootstrap_look_up(bootstrap_port, "com.ravynos.Dock",
	&m_bportDock);
    if(kr != KERN_SUCCESS)
    	m_bportDock = MACH_PORT_NULL;
    kr = bootstrap_look_up(bootstrap_port, "com.ravynos.Filer", &m_bportFiler);
    if(kr != KERN_SUCCESS)
    	m_bportFiler = MACH_PORT_NULL;
#endif
}

ravynOSMenu::~ravynOSMenu()
{
}

void ravynOSMenu::openMenu(int x, int y)
{
    m_menu.popup(QPoint(x,y));
}

void ravynOSMenu::menuHovered(QAction *action)
{
    if(action != m_recentItemsAction)
        m_refreshedRecent = false;
}

void ravynOSMenu::menuTriggered(QAction *action)
{
    if(action != m_recentItemsAction)
        m_refreshedRecent = false;
}

unsigned int ravynOSMenu::numCPUs()
{
    int mib[2];
    unsigned int cpus;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(cpus);
    sysctl((const int *)mib, 2, &cpus, &len, NULL,	0);
    return cpus;
}

unsigned long ravynOSMenu::realMemory()
{
    int mib[2];
    unsigned long mem;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_REALMEM;
    len = sizeof(mem);
    sysctl((const int *)mib, 2, &mem, &len, NULL,	0);
    return mem;
}

QString ravynOSMenu::formatAsGB(unsigned long bytes)
{
    double gb = (double)bytes;
    gb /=  (1024.0 * 1024.0 * 1024.0);
    return QString::asprintf("%.1f GB", gb);
}

QString ravynOSMenu::CPUModel()
{
    int mib[2];
    char model[128];
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    len = sizeof(model)*sizeof(char);
    sysctl((const int *)mib, 2, model, &len, NULL,	0);
    return QString(model);
}

QStringList ravynOSMenu::graphicsAdaptors()
{
    if(!m_adaptorsFound.isEmpty())
        return m_adaptorsFound;

    QStringList cards;
    for(int x = 0; x < 2; ++x)
        cards << QString::asprintf("vgapci%d", x);
    
    for(QStringList::iterator iter = cards.begin(); iter != cards.end(); iter++) {
        QProcess *pciconf = new QProcess(this);
        connect(pciconf, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
        connect(pciconf, SIGNAL(readyReadStandardOutput()), this, SLOT(pciconfOutputReady()));

        pciconf->start("/usr/sbin/pciconf", QStringList() << "-lv" << *iter);
    }

    return m_adaptorsFound;
}

void ravynOSMenu::pciconfOutputReady()
{
    QProcess *p = (QProcess *)sender();
    QList<QByteArray> lines = p->readAllStandardOutput().split('\n');
    QByteArray vendor, device, subclass;

    for(int n = 0; n < lines.count(); ++n) {
        QByteArray line = lines[n];

        if(line.contains("vendor")) {
            vendor = line.mid(line.indexOf('\'')+1, -1);
            vendor.chop(1);
        } else if(line.contains("device")) {
            device = line.mid(line.indexOf('\'')+1, -1);
            device.chop(1);
        } else if(line.contains("subclass")) {
            subclass = line.mid(line.indexOf('=')+1, -1);
        }
    }

    if(device.count() && vendor.count())
        m_adaptorsFound << QString::asprintf("%s %s", vendor.constData(), device.constData());
    else
        m_adaptorsFound << QString::asprintf("%s", subclass.constData());
}

void ravynOSMenu::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *p = (QProcess *)sender();
    delete p;
}

QString ravynOSMenu::hostUUID()
{
    int mib[2];
    char uuid[64];
    size_t len;

    mib[0] = CTL_KERN;
    mib[1] = KERN_HOSTUUID;
    len = sizeof(uuid)*sizeof(char);
    sysctl((const int *)mib, 2, uuid, &len, NULL,	0);
    return QString(uuid);
}

void ravynOSMenu::productName()
{
    QProcess *dmihelper = new QProcess(this);
    connect(dmihelper, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(dmihelper, SIGNAL(readyReadStandardOutput()), this, SLOT(dmiOutputReady()));

    dmihelper->start("/System/Library/CoreServices/DMIHelper", QStringList());
}

void ravynOSMenu::dmiOutputReady()
{
    QProcess *p = (QProcess *)sender();
    QByteArray output = p->readAllStandardOutput();

    QList<QByteArray> lines = output.split('\n');
    m_productName = QString::asprintf("%s %s", lines[0].constData(), lines[1].constData());
}

void ravynOSMenu::aboutThisComputer()
{
    if(m_about) {
        m_about->open();
        m_about->raise();
        m_about->activateWindow();
        return;
    }

    m_about = new AXMessageBox();
    m_about->setWindowTitle("About This Computer");
    m_about->setStandardButtons(0);
    m_about->setText("<table style=\"table-layout: fixed; borders: 0;\"><tr><td width=\"100%\" align=\"center\" valign=\"middle\">"
                   "<img width=\"140\" height=\"140\" src=\"/usr/share/plasma/plasmoids/com.ravynos.plasma.ravynOSMenu/contents/images/MarmosetLogo.tiff\">"
                   "</td><td>&nbsp;&nbsp;</td><td style=\"word-wrap: break-word; width: 100%;\">"
                   "<font face=\"Nimbus Sans\"><font size=\"+7\"><b>ravynOS</b> " RAVYNOS_CODENAME "</font><br>"
                   "Version " RAVYNOS_VERSION "<br>"
                   "</font size=\"-2\"><p><b>" + m_productName + "</b></p>"
                   "<p><b>Processor</b>&nbsp;&nbsp; "+ QString::asprintf("%d-core ", numCPUs()) + CPUModel() +"</p>"
                   "<p><b>Memory</b>&nbsp;&nbsp; "+ formatAsGB(realMemory()) +"</p>"
                   "<p><b>Graphics</b>&nbsp;&nbsp; "+ m_adaptorsFound.join("<br/>") +"</p>"
                   "</font></font></td></tr></table>");
    m_about->setWindowModality(Qt::NonModal);
    m_about->setWindowFlags(Qt::Tool); // disable minimize button
    m_about->open(this, SLOT(aboutFinished()));
}

void ravynOSMenu::aboutFinished()
{
    delete m_about;
    m_about = NULL;
}

void ravynOSMenu::requestRestart()
{
    requestKSMLogout(1,1,3); // KSMServer restart with confirmation
}

void ravynOSMenu::requestShutDown()
{
    requestKSMLogout(1,2,3); // KSMServer shut down with confirmation
}

void ravynOSMenu::requestLockScreen()
{
    QDBusMessage msgLock = QDBusMessage::createMethodCall("org.kde.ksmserver", "/ScreenSaver", "", "Lock");
    m_dbus.asyncCall(msgLock);
}

void ravynOSMenu::requestLogout()
{
    requestKSMLogout(1,0,3); // KSMServer logout with confirmation
}

void ravynOSMenu::requestKSMLogout(int a, int b, int c)
{
    QDBusMessage msgLogout = QDBusMessage::createMethodCall("org.kde.ksmserver", "/KSMServer", "", "logout");
    QVariantList args;
    args.append(a);
    args.append(b);
    args.append(c);
    msgLogout.setArguments(args);
    m_dbus.asyncCall(msgLogout);
}

void ravynOSMenu::systemPreferences()
{
    LSLaunchURLSpec spec;
    spec.appURL = CFURLCreateWithFileSystemPath(NULL, CFSTR("/usr/bin/systemsettings5"), kCFURLPOSIXPathStyle, false);
    spec.asyncRefCon = 0;
    spec.itemURLs = NULL;
    spec.launchFlags = kLSLaunchDefaults;
    LSOpenFromURLSpec(&spec, NULL);
    CFRelease(spec.appURL);
}

void ravynOSMenu::suspend()
{
    QDBusMessage msgSuspend = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement/Actions/SuspendSession",
        "org.kde.Solid.PowerManagement.Actions.SuspendSession", "suspendHybrid");
    m_dbus.asyncCall(msgSuspend);
}

void ravynOSMenu::refreshRecentItems()
{
    if(m_refreshedRecent)
        return; // I told em we already got one!

    QList<QAction *> actionList = m_recentItems.actions();
    m_recentItems.clear();

    for(QString item : KRecentDocument::recentDocuments()) {
        KDesktopFile df(item);
        QAction *a = new QAction(df.readName());
        a->setData(df.readUrl());
        connect(a, &QAction::triggered, this, &ravynOSMenu::openRecentItemsEntry);
        m_recentItems.addAction(a);
    }
    m_refreshedRecent = true;

    // delete the old action pointers
    for(QAction *a : actionList) {
        delete a;
    }
}

void ravynOSMenu::openRecentItemsEntry()
{
    QString path(((QAction *)QObject::sender())->data().toString());
    QUrl url(path);

    CFMutableArrayRef CFLSFiles = CFArrayCreateMutable(NULL, 2, NULL);
	CFStringRef item = CFStringCreateWithCString(NULL, url.path().toUtf8(), kCFStringEncodingUTF8);
	CFURLRef itemURL = CFURLCreateWithFileSystemPath(NULL, item, kCFURLPOSIXPathStyle, false);
	CFArrayAppendValue(CFLSFiles, itemURL);
	CFRelease(item);

    LSLaunchURLSpec spec;
    spec.appURL = NULL;
    spec.itemURLs = (CFArrayRef)CFLSFiles;
    LSOpenFromURLSpec(&spec, NULL);
    CFRelease(CFLSFiles);
}

K_PLUGIN_CLASS_WITH_JSON(ravynOSMenu, "metadata.json")

#include "sysmenu.moc"
