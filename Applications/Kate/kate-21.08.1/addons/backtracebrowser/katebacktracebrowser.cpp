/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008-2014 Dominik Haumann <dhaumann kde org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

// BEGIN Includes
#include "katebacktracebrowser.h"

#include "btparser.h"

#include <KAboutData>
#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString> // i18n
#include <KPluginFactory>
#include <KSharedConfig>
#include <ktexteditor/view.h>

#include <QClipboard>
#include <QDataStream>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QUrl>
// END Includes

K_PLUGIN_FACTORY_WITH_JSON(KateBtBrowserFactory, "katebacktracebrowserplugin.json", registerPlugin<KateBtBrowserPlugin>();)

KateBtBrowserPlugin *KateBtBrowserPlugin::s_self = nullptr;
static QStringList fileExtensions = QStringList() << QStringLiteral("*.cpp") << QStringLiteral("*.cxx") << QStringLiteral("*.c") << QStringLiteral("*.cc")
                                                  << QStringLiteral("*.h") << QStringLiteral("*.hpp") << QStringLiteral("*.hxx") << QStringLiteral("*.moc");

KateBtBrowserPlugin::KateBtBrowserPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
    , indexer(&db)
{
    s_self = this;
    db.loadFromFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/katebtbrowser/backtracedatabase.db"));
}

KateBtBrowserPlugin::~KateBtBrowserPlugin()
{
    if (indexer.isRunning()) {
        indexer.cancel();
        indexer.wait();
    }

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/katebtbrowser");
    QDir().mkpath(path);
    db.saveToFile(path + QStringLiteral("/backtracedatabase.db"));

    s_self = nullptr;
}

KateBtBrowserPlugin &KateBtBrowserPlugin::self()
{
    return *s_self;
}

QObject *KateBtBrowserPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    KateBtBrowserPluginView *view = new KateBtBrowserPluginView(this, mainWindow);
    return view;
}

KateBtDatabase &KateBtBrowserPlugin::database()
{
    return db;
}

BtFileIndexer &KateBtBrowserPlugin::fileIndexer()
{
    return indexer;
}

void KateBtBrowserPlugin::startIndexer()
{
    if (indexer.isRunning()) {
        indexer.cancel();
        indexer.wait();
    }
    KConfigGroup cg(KSharedConfig::openConfig(), "backtracebrowser");
    indexer.setSearchPaths(cg.readEntry("search-folders", QStringList()));
    indexer.setFilter(cg.readEntry("file-extensions", fileExtensions));
    indexer.start();
    Q_EMIT newStatus(i18n("Indexing files..."));
}

int KateBtBrowserPlugin::configPages() const
{
    return 1;
}

KTextEditor::ConfigPage *KateBtBrowserPlugin::configPage(int number, QWidget *parent)
{
    if (number == 0) {
        return new KateBtConfigWidget(parent);
    }

    return nullptr;
}

KateBtBrowserPluginView::KateBtBrowserPluginView(KateBtBrowserPlugin *plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_plugin(plugin)
{
    // init console
    QWidget *toolview = mainWindow->createToolView(plugin,
                                                   QStringLiteral("kate_private_plugin_katebacktracebrowserplugin"),
                                                   KTextEditor::MainWindow::Bottom,
                                                   QIcon::fromTheme(QStringLiteral("tools-report-bug")),
                                                   i18n("Backtrace Browser"));
    m_widget = new KateBtBrowserWidget(mainWindow, toolview);

    connect(plugin, &KateBtBrowserPlugin::newStatus, m_widget, &KateBtBrowserWidget::setStatus);
}

KateBtBrowserPluginView::~KateBtBrowserPluginView()
{
    // cleanup, kill toolview + widget
    QWidget *toolview = m_widget->parentWidget();
    delete m_widget;
    delete toolview;
}

KateBtBrowserWidget::KateBtBrowserWidget(KTextEditor::MainWindow *mainwindow, QWidget *parent)
    : QWidget(parent)
    , mw(mainwindow)
{
    setupUi(this);

    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &KateBtBrowserWidget::clearStatus);
    connect(btnBacktrace, &QPushButton::clicked, this, &KateBtBrowserWidget::loadFile);
    connect(btnClipboard, &QPushButton::clicked, this, &KateBtBrowserWidget::loadClipboard);
    connect(btnConfigure, &QPushButton::clicked, this, &KateBtBrowserWidget::configure);
    connect(lstBacktrace, &QTreeWidget::itemActivated, this, &KateBtBrowserWidget::itemActivated);
}

KateBtBrowserWidget::~KateBtBrowserWidget()
{
}

void KateBtBrowserWidget::loadFile()
{
    QString url = QFileDialog::getOpenFileName(mw->window(), i18n("Load Backtrace"));
    QFile f(url);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString str = QString::fromUtf8(f.readAll());
        loadBacktrace(str);
    }
}

void KateBtBrowserWidget::loadClipboard()
{
    QString bt = QApplication::clipboard()->text();
    loadBacktrace(bt);
}

void KateBtBrowserWidget::loadBacktrace(const QString &bt)
{
    const QList<BtInfo> infos = KateBtParser::parseBacktrace(bt);

    lstBacktrace->clear();
    for (const BtInfo &info : infos) {
        QTreeWidgetItem *it = new QTreeWidgetItem(lstBacktrace);
        it->setData(0, Qt::DisplayRole, QString::number(info.step));
        it->setData(0, Qt::ToolTipRole, QString::number(info.step));
        QFileInfo fi(info.filename);
        it->setData(1, Qt::DisplayRole, fi.fileName());
        it->setData(1, Qt::ToolTipRole, info.filename);

        if (info.type == BtInfo::Source) {
            it->setData(2, Qt::DisplayRole, QString::number(info.line));
            it->setData(2, Qt::ToolTipRole, QString::number(info.line));
            it->setData(2, Qt::UserRole, QVariant(info.line));
        }
        it->setData(3, Qt::DisplayRole, info.function);
        it->setData(3, Qt::ToolTipRole, info.function);

        lstBacktrace->addTopLevelItem(it);
    }
    lstBacktrace->resizeColumnToContents(0);
    lstBacktrace->resizeColumnToContents(1);
    lstBacktrace->resizeColumnToContents(2);

    if (lstBacktrace->topLevelItemCount()) {
        setStatus(i18n("Loading backtrace succeeded"));
    } else {
        setStatus(i18n("Loading backtrace failed"));
    }
}

void KateBtBrowserWidget::configure()
{
    KateBtConfigDialog dlg(mw->window());
    dlg.exec();
}

void KateBtBrowserWidget::itemActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    QVariant variant = item->data(2, Qt::UserRole);
    if (variant.isValid()) {
        int line = variant.toInt();
        QString file = QDir::fromNativeSeparators(item->data(1, Qt::ToolTipRole).toString());
        file = QDir::cleanPath(file);

        QString path = file;
        // if not absolute path + exists, try to find with index
        if (!QFile::exists(path)) {
            // try to match the backtrace forms ".*/foo/bar.txt" and "foo/bar.txt"
            static const QRegularExpression rx1(QStringLiteral("/([^/]+)/([^/]+)$"));
            QRegularExpressionMatch match = rx1.match(file);
            if (match.hasMatch()) {
                file = match.captured(1) + QLatin1Char('/') + match.captured(2);
            } else {
                static const QRegularExpression rx2(QStringLiteral("([^/]+)/([^/]+)$"));
                if (rx2.match(file).hasMatch()) {
                    // file is of correct form
                } else {
                    qDebug() << "file patter did not match:" << file;
                    setStatus(i18n("File not found: %1", file));
                    return;
                }
            }
            path = KateBtBrowserPlugin::self().database().value(file);
        }

        if (!path.isEmpty() && QFile::exists(path)) {
            KTextEditor::View *kv = mw->openUrl(QUrl::fromLocalFile(path));
            kv->setCursorPosition(KTextEditor::Cursor(line - 1, 0));
            kv->setFocus();
            setStatus(i18n("Opened file: %1", file));
        }
    } else {
        setStatus(i18n("No debugging information available"));
    }
}

void KateBtBrowserWidget::setStatus(const QString &status)
{
    lblStatus->setText(status);
    timer.start(10 * 1000);
}

void KateBtBrowserWidget::clearStatus()
{
    lblStatus->setText(QString());
}

KateBtConfigWidget::KateBtConfigWidget(QWidget *parent)
    : KTextEditor::ConfigPage(parent)
{
    setupUi(this);
    edtUrl->setMode(KFile::Directory);
    edtUrl->setUrl(QUrl(QDir().absolutePath()));

    reset();

    connect(btnAdd, &QPushButton::clicked, this, &KateBtConfigWidget::add);
    connect(btnRemove, &QPushButton::clicked, this, &KateBtConfigWidget::remove);
    connect(edtExtensions, &QLineEdit::textChanged, this, &KateBtConfigWidget::textChanged);

    m_changed = false;
}

KateBtConfigWidget::~KateBtConfigWidget()
{
}

QString KateBtConfigWidget::name() const
{
    return i18n("Backtrace");
}

QString KateBtConfigWidget::fullName() const
{
    return i18n("Backtrace Settings");
}

QIcon KateBtConfigWidget::icon() const
{
    return QIcon::fromTheme(QStringLiteral("tools-report-bug"));
}

void KateBtConfigWidget::apply()
{
    if (m_changed) {
        QStringList sl;
        for (int i = 0; i < lstFolders->count(); ++i) {
            sl << lstFolders->item(i)->data(Qt::DisplayRole).toString();
        }
        KConfigGroup cg(KSharedConfig::openConfig(), "backtracebrowser");
        cg.writeEntry("search-folders", sl);

        QString filter = edtExtensions->text();
        filter.replace(QLatin1Char(','), QLatin1Char(' ')).replace(QLatin1Char(';'), QLatin1Char(' '));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        cg.writeEntry("file-extensions", filter.split(QLatin1Char(' '), QString::SkipEmptyParts));
#else
        cg.writeEntry("file-extensions", filter.split(QLatin1Char(' '), Qt::SkipEmptyParts));
#endif

        KateBtBrowserPlugin::self().startIndexer();
        m_changed = false;
    }
}

void KateBtConfigWidget::reset()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "backtracebrowser");
    lstFolders->clear();
    lstFolders->addItems(cg.readEntry("search-folders", QStringList()));
    edtExtensions->setText(cg.readEntry("file-extensions", fileExtensions).join(QLatin1Char(' ')));
}

void KateBtConfigWidget::defaults()
{
    lstFolders->clear();
    edtExtensions->setText(fileExtensions.join(QLatin1Char(' ')));

    m_changed = true;
}

void KateBtConfigWidget::add()
{
    QDir url(edtUrl->lineEdit()->text());
    if (url.exists()) {
        if (lstFolders->findItems(url.absolutePath(), Qt::MatchExactly).empty()) {
            lstFolders->addItem(url.absolutePath());
            Q_EMIT changed();
            m_changed = true;
        }
    }
}

void KateBtConfigWidget::remove()
{
    QListWidgetItem *item = lstFolders->currentItem();
    if (item) {
        delete item;
        Q_EMIT changed();
        m_changed = true;
    }
}

void KateBtConfigWidget::textChanged()
{
    Q_EMIT changed();
    m_changed = true;
}

KateBtConfigDialog::KateBtConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Backtrace Browser Settings"));

    m_configWidget = new KateBtConfigWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QDialogButtonBox *box = new QDialogButtonBox(this);
    box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(m_configWidget);
    layout->addWidget(box);

    connect(this, &KateBtConfigDialog::accepted, m_configWidget, &KateBtConfigWidget::apply);
    connect(box, &QDialogButtonBox::accepted, this, &KateBtConfigDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &KateBtConfigDialog::reject);
}

KateBtConfigDialog::~KateBtConfigDialog()
{
}

#include "katebacktracebrowser.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
