/***************************************************************************
 *   Copyright (C) 2010 by Petr Vanek                                      *
 *   petr@scribus.info                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <qtermwidget.h>
#include <cassert>

#include "properties.h"
#include "config.h"
#include "mainwindow.h"
#include "qterminalapp.h"

Properties * Properties::m_instance = nullptr;


Properties * Properties::Instance(const QString& filename)
{
    if (!m_instance)
        m_instance = new Properties(filename);
    return m_instance;
}

Properties::Properties(const QString& filename)
    : filename(filename)
{
    if (filename.isEmpty())
        m_settings = new QSettings();
    else
        m_settings = new QSettings(filename);
    //qDebug("Properties constructor called");
}

Properties::~Properties()
{
    //qDebug("Properties destructor called");
    delete m_settings;
    m_instance = nullptr;
}

QFont Properties::defaultFont()
{
    QFont default_font = QApplication::font();
    default_font.setFamily(QLatin1String(DEFAULT_FONT));
    default_font.setPointSize(12);
    default_font.setStyleHint(QFont::TypeWriter);
    return default_font;
}

void Properties::loadSettings()
{
    guiStyle = m_settings->value(QLatin1String("guiStyle"), QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);

    colorScheme = m_settings->value(QLatin1String("colorScheme"), QLatin1String("Linux")).toString();

    highlightCurrentTerminal = m_settings->value(QLatin1String("highlightCurrentTerminal"), true).toBool();
    showTerminalSizeHint = m_settings->value(QLatin1String("showTerminalSizeHint"), true).toBool();

    font = QFont(qvariant_cast<QString>(m_settings->value(QLatin1String("fontFamily"), defaultFont().family())),
                 qvariant_cast<int>(m_settings->value(QLatin1String("fontSize"), defaultFont().pointSize())));
    //Legacy font setting
    font = qvariant_cast<QFont>(m_settings->value(QLatin1String("font"), font));

    mainWindowSize = m_settings->value(QLatin1String("MainWindow/size")).toSize();
    fixedWindowSize = m_settings->value(QLatin1String("MainWindow/fixedSize"), QSize(600, 400)).toSize().expandedTo(QSize(300, 200));
    mainWindowPosition = m_settings->value(QLatin1String("MainWindow/pos")).toPoint();
    mainWindowState = m_settings->value(QLatin1String("MainWindow/state")).toByteArray();

    historyLimited = m_settings->value(QLatin1String("HistoryLimited"), true).toBool();
    historyLimitedTo = m_settings->value(QLatin1String("HistoryLimitedTo"), 1000).toUInt();

    emulation = m_settings->value(QLatin1String("emulation"), QLatin1String("default")).toString();

    // sessions
    int size = m_settings->beginReadArray(QLatin1String("Sessions"));
    for (int i = 0; i < size; ++i)
    {
        m_settings->setArrayIndex(i);
        QString name(m_settings->value(QLatin1String("name")).toString());
        if (name.isEmpty())
            continue;
        sessions[name] = m_settings->value(QLatin1String("state")).toString();
    }
    m_settings->endArray();

    terminalMargin = m_settings->value(QLatin1String("TerminalMargin"), 0).toInt();

    appTransparency = m_settings->value(QLatin1String("MainWindow/ApplicationTransparency"), 0).toInt();
    termTransparency = m_settings->value(QLatin1String("TerminalTransparency"), 0).toInt();
    backgroundImage = m_settings->value(QLatin1String("TerminalBackgroundImage"), QString()).toString();
    backgroundMode = qBound(0, m_settings->value(QLatin1String("TerminalBackgroundMode"), 0).toInt(), 4);

    /* default to Right. see qtermwidget.h */
    scrollBarPos = m_settings->value(QLatin1String("ScrollbarPosition"), 2).toInt();
    /* default to North. I'd prefer South but North is standard (they say) */
    tabsPos = m_settings->value(QLatin1String("TabsPosition"), 0).toInt();
    /* default to BlockCursor */
    keyboardCursorShape = m_settings->value(QLatin1String("KeyboardCursorShape"), 0).toInt();
    hideTabBarWithOneTab = m_settings->value(QLatin1String("HideTabBarWithOneTab"), false).toBool();
    // For "Motion after paste", 2 (scrolling to bottom) makes more sense
    m_motionAfterPaste = m_settings->value(QLatin1String("MotionAfterPaste"), 2).toInt();
    m_disableBracketedPasteMode = m_settings->value(QLatin1String("DisableBracketedPasteMode"), false).toBool();

    /* fixed tabs width */
    fixedTabWidth = m_settings->value(QLatin1String("FixedTabWidth"), true).toBool();
    fixedTabWidthValue = m_settings->value(QLatin1String("FixedTabWidthValue"), 500).toInt();
    /* tabs features */
    showCloseTabButton = m_settings->value(QLatin1String("ShowCloseTabButton"), true).toBool();
    closeTabOnMiddleClick = m_settings->value(QLatin1String("CloseTabOnMiddleClick"), true).toBool();

    /* toggles */
    borderless = m_settings->value(QLatin1String("Borderless"), false).toBool();
    tabBarless = m_settings->value(QLatin1String("TabBarless"), false).toBool();
    menuVisible = m_settings->value(QLatin1String("MenuVisible"), true).toBool();
    boldIntense = m_settings->value(QLatin1String("BoldIntense"), true).toBool();
    noMenubarAccel = m_settings->value(QLatin1String("NoMenubarAccel"), true).toBool();
    askOnExit = m_settings->value(QLatin1String("AskOnExit"), true).toBool();
    saveSizeOnExit = m_settings->value(QLatin1String("SaveSizeOnExit"), true).toBool();
    savePosOnExit = m_settings->value(QLatin1String("SavePosOnExit"), true).toBool();
    useCWD = m_settings->value(QLatin1String("UseCWD"), false).toBool();
    m_openNewTabRightToActiveTab = m_settings->value(QLatin1String("OpenNewTabRightToActiveTab"), false).toBool();
    term = m_settings->value(QLatin1String("Term"), QLatin1String("xterm-256color")).toString();
    handleHistoryCommand = m_settings->value(QLatin1String("HandleHistory")).toString();

    // bookmarks
    useBookmarks = m_settings->value(QLatin1String("UseBookmarks"), false).toBool();
    bookmarksVisible = m_settings->value(QLatin1String("BookmarksVisible"), true).toBool();
    const QString s = QFileInfo(m_settings->fileName()).canonicalPath() + QString::fromLatin1("/qterminal_bookmarks.xml");
    bookmarksFile = m_settings->value(QLatin1String("BookmarksFile"), s).toString();

    terminalsPreset = m_settings->value(QLatin1String("TerminalsPreset"), 0).toInt();

    m_settings->beginGroup(QLatin1String("DropMode"));
    dropShortCut = QKeySequence(m_settings->value(QLatin1String("ShortCut"), QLatin1String("F12")).toString());
    dropKeepOpen = m_settings->value(QLatin1String("KeepOpen"), false).toBool();
    dropShowOnStart = m_settings->value(QLatin1String("ShowOnStart"), true).toBool();
    dropWidht = m_settings->value(QLatin1String("Width"), 70).toInt();
    dropHeight = m_settings->value(QLatin1String("Height"), 45).toInt();
    m_settings->endGroup();

    changeWindowTitle = m_settings->value(QLatin1String("ChangeWindowTitle"), true).toBool();
    changeWindowIcon = m_settings->value(QLatin1String("ChangeWindowIcon"), true).toBool();
    enabledBidiSupport = m_settings->value(QLatin1String("enabledBidiSupport"), true).toBool();
    useFontBoxDrawingChars = m_settings->value(QLatin1String("UseFontBoxDrawingChars"), false).toBool();

    confirmMultilinePaste = m_settings->value(QLatin1String("ConfirmMultilinePaste"), false).toBool();
    trimPastedTrailingNewlines = m_settings->value(QLatin1String("TrimPastedTrailingNewlines"), false).toBool();

    windowMaximized = m_settings->value(QLatin1String("LastWindowMaximized"), false).toBool();

    prefDialogSize = m_settings->value(QLatin1String("PrefDialogSize")).toSize();
}

void Properties::saveSettings()
{
    m_settings->setValue(QLatin1String("guiStyle"), guiStyle);
    m_settings->setValue(QLatin1String("colorScheme"), colorScheme);
    m_settings->setValue(QLatin1String("highlightCurrentTerminal"), highlightCurrentTerminal);
    m_settings->setValue(QLatin1String("showTerminalSizeHint"), showTerminalSizeHint);
    m_settings->setValue(QLatin1String("fontFamily"), font.family());
    m_settings->setValue(QLatin1String("fontSize"), font.pointSize());
    //Clobber legacy setting
    m_settings->remove(QLatin1String("font"));

    m_settings->beginGroup(QLatin1String("Shortcuts"));
    MainWindow *mainWindow = QTerminalApp::Instance()->getWindowList()[0];
    assert(mainWindow != nullptr);

    QMapIterator< QString, QAction * > it(mainWindow->leaseActions());
    while( it.hasNext() )
    {
        it.next();
        QStringList sequenceStrings;
        const auto shortcuts = it.value()->shortcuts();
        for (const QKeySequence &shortcut : shortcuts)
            sequenceStrings.append(shortcut.toString());
        m_settings->setValue(it.key(), sequenceStrings.join(QLatin1Char('|')));
    }
    m_settings->endGroup();

    m_settings->setValue(QLatin1String("MainWindow/size"), mainWindowSize);
    m_settings->setValue(QLatin1String("MainWindow/fixedSize"), fixedWindowSize);
    m_settings->setValue(QLatin1String("MainWindow/pos"), mainWindowPosition);
    m_settings->setValue(QLatin1String("MainWindow/state"), mainWindowState);

    m_settings->setValue(QLatin1String("HistoryLimited"), historyLimited);
    m_settings->setValue(QLatin1String("HistoryLimitedTo"), historyLimitedTo);

    m_settings->setValue(QLatin1String("emulation"), emulation);

    // sessions
    m_settings->beginWriteArray(QLatin1String("Sessions"));
    int i = 0;
    Sessions::iterator sit = sessions.begin();
    while (sit != sessions.end())
    {
        m_settings->setArrayIndex(i);
        m_settings->setValue(QLatin1String("name"), sit.key());
        m_settings->setValue(QLatin1String("state"), sit.value());
        ++sit;
        ++i;
    }
    m_settings->endArray();

    m_settings->setValue(QLatin1String("MainWindow/ApplicationTransparency"), appTransparency);
    m_settings->setValue(QLatin1String("TerminalMargin"), terminalMargin);
    m_settings->setValue(QLatin1String("TerminalTransparency"), termTransparency);
    m_settings->setValue(QLatin1String("TerminalBackgroundImage"), backgroundImage);
    m_settings->setValue(QLatin1String("TerminalBackgroundMode"), backgroundMode);
    m_settings->setValue(QLatin1String("ScrollbarPosition"), scrollBarPos);
    m_settings->setValue(QLatin1String("TabsPosition"), tabsPos);
    m_settings->setValue(QLatin1String("KeyboardCursorShape"), keyboardCursorShape);
    m_settings->setValue(QLatin1String("HideTabBarWithOneTab"), hideTabBarWithOneTab);
    m_settings->setValue(QLatin1String("MotionAfterPaste"), m_motionAfterPaste);
    m_settings->setValue(QLatin1String("DisableBracketedPasteMode"), m_disableBracketedPasteMode);

    m_settings->setValue(QLatin1String("FixedTabWidth"), fixedTabWidth);
    m_settings->setValue(QLatin1String("FixedTabWidthValue"), fixedTabWidthValue);
    m_settings->setValue(QLatin1String("ShowCloseTabButton"), showCloseTabButton);
    m_settings->setValue(QLatin1String("CloseTabOnMiddleClick"), closeTabOnMiddleClick);

    m_settings->setValue(QLatin1String("Borderless"), borderless);
    m_settings->setValue(QLatin1String("TabBarless"), tabBarless);
    m_settings->setValue(QLatin1String("BoldIntense"), boldIntense);
    m_settings->setValue(QLatin1String("NoMenubarAccel"), noMenubarAccel);
    m_settings->setValue(QLatin1String("MenuVisible"), menuVisible);
    m_settings->setValue(QLatin1String("AskOnExit"), askOnExit);
    m_settings->setValue(QLatin1String("SavePosOnExit"), savePosOnExit);
    m_settings->setValue(QLatin1String("SaveSizeOnExit"), saveSizeOnExit);
    m_settings->setValue(QLatin1String("UseCWD"), useCWD);
    m_settings->setValue(QLatin1String("OpenNewTabRightToActiveTab"), m_openNewTabRightToActiveTab);
    m_settings->setValue(QLatin1String("Term"), term);
    m_settings->setValue(QLatin1String("HandleHistory"), handleHistoryCommand);

    // bookmarks
    m_settings->setValue(QLatin1String("UseBookmarks"), useBookmarks);
    m_settings->setValue(QLatin1String("BookmarksVisible"), bookmarksVisible);
    m_settings->setValue(QLatin1String("BookmarksFile"), bookmarksFile);

    m_settings->setValue(QLatin1String("TerminalsPreset"), terminalsPreset);

    m_settings->beginGroup(QLatin1String("DropMode"));
    m_settings->setValue(QLatin1String("ShortCut"), dropShortCut.toString());
    m_settings->setValue(QLatin1String("KeepOpen"), dropKeepOpen);
    m_settings->setValue(QLatin1String("ShowOnStart"), dropShowOnStart);
    m_settings->setValue(QLatin1String("Width"), dropWidht);
    m_settings->setValue(QLatin1String("Height"), dropHeight);
    m_settings->endGroup();

    m_settings->setValue(QLatin1String("ChangeWindowTitle"), changeWindowTitle);
    m_settings->setValue(QLatin1String("ChangeWindowIcon"), changeWindowIcon);
    m_settings->setValue(QLatin1String("enabledBidiSupport"), enabledBidiSupport);
    m_settings->setValue(QLatin1String("UseFontBoxDrawingChars"), useFontBoxDrawingChars);

    m_settings->setValue(QLatin1String("ConfirmMultilinePaste"), confirmMultilinePaste);
    m_settings->setValue(QLatin1String("TrimPastedTrailingNewlines"), trimPastedTrailingNewlines);

    m_settings->setValue(QLatin1String("LastWindowMaximized"), windowMaximized);

    m_settings->setValue(QLatin1String("PrefDialogSize"), prefDialogSize);
}

int Properties::versionComparison(const QString &v1, const QString &v2)
{
    int res = 0;
    QStringList list1 = v1.split(QLatin1String("."));
    QStringList list2 = v2.split(QLatin1String("."));
    int N = qMin(list1.size(), list2.size());
    for (int i = 0; i < N; ++i)
    {
        int n1 = list1.at(i).toInt();
        int n2 = list2.at(i).toInt();
        if (n1 != n2)
        {
            if (n1 < n2)
                res = -1;
            else
                res = 1;
            break;
        }
    }
    if (res == 0 && list1.size() > list2.size())
        res = -1;
    return res;
}

void Properties::migrate_settings()
{
    // Deal with rearrangements of settings.
    // If this method becomes unbearably huge we should look at the config-update
    // system used by kde and razor.
    QSettings settings;
    QString lastVersion = settings.value(QLatin1String("version"), QLatin1String("0.0.0")).toString();
    QString currentVersion(QLatin1String(QTERMINAL_VERSION));
    if (versionComparison(currentVersion, lastVersion) < 0)
    {
        qDebug() << "Warning: Configuration file was written by a newer version "
                 << "of QTerminal. Some settings might be incompatible.";
    }

    if (versionComparison(lastVersion, QLatin1String("0.4.0")) < 0)
    {
        // ===== Paste Selection -> Paste Clipboard =====
        settings.beginGroup(QLatin1String("Shortcuts"));
        if(!settings.contains(QLatin1String(PASTE_CLIPBOARD)))
        {
            QString value = settings.value(QLatin1String("Paste Selection"), QLatin1String(PASTE_CLIPBOARD_SHORTCUT)).toString();
            settings.setValue(QLatin1String(PASTE_CLIPBOARD), value);
        }
        settings.remove(QLatin1String("Paste Selection"));
        settings.endGroup();
    }

    if (versionComparison(lastVersion, QLatin1String("0.6.0")) <= 0)
    {
        // ===== AlwaysShowTabs -> HideTabBarWithOneTab =====
        if(!settings.contains(QLatin1String("HideTabBarWithOneTab")))
        {
            QString hideValue = settings.value(QLatin1String("AlwaysShowTabs"), false).toString();
            settings.setValue(QLatin1String("HideTabBarWithOneTab"), hideValue);
        }
        settings.remove(QLatin1String("AlwaysShowTabs"));

        // ===== appOpacity -> ApplicationTransparency =====
        //
        // Note: In 0.6.0 the opacity values had been erroneously
        // restricted to [0,99] instead of [1,100]. We fix this here by
        // setting the opacity to 100 if it was 99 and to 1 if it was 0.
        //
        if(!settings.contains(QLatin1String("MainWindow/ApplicationTransparency")))
        {
            int appOpacityValue = settings.value(QLatin1String("MainWindow/appOpacity"), 100).toInt();
            appOpacityValue = appOpacityValue == 99 ? 100 : appOpacityValue;
            appOpacityValue = appOpacityValue == 0 ? 1 : appOpacityValue;
            settings.setValue(QLatin1String("MainWindow/ApplicationTransparency"), 100 - appOpacityValue);
        }
        settings.remove(QLatin1String("MainWindow/appOpacity"));

        // ===== termOpacity -> TerminalTransparency =====
        if(!settings.contains(QLatin1String("TerminalTransparency")))
        {
            int termOpacityValue = settings.value(QLatin1String("termOpacity"), 100).toInt();
            termOpacityValue = termOpacityValue == 99  ? 100 : termOpacityValue;
            settings.setValue(QLatin1String("TerminalTransparency"), 100 - termOpacityValue);
        }
        settings.remove(QLatin1String("termOpacity"));
        // geometry -> size, pos
        if (!settings.contains(QLatin1String("MainWindow/size")) && settings.contains(QLatin1String("MainWindow/geometry")))
        {
            QWidget geom;
            geom.restoreGeometry(settings.value(QLatin1String("MainWindow/geometry")).toByteArray());
            settings.setValue(QLatin1String("MainWindow/size"), geom.size());
            settings.setValue(QLatin1String("MainWindow/pos"), geom.pos());
            settings.setValue(QLatin1String("MainWindow/isMaximized"), geom.isMaximized());
            settings.remove(QLatin1String("MainWindow/geometry"));
        }
    }

    if (currentVersion > lastVersion)
        settings.setValue(QLatin1String("version"), currentVersion);
}

void Properties::removeAccelerator(QString& str)
{
    // Chinese, Japanese,...
    str.remove(QRegularExpression(QStringLiteral("\\s*\\(&[a-zA-Z0-9]\\)\\s*")));
    // other languages
    str.remove(QLatin1Char('&'));
}

