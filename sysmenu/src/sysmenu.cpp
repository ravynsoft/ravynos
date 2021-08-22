/*
*/

#include "sysmenu.h"
#include "version.h"
#include <QDebug>

AXMessageBox::AXMessageBox()
    : QMessageBox()
{
}

void AXMessageBox::closeEvent(QCloseEvent *event)
{
    this->done(0);
}

AiryxMenu::AiryxMenu(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    m_about = NULL;
}

AiryxMenu::~AiryxMenu()
{
}

void AiryxMenu::aboutThisComputer()
{
    if(m_about) {
        m_about->open();
        m_about->raise();
        m_about->activateWindow();
        return;
    }

    m_about = new AXMessageBox();
    m_about->setWindowTitle("About this Computer");
    m_about->setStandardButtons(0);
    m_about->setText("<table borders=\"0\"><tr><td align=\"center\" valign=\"middle\">"
                   "<img src=\"/usr/share/plasma/plasmoids/org.airyx.plasma.AiryxMenu/contents/images/tanuki_logo.png\">"
                   "</td><td>&nbsp;&nbsp;</td><td>"
                   "<font face=\"Nimbus Sans\"><font size=\"+8\"><b>airyxOS</b> " AIRYX_CODENAME "</font><br>"
                   "Version " AIRYX_VERSION "<br>&nbsp;<br>&nbsp;<br>"
                   "<b>Model from BIOS</b><br>"
                   "<b>Processor</b> xxx xxx xxx<br>"
                   "<b>Memory</b> xxxx xxxx xx<br>"
                   "<b>Graphics</b> xxxxx xxxx xxx<br>"
                   "<br>"
                   "</font></td></tr></table>");
    m_about->setWindowModality(Qt::NonModal);
    m_about->open(this, SLOT(aboutFinished()));
}

void AiryxMenu::aboutFinished()
{
    delete m_about;
    m_about = NULL;
}

K_PLUGIN_CLASS_WITH_JSON(AiryxMenu, "metadata.json")

#include "sysmenu.moc"
