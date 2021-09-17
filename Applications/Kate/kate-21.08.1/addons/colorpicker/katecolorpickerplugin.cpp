/*
    SPDX-FileCopyrightText: 2018 Sven Brauch <mail@svenbrauch.de>
    SPDX-FileCopyrightText: 2018 Michal Srb <michalsrb@gmail.com>
    SPDX-FileCopyrightText: 2020 Jan Paul Batrina <jpmbatrina01@gmail.com>
    SPDX-FileCopyrightText: 2021 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecolorpickerplugin.h"
#include "colorpickerconfigpage.h"

#include <algorithm>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KTextEditor/Document>
#include <KTextEditor/InlineNoteInterface>
#include <KTextEditor/InlineNoteProvider>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QColor>
#include <QColorDialog>
#include <QFontMetricsF>
#include <QHash>
#include <QPainter>
#include <QRegularExpression>
#include <QVariant>

ColorPickerInlineNoteProvider::ColorPickerInlineNoteProvider(KTextEditor::Document *doc)
    : m_doc(doc)
{
    // initialize the color regex
    m_colorRegex.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
    updateColorMatchingCriteria();

    for (auto view : m_doc->views()) {
        qobject_cast<KTextEditor::InlineNoteInterface *>(view)->registerInlineNoteProvider(this);
    }

    connect(m_doc, &KTextEditor::Document::viewCreated, this, [this](KTextEditor::Document *, KTextEditor::View *view) {
        qobject_cast<KTextEditor::InlineNoteInterface *>(view)->registerInlineNoteProvider(this);
    });

    auto lineChanged = [this](const int line) {
        if (m_startChangedLines == -1 || m_endChangedLines == -1) {
            m_startChangedLines = line;
            // changed line is directly above/below the previous changed line, so we just update them
        } else if (line == m_endChangedLines) { // handled below. Condition added here to avoid fallthrough
        } else if (line == m_startChangedLines - 1) {
            m_startChangedLines = line;
        } else if (line < m_startChangedLines || line > m_endChangedLines) {
            // changed line is outside the range of previous changes. Change proably skipped lines
            updateNotes(m_startChangedLines, m_endChangedLines);
            m_startChangedLines = line;
            m_endChangedLines = -1;
        }

        m_endChangedLines = line >= m_endChangedLines ? line + 1 : m_endChangedLines;
    };

    // textInserted and textRemoved are emitted per line, then the last line is followed by a textChanged signal
    connect(m_doc, &KTextEditor::Document::textInserted, this, [lineChanged](KTextEditor::Document *, const KTextEditor::Cursor &cur, const QString &) {
        lineChanged(cur.line());
    });
    connect(m_doc, &KTextEditor::Document::textRemoved, this, [lineChanged](KTextEditor::Document *, const KTextEditor::Range &range, const QString &) {
        lineChanged(range.start().line());
    });
    connect(m_doc, &KTextEditor::Document::textChanged, this, [this](KTextEditor::Document *) {
        int newNumLines = m_doc->lines();
        if (m_startChangedLines == -1) {
            // textChanged not preceded by textInserted or textRemoved. This probably means that either:
            //   *empty line(s) were inserted/removed (TODO: Update only the lines directly below the removed/inserted empty line(s))
            //   *the document is newly opened so we update all lines
            updateNotes();
        } else {
            if (m_previousNumLines != newNumLines) {
                // either whole line(s) were removed or inserted. We update all lines (even those that are now non-existent) below m_startChangedLines
                m_endChangedLines = newNumLines > m_previousNumLines ? newNumLines : m_previousNumLines;
            }
            updateNotes(m_startChangedLines, m_endChangedLines);
        }

        m_startChangedLines = -1;
        m_endChangedLines = -1;
        m_previousNumLines = newNumLines;
    });

    updateNotes();
}

ColorPickerInlineNoteProvider::~ColorPickerInlineNoteProvider()
{
    for (auto view : m_doc->views()) {
        qobject_cast<KTextEditor::InlineNoteInterface *>(view)->unregisterInlineNoteProvider(this);
    }
}

void ColorPickerInlineNoteProvider::updateColorMatchingCriteria()
{
    KConfigGroup config(KSharedConfig::openConfig(), "ColorPicker");
    m_matchHexLengths = config.readEntry("HexLengths", QList<int>{12, 9, 6, 3}).toVector();
    m_putPreviewAfterColor = config.readEntry("PreviewAfterColor", true);
    m_matchNamedColors = config.readEntry("NamedColors", false);

    QString colorRegex;
    if (m_matchHexLengths.size() > 0) {
        colorRegex += QLatin1String("(#[[:xdigit:]]{3,12})");
    }

    if (m_matchNamedColors) {
        if (!colorRegex.isEmpty()) {
            colorRegex += QLatin1Char('|');
        }
        // shortest and longest colors have 3 (e.g. red) and 20 (lightgoldenrodyellow) characters respectively
        colorRegex += QLatin1String("((?<![\\w])[a-z]{3,20})");
    }

    if (!colorRegex.isEmpty()) {
        colorRegex = QStringLiteral("(?<![-])(%1)(?![-\\w])").arg(colorRegex);
    } else {
        // No matching criteria enabled. Set regex to negative lookahead to match nothing.
        colorRegex = QLatin1String("(?!)");
    }

    m_colorRegex.setPattern(colorRegex);
}

void ColorPickerInlineNoteProvider::updateNotes(int startLine, int endLine)
{
    startLine = startLine < -1 ? -1 : startLine;
    if (startLine == -1) {
        startLine = 0;
        //  we use whichever of newNumLines and m_previousNumLines are longer so that note indices for non-existent lines are also removed
        const int lastLine = m_doc->lines();
        endLine = lastLine > m_previousNumLines ? lastLine : m_previousNumLines;
    }

    if (endLine == -1) {
        endLine = startLine;
    }

    for (int line = startLine; line < endLine; ++line) {
        m_colorNoteIndices.remove(line);
        Q_EMIT inlineNotesChanged(line);
    }
}

QVector<int> ColorPickerInlineNoteProvider::inlineNotes(int line) const
{
    if (!m_colorNoteIndices.contains(line)) {
        m_colorNoteIndices.insert(line, {});

        const QString lineText = m_doc->line(line);
        auto matchIter = m_colorRegex.globalMatch(lineText);
        while (matchIter.hasNext()) {
            const auto match = matchIter.next();
            if (!QColor(match.captured()).isValid()) {
                continue;
            }

            if (lineText.at(match.capturedStart()) == QLatin1Char('#') && !m_matchHexLengths.contains(match.capturedLength() - 1)) {
                // matching for this hex color format is disabled
                continue;
            }

            int start = match.capturedStart();
            int end = start + match.capturedLength();
            if (m_putPreviewAfterColor) {
                start = end;
                end = match.capturedStart();
            }

            m_colorNoteIndices[line].colorNoteIndices.append(start);
            m_colorNoteIndices[line].otherColorIndices.append(end);
        }
    }

    return m_colorNoteIndices[line].colorNoteIndices;
}

QSize ColorPickerInlineNoteProvider::inlineNoteSize(const KTextEditor::InlineNote &note) const
{
    return QSize(note.lineHeight() - 1, note.lineHeight() - 1);
}

void ColorPickerInlineNoteProvider::paintInlineNote(const KTextEditor::InlineNote &note, QPainter &painter) const
{
    const auto line = note.position().line();
    auto colorEnd = note.position().column();

    const QVector<int> &colorNoteIndices = m_colorNoteIndices[line].colorNoteIndices;
    // Since the colorNoteIndices are inserted in left-to-right (increasing) order in inlineNotes(), we can use binary search to find the index (or color note
    // number) for the line
    const int colorNoteNumber = std::lower_bound(colorNoteIndices.cbegin(), colorNoteIndices.cend(), colorEnd) - colorNoteIndices.cbegin();
    auto colorStart = m_colorNoteIndices[line].otherColorIndices[colorNoteNumber];

    if (colorStart > colorEnd) {
        colorEnd = colorStart;
        colorStart = note.position().column();
    }

    const auto color = QColor(m_doc->text({line, colorStart, line, colorEnd}));
    // ensure that the border color is always visible
    QColor penColor = color;
    penColor.setAlpha(255);
    painter.setPen(penColor.value() < 128 ? penColor.lighter(150) : penColor.darker(150));

    painter.setBrush(color);
    painter.setRenderHint(QPainter::Antialiasing, false);
    const QFontMetricsF fm(note.font());
    const int inc = note.underMouse() ? 1 : 0;
    const int ascent = fm.ascent();
    const int margin = (note.lineHeight() - ascent) / 2;
    painter.drawRect(margin - inc, margin - inc, ascent - 1 + 2 * inc, ascent - 1 + 2 * inc);
}

void ColorPickerInlineNoteProvider::inlineNoteActivated(const KTextEditor::InlineNote &note, Qt::MouseButtons, const QPoint &)
{
    const auto line = note.position().line();
    auto colorEnd = note.position().column();

    const QVector<int> &colorNoteIndices = m_colorNoteIndices[line].colorNoteIndices;
    // Since the colorNoteIndices are inserted in left-to-right (increasing) order in inlineNotes, we can use binary search to find the index (or color note
    // number) for the line
    const int colorNoteNumber = std::lower_bound(colorNoteIndices.cbegin(), colorNoteIndices.cend(), colorEnd) - colorNoteIndices.cbegin();
    auto colorStart = m_colorNoteIndices[line].otherColorIndices[colorNoteNumber];
    if (colorStart > colorEnd) {
        colorEnd = colorStart;
        colorStart = note.position().column();
    }

    const auto oldColor = QColor(m_doc->text({line, colorStart, line, colorEnd}));
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
    QString title = i18n("Select Color (Hex output)");
    if (!m_doc->isReadWrite()) {
        dialogOptions |= QColorDialog::NoButtons;
        title = i18n("View Color [Read only]");
    }
    const QColor newColor = QColorDialog::getColor(oldColor, const_cast<KTextEditor::View *>(note.view()), title, dialogOptions);
    if (!newColor.isValid()) {
        return;
    }

    // include alpha channel if the new color has transparency or the old color included transparency (#AARRGGBB, 9 hex digits)
    auto colorNameFormat = (newColor.alpha() != 255 || colorEnd - colorStart == 9) ? QColor::HexArgb : QColor::HexRgb;
    m_doc->replaceText({line, colorStart, line, colorEnd}, newColor.name(colorNameFormat));
}

K_PLUGIN_FACTORY_WITH_JSON(KateColorPickerPluginFactory, "katecolorpickerplugin.json", registerPlugin<KateColorPickerPlugin>();)

KateColorPickerPlugin::KateColorPickerPlugin(QObject *parent, const QList<QVariant> &)
    : KTextEditor::Plugin(parent)
{
}

KateColorPickerPlugin::~KateColorPickerPlugin()
{
    qDeleteAll(m_inlineColorNoteProviders);
}

QObject *KateColorPickerPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    for (auto view : m_mainWindow->views()) {
        addDocument(view->document());
    }

    connect(m_mainWindow, &KTextEditor::MainWindow::viewCreated, this, [this](KTextEditor::View *view) {
        addDocument(view->document());
    });

    return nullptr;
}

void KateColorPickerPlugin::addDocument(KTextEditor::Document *doc)
{
    if (!m_inlineColorNoteProviders.contains(doc)) {
        m_inlineColorNoteProviders.insert(doc, new ColorPickerInlineNoteProvider(doc));
    }

    connect(doc, &KTextEditor::Document::destroyed, this, [this, doc]() {
        m_inlineColorNoteProviders.remove(doc);
    });
}

void KateColorPickerPlugin::readConfig()
{
    for (auto colorNoteProvider : m_inlineColorNoteProviders.values()) {
        colorNoteProvider->updateColorMatchingCriteria();
        colorNoteProvider->updateNotes();
    }
}

#include "katecolorpickerplugin.moc"
