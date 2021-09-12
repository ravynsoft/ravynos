/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <QApplication>
#include <QtDebug>
#include <QIcon>
#include <QMainWindow>
#include <QMenuBar>

#include "qtermwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QIcon::setThemeName(QStringLiteral("oxygen"));
    QMainWindow *mainWindow = new QMainWindow();

    QTermWidget *console = new QTermWidget();

    QMenuBar *menuBar = new QMenuBar(mainWindow);
    QMenu *actionsMenu = new QMenu(QStringLiteral("Actions"), menuBar);
    menuBar->addMenu(actionsMenu);
    actionsMenu->addAction(QStringLiteral("Find..."), console, &QTermWidget::toggleShowSearchBar,
                           QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
    actionsMenu->addAction(QStringLiteral("Copy"), console, &QTermWidget::copyClipboard,
                           QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
    actionsMenu->addAction(QStringLiteral("Paste"), console, &QTermWidget::pasteClipboard,
                           QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
    actionsMenu->addAction(QStringLiteral("About Qt"), &app, &QApplication::aboutQt);
    mainWindow->setMenuBar(menuBar);

    QFont font = QApplication::font();
#ifdef Q_OS_MACOS
    font.setFamily(QStringLiteral("Monaco"));
#elif defined(Q_WS_QWS)
    font.setFamily(QStringLiteral("fixed"));
#else
    font.setFamily(QStringLiteral("Monospace"));
#endif
    font.setPointSize(12);

    console->setTerminalFont(font);

   // console->setColorScheme(COLOR_SCHEME_BLACK_ON_LIGHT_YELLOW);
    console->setScrollBarPosition(QTermWidget::ScrollBarRight);

    const auto arguments = QApplication::arguments();
    for (const QString& arg : arguments)
    {
        if (console->availableColorSchemes().contains(arg))
            console->setColorScheme(arg);
        if (console->availableKeyBindings().contains(arg))
            console->setKeyBindings(arg);
    }

    mainWindow->setCentralWidget(console);
    mainWindow->resize(600, 400);

    // info output
    qDebug() << "* INFO *************************";
    qDebug() << " availableKeyBindings:" << console->availableKeyBindings();
    qDebug() << " keyBindings:" << console->keyBindings();
    qDebug() << " availableColorSchemes:" << console->availableColorSchemes();
    qDebug() << "* INFO END *********************";

    // real startup
    QObject::connect(console, &QTermWidget::finished, mainWindow, &QMainWindow::close);

    mainWindow->show();
    return app.exec();
}
