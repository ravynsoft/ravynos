/*
*/

#include "sysmenu.h"
#include <stdlib.h>
#include <stdio.h>

AiryxMenu::AiryxMenu(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    m_menu = new QMenu("AIRYX", nullptr);
    m_menu->addSection("About");
    emit menuChanged();
}

QMenu *AiryxMenu::menu() const
{
    return m_menu;
}

void AiryxMenu::setMenu(QMenu *menu)
{
    m_menu = menu;
}

AiryxMenu::~AiryxMenu()
{
    if(m_menu)
        delete m_menu;
}

K_PLUGIN_CLASS_WITH_JSON(AiryxMenu, "metadata.json")

#include "sysmenu.moc"
