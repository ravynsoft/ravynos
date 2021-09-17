/*
    SPDX-FileCopyrightText: 2014-2019 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2020 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gotosymbolwidget.h"
#include "gotoglobalsymbolmodel.h"
#include "gotosymbolmodel.h"
#include "gotosymboltreeview.h"
#include "kate_ctags_view.h"
#include "tags.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QVBoxLayout>

#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KTextEditor/View>

class QuickOpenFilterProxyModel : public QSortFilterProxyModel
{
public:
    QuickOpenFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        const QString fileName = sourceModel()->index(sourceRow, 0, sourceParent).data().toString();
        for (const QString &str : m_filterStrings) {
            if (!fileName.contains(str, Qt::CaseInsensitive)) {
                return false;
            }
        }
        return true;
    }

    QStringList filterStrings() const
    {
        return m_filterStrings;
    }

public Q_SLOTS:
    void setFilterText(const QString &text)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        m_filterStrings = text.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
        m_filterStrings = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif

        invalidateFilter();
    }

private:
    QStringList m_filterStrings;
};

class GotoStyleDelegate : public QStyledItemDelegate
{
public:
    GotoStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        QTextDocument doc;

        QString str = index.data().toString();
        for (const auto &string : m_filterStrings) {
            // FIXME: This will skip the letter 'b' if the string
            // has only one letter so that we don't match inside
            // <b> tags.
            if (string == QLatin1String("b")) {
                continue;
            }
            const QRegularExpression re(QStringLiteral("(") + QRegularExpression::escape(string) + QStringLiteral(")"),
                                        QRegularExpression::CaseInsensitiveOption);
            str.replace(re, QStringLiteral("<b>\\1</b>"));
        }

        auto file = index.data(GotoGlobalSymbolModel::FileUrl).toString();
        // this will be empty for local symbol mode
        if (!file.isEmpty()) {
            str += QStringLiteral(" &nbsp;<span style=\"color: gray;\">") + QFileInfo(file).fileName() + QStringLiteral("</span>");
        }

        doc.setHtml(str);
        doc.setDocumentMargin(2);

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        // draw text
        painter->translate(option.rect.x(), option.rect.y());
        if (index.column() == 0) {
            painter->translate(25, 0);
        }
        doc.drawContents(painter);

        painter->restore();
    }

public Q_SLOTS:
    void setFilterStrings(const QString &text)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        m_filterStrings = text.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
        m_filterStrings = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif
    }

private:
    QStringList m_filterStrings;
};

GotoSymbolWidget::GotoSymbolWidget(KTextEditor::MainWindow *mainWindow, KateCTagsView *pluginView, QWidget *widget)
    : QWidget(widget)
    , ctagsPluginView(pluginView)
    , m_mainWindow(mainWindow)
    , oldPos(-1, -1)
{
    setWindowFlags(Qt::FramelessWindowHint);

    mode = Local;

    m_treeView = new GotoSymbolTreeView(mainWindow, this);
    m_styleDelegate = new GotoStyleDelegate(this);
    m_treeView->setItemDelegate(m_styleDelegate);
    m_lineEdit = new QLineEdit(this);

    setFocusProxy(m_lineEdit);

    m_proxyModel = new QuickOpenFilterProxyModel(this);
    m_proxyModel->setSortRole(Qt::DisplayRole);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(0);

    m_symbolsModel = new GotoSymbolModel(this);
    m_globalSymbolsModel = new GotoGlobalSymbolModel(this);

    m_proxyModel->setSourceModel(m_symbolsModel);
    m_treeView->setModel(m_proxyModel);

    connect(m_lineEdit, &QLineEdit::textChanged, m_proxyModel, &QuickOpenFilterProxyModel::setFilterText);
    connect(m_lineEdit, &QLineEdit::textChanged, m_styleDelegate, &GotoStyleDelegate::setFilterStrings);
    connect(m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        m_treeView->viewport()->update();
    });
    connect(m_lineEdit, &QLineEdit::textChanged, this, &GotoSymbolWidget::loadGlobalSymbols);
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &GotoSymbolWidget::slotReturnPressed);

    connect(m_treeView, &QTreeView::activated, this, &GotoSymbolWidget::slotReturnPressed);
    connect(m_proxyModel, &QSortFilterProxyModel::rowsInserted, this, &GotoSymbolWidget::reselectFirst);
    connect(m_proxyModel, &QSortFilterProxyModel::rowsRemoved, this, &GotoSymbolWidget::reselectFirst);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_treeView);
    setLayout(layout);

    m_treeView->installEventFilter(this);
    m_lineEdit->installEventFilter(this);
}

bool GotoSymbolWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == m_lineEdit) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(m_treeView, event);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Escape) {
                if (oldPos.isValid()) {
                    m_mainWindow->activeView()->setCursorPosition(oldPos);
                }
                m_lineEdit->clear();
                keyEvent->accept();
                hide();
                return true;
            }
        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down) && (keyEvent->key() != Qt::Key_PageUp)
                && (keyEvent->key() != Qt::Key_PageDown) && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(m_lineEdit, event);
                return true;
            }
        }
    }

    else if (event->type() == QEvent::FocusOut && !(m_lineEdit->hasFocus() || m_treeView->hasFocus())) {
        m_lineEdit->clear();
        hide();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void GotoSymbolWidget::showSymbols(const QString &filePath)
{
    changeMode(Local);
    oldPos = m_mainWindow->activeView()->cursorPosition();
    m_symbolsModel->refresh(filePath);
    updateViewGeometry();
    reselectFirst();
}

void GotoSymbolWidget::showGlobalSymbols(const QString &tagFilePath)
{
    changeMode(Global);
    m_tagFile = tagFilePath;
    updateViewGeometry();
}

void GotoSymbolWidget::loadGlobalSymbols(const QString &text)
{
    if (m_tagFile.isEmpty() || !QFileInfo::exists(m_tagFile) || !QFileInfo(m_tagFile).isFile()) {
        Tags::TagEntry e(i18n("Tags file not found. Please generate one manually or using the CTags plugin"), QString(), QString(), QString());
        m_globalSymbolsModel->setSymbolsData({e});
        return;
    }

    if (text.length() < 3 || mode == Local) {
        return;
    }

    QString currentWord = text;
    Tags::TagList list = Tags::getPartialMatchesNoi8n(m_tagFile, currentWord);

    if (list.isEmpty()) {
        return;
    }

    m_globalSymbolsModel->setSymbolsData(std::move(list));
    updateViewGeometry();
    reselectFirst();
}

void GotoSymbolWidget::slotReturnPressed()
{
    const auto idx = m_proxyModel->index(m_treeView->currentIndex().row(), 0);
    if (!idx.isValid()) {
        return;
    }

    if (mode == Global) {
        QString tag = idx.data(Qt::UserRole).toString();
        QString pattern = idx.data(GotoGlobalSymbolModel::Pattern).toString();
        QString file = idx.data(GotoGlobalSymbolModel::FileUrl).toString();
        bool fileFound = true;

        QFileInfo fi(file);
        QString url;
        // if the file doesn't exist, try to load it using project base dir
        if (!fi.exists()) {
            fileFound = false;
            QObject *projectView = m_mainWindow->pluginView(QStringLiteral("kateprojectplugin"));
            QString ret = projectView ? projectView->property("projectBaseDir").toString() : QString();
            if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/'))) {
                ret.append(QLatin1Char('/'));
            }
            url = ret + file;
            fi.setFile(url);

            // check again
            // not found? use tagFile path as base path
            if (!fi.exists()) {
                url.clear();
                fi.setFile(m_tagFile);
                QString path = fi.absolutePath();
                url = path + QStringLiteral("/") + file;

                fi.setFile(url);
                if (fi.exists()) {
                    fileFound = true;
                }
            } else {
                fileFound = true;
            }
        } else {
            url = file;
        }

        if (fileFound) {
            ctagsPluginView->jumpToTag(url, pattern, tag);
        } else {
            QString msg = i18n("File for '%1' not found.", tag);
            auto message = new KTextEditor::Message(msg, KTextEditor::Message::MessageType::Error);
            if (auto view = m_mainWindow->activeView()) {
                view->document()->postMessage(message);
            }
        }

    } else {
        int line = idx.data(Qt::UserRole).toInt();

        // try to find the start position of this tag
        // and put the cursor there
        QString tag = idx.data().toString();
        QString textLine = m_mainWindow->activeView()->document()->line(--line);
        int col = textLine.indexOf(tag.midRef(0, 4));
        col = col >= 0 ? col : 0;
        KTextEditor::Cursor c(line, col);

        m_mainWindow->activeView()->setCursorPosition(c);
    }

    // block signals, so that rowsInserted isn't emitted causing us to loose position
    const QSignalBlocker blocker(m_proxyModel);

    m_lineEdit->clear();
    hide();
}

void GotoSymbolWidget::changeMode(GotoSymbolWidget::Mode newMode)
{
    mode = newMode;
    if (mode == Global) {
        m_proxyModel->setSourceModel(m_globalSymbolsModel);
        m_treeView->setGlobalMode(true);
    } else if (mode == Local) {
        m_proxyModel->setSourceModel(m_symbolsModel);
        m_treeView->setGlobalMode(false);
    }
}

void GotoSymbolWidget::updateViewGeometry()
{
    QWidget *window = m_mainWindow->window();
    const QSize centralSize = window->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    const int rowHeight = m_treeView->sizeHintForRow(0) == -1 ? 0 : m_treeView->sizeHintForRow(0);

    int frameWidth = this->frameSize().width();
    frameWidth = frameWidth > centralSize.width() / 2.4 ? centralSize.width() / 2.4 : frameWidth;

    const int width = viewMaxSize.width();

    const int rowCount = mode == Global ? m_globalSymbolsModel->rowCount() : m_symbolsModel->rowCount();

    const QSize viewSize(width, std::min(std::max(rowHeight * rowCount + 2 * frameWidth, rowHeight * 6), viewMaxSize.height()));

    // Position should be central over the editor area, so map to global from
    // parent of central widget since the view is positioned in global coords
    const QPoint centralWidgetPos = window->parentWidget() ? window->mapToGlobal(window->pos()) : window->pos();
    const int xPos = std::max(0, centralWidgetPos.x() + (centralSize.width() - viewSize.width()) / 2);
    const int yPos = std::max(0, centralWidgetPos.y() + (centralSize.height() - viewSize.height()) * 1 / 4);

    move(xPos, yPos);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
    animation->setDuration(150);
    animation->setStartValue(this->size());
    animation->setEndValue(viewSize);

    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void GotoSymbolWidget::reselectFirst()
{
    QModelIndex index = m_proxyModel->index(0, 0);
    if (index.isValid()) {
        m_treeView->setCurrentIndex(index);
    }
}
