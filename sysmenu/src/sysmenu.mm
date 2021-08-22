/*
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
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
#include <QDebug>
#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform.h>

const char *getCpuInfo()
{
    NSString *result = [NSString stringWithFormat:@"%d cores (NOT IMPLEMENTED)", 0];
    return [[result retain] UTF8String];
}

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
                   "<img width=\"128\" height=\"128\" src=\"/usr/share/plasma/plasmoids/org.airyx.plasma.AiryxMenu/contents/images/tanuki_logo.png\">"
                   "</td><td>&nbsp;&nbsp;</td><td width=\"100%\">"
                   "<font face=\"Nimbus Sans\"><font size=\"+8\"><b>airyxOS</b> " AIRYX_CODENAME "</font><br>"
                   "Version " AIRYX_VERSION "<br>&nbsp;<br>&nbsp;<br>"
                   "<b>Model from BIOS</b><br><br>"
                   "<b>Processor</b> "+ QString(getCpuInfo()) +"<br><br>"
                   "<b>Memory</b> xxxx xxxx xx<br><br>"
                   "<b>Graphics</b> xxxxx xxxx xxx<br><br>"
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
