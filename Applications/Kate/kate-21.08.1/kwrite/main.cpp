/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "config.h"

#include "kwrite.h"
#include "kwriteapplication.h"

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>
#include <QUrlQuery>

#include <urlinfo.h>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif
#include <iostream>

extern "C" Q_DECL_EXPORT int main(int argc, char **argv)
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_HAIKU)
    // Prohibit using sudo or kdesu (but allow using the root user directly)
    if (getuid() == 0) {
        if (!qEnvironmentVariableIsEmpty("SUDO_USER")) {
            std::cout << "Executing KWrite with sudo is not possible due to unfixable security vulnerabilities. "
                         "It is also not necessary; simply use KWrite normally, and you will be prompted for "
                         "elevated privileges when saving documents if needed."
                      << std::endl;
            return EXIT_FAILURE;
        } else if (!qEnvironmentVariableIsEmpty("KDESU_USER")) {
            std::cout << "Executing KWrite with kdesu is not possible due to unfixable security vulnerabilities. "
                         "It is also not necessary; simply use KWrite normally, and you will be prompted for "
                         "elevated privileges when saving documents if needed."
                      << std::endl;
            return EXIT_FAILURE;
        }
    }
#endif

    /**
     * enable high dpi support
     */
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    /**
     * allow fractional scaling
     * we only activate this on Windows, it seems to creates problems on unices
     * (and there the fractional scaling with the QT_... env vars as set by KScreen works)
     * see bug 416078
     *
     * we switched to Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor because of font rendering issues
     * we follow what Krita does here, see https://invent.kde.org/graphics/krita/-/blob/master/krita/main.cc
     * we raise the Qt requirement to  5.15 as it seems some patches went in after 5.14 that are needed
     * see Krita comments, too
     */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) && defined(Q_OS_WIN)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#endif

    /**
     * Create application first
     * Enforce application name even if the executable is renamed
     */
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kwrite"));

    /**
     * For Windows and macOS: use Breeze if available
     * Of all tested styles that works the best for us
     */
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif

    /**
     * Enable crash handling through KCrash.
     */
    KCrash::initialize();

    /**
     * Connect application with translation catalogs
     */
    KLocalizedString::setApplicationDomain("kwrite");

    /**
     * then use i18n and co
     */
    KAboutData aboutData(QStringLiteral("kwrite"),
                         i18n("KWrite"),
                         QStringLiteral(KWRITE_VERSION),
                         i18n("KWrite - Text Editor"),
                         KAboutLicense::LGPL_V2,
                         i18n("(c) 2000-2021 The Kate Authors"),
                         QString(),
                         QStringLiteral("https://kate-editor.org"));

    /**
     * right dbus prefix == org.kde.
     */
    aboutData.setOrganizationDomain(QByteArray("kde.org"));

    /**
     * desktop file association to make application icon work (e.g. in Wayland window decoration)
     */
    aboutData.setDesktopFileName(QStringLiteral("org.kde.kwrite"));

    /**
     * authors & co.
     */
    aboutData.addAuthor(i18n("Christoph Cullmann"), i18n("Maintainer"), QStringLiteral("cullmann@kde.org"), QStringLiteral("https://cullmann.io"));
    aboutData.addAuthor(i18n("Dominik Haumann"), i18n("Core Developer"), QStringLiteral("dhaumann@kde.org"));
    aboutData.addAuthor(i18n("Anders Lund"), i18n("Core Developer"), QStringLiteral("anders@alweb.dk"), QStringLiteral("https://www.alweb.dk"));
    aboutData.addAuthor(i18n("Joseph Wenninger"),
                        i18n("Core Developer"),
                        QStringLiteral("jowenn@kde.org"),
                        QStringLiteral("http://stud3.tuwien.ac.at/~e9925371"));
    aboutData.addAuthor(i18n("Hamish Rodda"), i18n("Core Developer"), QStringLiteral("rodda@kde.org"));
    aboutData.addAuthor(i18n("Waldo Bastian"), i18n("The cool buffersystem"), QStringLiteral("bastian@kde.org"));
    aboutData.addAuthor(i18n("Charles Samuels"), i18n("The Editing Commands"), QStringLiteral("charles@kde.org"));
    aboutData.addAuthor(i18n("Matt Newell"),
                        i18nc("Credit text for someone that did testing and some other similar things", "Testing, ..."),
                        QStringLiteral("newellm@proaxis.com"));
    aboutData.addAuthor(i18n("Michael Bartl"), i18n("Former Core Developer"), QStringLiteral("michael.bartl1@chello.at"));
    aboutData.addAuthor(i18n("Michael McCallum"), i18n("Core Developer"), QStringLiteral("gholam@xtra.co.nz"));
    aboutData.addAuthor(i18n("Jochen Wilhemly"), i18n("KWrite Author"), QStringLiteral("digisnap@cs.tu-berlin.de"));
    aboutData.addAuthor(i18n("Michael Koch"), i18n("KWrite port to KParts"), QStringLiteral("koch@kde.org"));
    aboutData.addAuthor(i18n("Christian Gebauer"), QString(), QStringLiteral("gebauer@kde.org"));
    aboutData.addAuthor(i18n("Simon Hausmann"), QString(), QStringLiteral("hausmann@kde.org"));
    aboutData.addAuthor(i18n("Glen Parker"), i18n("KWrite Undo History, Kspell integration"), QStringLiteral("glenebob@nwlink.com"));
    aboutData.addAuthor(i18n("Scott Manson"), i18n("KWrite XML Syntax highlighting support"), QStringLiteral("sdmanson@alltel.net"));
    aboutData.addAuthor(i18n("John Firebaugh"), i18n("Patches and more"), QStringLiteral("jfirebaugh@kde.org"));
    aboutData.addAuthor(i18n("Gerald Senarclens de Grancy"),
                        i18n("QA and Scripting"),
                        QStringLiteral("oss@senarclens.eu"),
                        QStringLiteral("http://find-santa.eu/"));

    aboutData.addCredit(i18n("Matteo Merli"), i18n("Highlighting for RPM Spec-Files, Perl, Diff and more"), QStringLiteral("merlim@libero.it"));
    aboutData.addCredit(i18n("Rocky Scaletta"), i18n("Highlighting for VHDL"), QStringLiteral("rocky@purdue.edu"));
    aboutData.addCredit(i18n("Yury Lebedev"), i18n("Highlighting for SQL"));
    aboutData.addCredit(i18n("Chris Ross"), i18n("Highlighting for Ferite"));
    aboutData.addCredit(i18n("Nick Roux"), i18n("Highlighting for ILERPG"));
    aboutData.addCredit(i18n("Carsten Niehaus"), i18n("Highlighting for LaTeX"));
    aboutData.addCredit(i18n("Per Wigren"), i18n("Highlighting for Makefiles, Python"));
    aboutData.addCredit(i18n("Jan Fritz"), i18n("Highlighting for Python"));
    aboutData.addCredit(i18n("Daniel Naber"));
    aboutData.addCredit(i18n("Roland Pabel"), i18n("Highlighting for Scheme"));
    aboutData.addCredit(i18n("Cristi Dumitrescu"), i18n("PHP Keyword/Datatype list"));
    aboutData.addCredit(i18n("Carsten Pfeiffer"), i18nc("Credit text for someone that helped a lot", "Very nice help"));
    aboutData.addCredit(i18n("All people who have contributed and I have forgotten to mention"));

    /**
     * bugzilla
     */
    aboutData.setProductName(QByteArray("kate/kwrite"));

    /**
     * set and register app about data
     */
    KAboutData::setApplicationData(aboutData);

    /**
     * set the program icon
     */
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("accessories-text-editor"), app.windowIcon()));

    /**
     * Create command line parser and feed it with known options
     */
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    // -e/--encoding option
    const QCommandLineOption useEncoding(QStringList() << QStringLiteral("e") << QStringLiteral("encoding"),
                                         i18n("Set encoding for the file to open."),
                                         i18n("encoding"));
    parser.addOption(useEncoding);

    // -l/--line option
    const QCommandLineOption gotoLine(QStringList() << QStringLiteral("l") << QStringLiteral("line"), i18n("Navigate to this line."), i18n("line"));
    parser.addOption(gotoLine);

    // -c/--column option
    const QCommandLineOption gotoColumn(QStringList() << QStringLiteral("c") << QStringLiteral("column"), i18n("Navigate to this column."), i18n("column"));
    parser.addOption(gotoColumn);

    // -i/--stdin option
    const QCommandLineOption readStdIn(QStringList() << QStringLiteral("i") << QStringLiteral("stdin"), i18n("Read the contents of stdin."));
    parser.addOption(readStdIn);

    // --tempfile option
    const QCommandLineOption tempfile(QStringList() << QStringLiteral("tempfile"), i18n("The files/URLs opened by the application will be deleted after use"));
    parser.addOption(tempfile);

    // urls to open
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Documents to open."), i18n("[urls...]"));

    /**
     * do the command line parsing
     */
    parser.process(app);

    /**
     * handle standard options
     */
    aboutData.processCommandLine(&parser);

    KWriteApplication kapp;

    if (app.isSessionRestored()) {
        kapp.restore();
    } else {
        bool nav = false;
        int line = 0, column = 0;

        QTextCodec *codec =
            parser.isSet(QStringLiteral("encoding")) ? QTextCodec::codecForName(parser.value(QStringLiteral("encoding")).toLocal8Bit()) : nullptr;

        if (parser.isSet(QStringLiteral("line"))) {
            line = parser.value(QStringLiteral("line")).toInt() - 1;
            nav = true;
        }

        if (parser.isSet(QStringLiteral("column"))) {
            column = parser.value(QStringLiteral("column")).toInt() - 1;
            nav = true;
        }

        if (parser.positionalArguments().count() == 0) {
            KWrite *t = kapp.newWindow();

            if (parser.isSet(QStringLiteral("stdin"))) {
                QTextStream input(stdin, QIODevice::ReadOnly);

                // set chosen codec
                if (codec) {
                    input.setCodec(codec);
                }

                QString line;
                QString text;

                do {
                    line = input.readLine();
                    text.append(line + QLatin1Char('\n'));
                } while (!line.isNull());

                KTextEditor::Document *doc = t->activeView()->document();
                if (doc) {
                    // remember codec in document, e.g. to show the right one
                    if (codec) {
                        doc->setEncoding(QString::fromLatin1(codec->name()));
                    }
                    doc->setText(text);
                }
            }

            if (nav && t->activeView()) {
                t->activeView()->setCursorPosition(KTextEditor::Cursor(line, column));
            }
        } else {
            int docs_opened = 0;
            const auto positionalArguments = parser.positionalArguments();
            for (const QString &positionalArgument : positionalArguments) {
                UrlInfo info(positionalArgument);
                if (nav) {
                    info.cursor = KTextEditor::Cursor(line, column);
                }

                // this file is no local dir, open it, else warn
                bool noDir = !info.url.isLocalFile() || !QFileInfo(info.url.toLocalFile()).isDir();

                if (noDir) {
                    ++docs_opened;
                    KWrite *t = kapp.newWindow();

                    if (codec) {
                        t->activeView()->document()->setEncoding(QString::fromLatin1(codec->name()));
                    }

                    t->loadURL(info.url);

                    if (info.cursor.isValid()) {
                        t->activeView()->setCursorPosition(info.cursor);
                    } else if (info.url.hasQuery()) {
                        QUrlQuery q(info.url);
                        QString lineStr = q.queryItemValue(QStringLiteral("line"));
                        QString columnStr = q.queryItemValue(QStringLiteral("column"));

                        line = lineStr.toInt();
                        if (line > 0) {
                            line--;
                        }

                        column = columnStr.toInt();
                        if (column > 0) {
                            column--;
                        }

                        t->activeView()->setCursorPosition(KTextEditor::Cursor(line, column));
                    }
                } else {
                    KMessageBox::sorry(nullptr, i18n("The file '%1' could not be opened: it is not a normal file, it is a folder.", info.url.toString()));
                }
            }
            if (!docs_opened) {
                ::exit(1); // see https://bugs.kde.org/show_bug.cgi?id=124708
            }
        }
    }

    // no window there, uh, ohh, for example borked session config !!!
    // create at least one !!
    if (kapp.noWindows()) {
        kapp.newWindow();
    }

    /**
     * finally register this kwrite instance for dbus, don't die if no dbus is around!
     */
    const KDBusService dbusService(KDBusService::Multiple | KDBusService::NoExitOnFailure);

    /**
     * Run the event loop
     */
    return app.exec();
}
