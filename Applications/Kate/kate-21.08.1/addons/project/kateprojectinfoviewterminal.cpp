/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojectinfoviewterminal.h"
#include "kateprojectpluginview.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KSharedConfig>
#include <kde_terminal_interface.h>

KPluginFactory *KateProjectInfoViewTerminal::s_pluginFactory = nullptr;

KateProjectInfoViewTerminal::KateProjectInfoViewTerminal(KateProjectPluginView *pluginView, const QString &directory)
    : m_pluginView(pluginView)
    , m_directory(directory)
    , m_konsolePart(nullptr)
{
    /**
     * layout widget
     */
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

KateProjectInfoViewTerminal::~KateProjectInfoViewTerminal()
{
    /**
     * avoid endless loop
     */
    if (m_konsolePart) {
        disconnect(m_konsolePart, &KParts::ReadOnlyPart::destroyed, this, &KateProjectInfoViewTerminal::loadTerminal);
    }
}

KPluginFactory *KateProjectInfoViewTerminal::pluginFactory()
{
    if (s_pluginFactory) {
        return s_pluginFactory;
    }
    return s_pluginFactory = KPluginLoader(QStringLiteral("konsolepart")).factory();
}

void KateProjectInfoViewTerminal::showEvent(QShowEvent *)
{
    /**
     * we delay the terminal construction until we have some part to have a usable WINDOWID, see bug 411965
     */
    if (!m_konsolePart) {
        loadTerminal();
    }
}

void KateProjectInfoViewTerminal::loadTerminal()
{
    /**
     * null in any case, if loadTerminal fails below and we are in the destroyed event
     */
    m_konsolePart = nullptr;
    setFocusProxy(nullptr);

    /**
     * we shall not arrive here without a factory, if it is not there, no terminal toolview shall be created
     */
    Q_ASSERT(pluginFactory());

    /**
     * create part
     */
    m_konsolePart = pluginFactory()->create<KParts::ReadOnlyPart>(this, this);
    if (!m_konsolePart) {
        return;
    }

    /**
     * init locale translation stuff
     */
    // FIXME KF5 KGlobal::locale()->insertCatalog("konsole");

    /**
     * switch to right directory
     */
    qobject_cast<TerminalInterface *>(m_konsolePart)->showShellInDir(m_directory);

    /**
     * add to widget
     */
    m_layout->addWidget(m_konsolePart->widget());
    setFocusProxy(m_konsolePart->widget());

    /**
     * guard destruction, create new terminal!
     */
    connect(m_konsolePart, &KParts::ReadOnlyPart::destroyed, this, &KateProjectInfoViewTerminal::loadTerminal);
    // clang-format off
    connect(m_konsolePart, SIGNAL(overrideShortcut(QKeyEvent*,bool&)), this, SLOT(overrideShortcut(QKeyEvent*,bool&)));
    // clang-format on
}

void KateProjectInfoViewTerminal::overrideShortcut(QKeyEvent *, bool &override)
{
    /**
     * let konsole handle all shortcuts
     */
    override = true;
}

// share with konsole plugin
static const QStringList s_escapeExceptions{QStringLiteral("vi"), QStringLiteral("vim"), QStringLiteral("nvim")};

bool KateProjectInfoViewTerminal::ignoreEsc() const
{
    if (!m_konsolePart || !KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("KonsoleEscKeyBehaviour", true)) {
        return false;
    }

    const QStringList exceptList = KConfigGroup(KSharedConfig::openConfig(), "Konsole").readEntry("KonsoleEscKeyExceptions", s_escapeExceptions);
    const auto app = qobject_cast<TerminalInterface *>(m_konsolePart)->foregroundProcessName();
    return exceptList.contains(app);
}

bool KateProjectInfoViewTerminal::isLoadable()
{
    return (pluginFactory() != nullptr);
}

void KateProjectInfoViewTerminal::respawn(const QString &directory)
{
    if (!isLoadable()) {
        return;
    }

    m_directory = directory;
    disconnect(m_konsolePart, &KParts::ReadOnlyPart::destroyed, this, &KateProjectInfoViewTerminal::loadTerminal);

    if (m_konsolePart != nullptr) {
        delete m_konsolePart;
    }

    loadTerminal();
}

