/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-FileCopyrightText: 2021 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: MIT
*/
#include "gitblametooltip.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QScreen>
#include <QScrollBar>
#include <QString>
#include <QTextBrowser>
#include <QTimer>

#include <KTextEditor/ConfigInterface>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/State>

using KSyntaxHighlighting::AbstractHighlighter;
using KSyntaxHighlighting::Format;

static QString toHtmlRgbaString(const QColor &color)
{
    if (color.alpha() == 0xFF)
        return color.name();

    QString rgba = QStringLiteral("rgba(");
    rgba.append(QString::number(color.red()));
    rgba.append(QLatin1Char(','));
    rgba.append(QString::number(color.green()));
    rgba.append(QLatin1Char(','));
    rgba.append(QString::number(color.blue()));
    rgba.append(QLatin1Char(','));
    // this must be alphaF
    rgba.append(QString::number(color.alphaF()));
    rgba.append(QLatin1Char(')'));
    return rgba;
}

class HtmlHl : public AbstractHighlighter
{
public:
    HtmlHl()
        : out(&outputString)
    {
    }

    void setText(const QString &txt)
    {
        text = txt;
        QTextStream in(&text);

        out.reset();
        outputString.clear();

        bool inDiff = false;

        KSyntaxHighlighting::State state;
        out << "<pre>";
        while (!in.atEnd()) {
            currentLine = in.readLine();

            // allow empty lines in code blocks, no ruler here
            if (!inDiff && currentLine.isEmpty()) {
                out << "<hr>";
                continue;
            }

            // diff block
            if (!inDiff && currentLine.startsWith(QLatin1String("diff"))) {
                inDiff = true;
            }

            state = highlightLine(currentLine, state);
            out << "\n";
        }
        out << "</pre>";
    }

    QString html() const
    {
        //        while (!out.atEnd())
        //            qWarning() << out.readLine();
        return outputString;
    }

protected:
    void applyFormat(int offset, int length, const Format &format) override
    {
        if (!length)
            return;

        QString formatOutput;

        if (format.hasTextColor(theme())) {
            formatOutput = toHtmlRgbaString(format.textColor(theme()));
        }

        if (!formatOutput.isEmpty()) {
            out << "<span style=\"color:" << formatOutput << "\">";
        }

        out << currentLine.mid(offset, length).toHtmlEscaped();

        if (!formatOutput.isEmpty()) {
            out << "</span>";
        }
    }

private:
    QString text;
    QString currentLine;
    QString outputString;
    QTextStream out;
};

class GitBlameTooltip::Private : public QTextBrowser
{
    Q_OBJECT

public:
    QKeySequence m_ignoreKeySequence;

    static const uint64_t ModifierMask =
        Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier | Qt::GroupSwitchModifier;

    Private()
        : QTextBrowser(nullptr)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::BypassGraphicsProxyWidget | Qt::ToolTip);
        setWordWrapMode(QTextOption::NoWrap);
        document()->setDocumentMargin(10);
        setFrameStyle(QFrame::Box | QFrame::Raised);
        connect(&m_hideTimer, &QTimer::timeout, this, &Private::hideTooltip);

        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        m_htmlHl.setDefinition(m_syntaxHlRepo.definitionForName(QStringLiteral("Diff")));

        auto updateColors = [this](KTextEditor::Editor *e) {
            auto theme = e->theme();
            m_htmlHl.setTheme(theme);

            auto pal = palette();
            const QColor bg = theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor);
            pal.setColor(QPalette::Base, bg);
            const QColor normal = theme.textColor(KSyntaxHighlighting::Theme::Normal);
            pal.setColor(QPalette::Text, normal);
            setPalette(pal);
        };
        updateColors(KTextEditor::Editor::instance());
        connect(KTextEditor::Editor::instance(), &KTextEditor::Editor::configChanged, this, updateColors);
    }

    bool eventFilter(QObject *, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::ShortcutOverride: {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->matches(QKeySequence::Copy)) {
                copy();
            } else if (ke->matches(QKeySequence::SelectAll)) {
                selectAll();
            }
            event->accept();
            return true;
        }
        case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            int ignoreKey = 0;
            if (m_ignoreKeySequence.count() > 0) {
                ignoreKey = m_ignoreKeySequence[m_ignoreKeySequence.count() - 1] & ~ModifierMask;
            }
            if (ke->matches(QKeySequence::Copy) || ke->matches(QKeySequence::SelectAll) || (ignoreKey != 0 && ignoreKey == ke->key())
                || ke->key() == Qt::Key_Control || ke->key() == Qt::Key_Alt || ke->key() == Qt::Key_Shift || ke->key() == Qt::Key_AltGr
                || ke->key() == Qt::Key_Meta) {
                event->accept();
                return true;
            }
        } // fall through
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            hideTooltip();
            break;
        default:
            break;
        }
        return false;
    }

    void showTooltip(const QString &text, const QPointer<KTextEditor::View> view)
    {
        if (text.isEmpty() || !view) {
            return;
        }

        m_htmlHl.setText(text);
        setHtml(m_htmlHl.html());
        // view changed?
        // => update definition
        // => update font
        if (view != m_view) {
            if (m_view && m_view->focusProxy()) {
                m_view->focusProxy()->removeEventFilter(this);
            }
            m_view = view;
            // update font
            auto ciface = qobject_cast<KTextEditor::ConfigInterface *>(m_view);
            auto font = ciface->configValue(QStringLiteral("font")).value<QFont>();
            setFont(font);
            m_view->focusProxy()->installEventFilter(this);
        }

        const int scrollBarHeight = horizontalScrollBar()->height();
        QFontMetrics fm(font());
        QSize size = fm.size(Qt::TextSingleLine, QStringLiteral("m"));
        int fontHeight = size.height();
        size.setHeight(m_view->height() - fontHeight * 2 - scrollBarHeight);
        size.setWidth(qRound(m_view->width() * 0.7));
        resize(size);

        QPoint p = m_view->mapToGlobal(m_view->pos());
        p.setY(p.y() + fontHeight);
        p.setX(p.x() + m_view->textAreaRect().left() + m_view->textAreaRect().width() - size.width() - fontHeight);
        this->move(p);

        show();
    }

    Q_SLOT void hideTooltip()
    {
        if (m_view && m_view->focusProxy()) {
            m_view->focusProxy()->removeEventFilter(this);
        }
        close();
        setText(QString());
        m_inContextMenu = false;
    }

protected:
    void showEvent(QShowEvent *event) override
    {
        m_hideTimer.start(3000);
        return QTextBrowser::showEvent(event);
    }

    void enterEvent(QEvent *event) override
    {
        m_inContextMenu = false;
        m_hideTimer.stop();
        return QTextBrowser::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        if (!m_hideTimer.isActive() && !m_inContextMenu && textCursor().selectionStart() == textCursor().selectionEnd()) {
            hideTooltip();
        }
        return QTextBrowser::leaveEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        auto pos = event->pos();
        if (rect().contains(pos) || m_inContextMenu || textCursor().selectionStart() != textCursor().selectionEnd()) {
            return QTextBrowser::mouseMoveEvent(event);
        }
        hideTooltip();
    }

    void contextMenuEvent(QContextMenuEvent *event) override
    {
        m_inContextMenu = true;
        return QTextBrowser::contextMenuEvent(event);
    }

private:
    bool m_inContextMenu = false;
    QPointer<KTextEditor::View> m_view;
    QTimer m_hideTimer;
    HtmlHl m_htmlHl;
    KSyntaxHighlighting::Repository m_syntaxHlRepo;
};

GitBlameTooltip::GitBlameTooltip()
    : d(new GitBlameTooltip::Private())
{
}
GitBlameTooltip::~GitBlameTooltip()
{
    delete d;
}

void GitBlameTooltip::show(const QString &text, QPointer<KTextEditor::View> view)
{
    if (text.isEmpty() || !view || !view->document()) {
        return;
    }

    d->showTooltip(text, view);
}

void GitBlameTooltip::setIgnoreKeySequence(QKeySequence sequence)
{
    d->m_ignoreKeySequence = sequence;
}

#include "gitblametooltip.moc"
