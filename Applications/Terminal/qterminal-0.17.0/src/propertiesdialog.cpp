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

#include <QDebug>
#include <QStyleFactory>
#include <QFileDialog>
#include <QScreen>
#include <QWindow>

#include "propertiesdialog.h"
#include "properties.h"
#include "fontdialog.h"
#include "config.h"
#include "qterminalapp.h"

void KeySequenceEdit::keyPressEvent(QKeyEvent* event)
{
    // by not allowing multiple shortcuts,
    // the Qt bug that makes Meta a non-modifier is worked around
    clear();
    QKeySequenceEdit::keyPressEvent(event);
}

Delegate::Delegate (QObject *parent)
    : QStyledItemDelegate (parent)
{
}

QWidget* Delegate::createEditor(QWidget *parent,
                                const QStyleOptionViewItem& /*option*/,
                                const QModelIndex& /*index*/) const
{
    return new KeySequenceEdit(parent);
}

bool Delegate::eventFilter(QObject *object, QEvent *event)
{
    KeySequenceEdit *editor = qobject_cast<KeySequenceEdit*>(object);
    if (editor && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        int k = ke->key();
        if (k == Qt::Key_Return || k == Qt::Key_Enter) {
            emit QAbstractItemDelegate::commitData(editor);
            emit QAbstractItemDelegate::closeEditor(editor);
            return true;
        }
        // treat Tab and Backtab like other keys
        else if(k == Qt::Key_Tab || k ==  Qt::Key_Backtab) {
            editor->pressKey(ke);
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter (object, event);
}

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PropertiesDialog::apply);
    connect(changeFontButton, &QPushButton::clicked,
            this, &PropertiesDialog::changeFontButton_clicked);
    connect(chooseBackgroundImageButton, &QPushButton::clicked,
            this, &PropertiesDialog::chooseBackgroundImageButton_clicked);

    // fixed size
    connect(saveSizeOnExitCheckBox, &QCheckBox::stateChanged, [this] (int state) {
        fixedSizeLabel->setEnabled(state == Qt::Unchecked);
        xLabel->setEnabled(state == Qt::Unchecked);
        fixedWithSpinBox->setEnabled(state == Qt::Unchecked);
        fixedHeightSpinBox->setEnabled(state == Qt::Unchecked);
        getCurrentSizeButton->setEnabled(state == Qt::Unchecked);
    });
    connect(getCurrentSizeButton, &QAbstractButton::clicked, [this, parent] {
        if (parent != nullptr)
        {
            QSize pSize = parent->window()->geometry().size();
            fixedWithSpinBox->setValue(pSize.width());
            fixedHeightSpinBox->setValue(pSize.height());
        }
    });
    QSize ag;
    QSize minWinSize(0, 0);
    if (parent != nullptr)
    {
        minWinSize = parent->minimumSize();
        if (QWindow *win = parent->windowHandle())
        {
            if (QScreen *sc = win->screen())
            {
                ag = sc->availableVirtualGeometry().size()
                     // also consider the parent frame thickness because the parent window is fully formed
                     - (parent->window()->frameGeometry().size()
                        - parent->window()->geometry().size());
            }
        }
    }
    fixedWithSpinBox->setMinimum(minWinSize.width());
    fixedHeightSpinBox->setMinimum(minWinSize.height());
    if (!ag.isEmpty())
    {
        fixedWithSpinBox->setMaximum(qMax(ag.width(), minWinSize.width()));
        fixedHeightSpinBox->setMaximum(qMax(ag.height() , minWinSize.height()));
    }

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();
    colorSchemes.sort(Qt::CaseInsensitive);

    listWidget->setCurrentRow(0);
    // resize the list widget to its content
    listWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    listWidget->setMaximumWidth(listWidget->sizeHintForColumn(0) + 2 * listWidget->frameWidth() + 4);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(Properties::Instance()->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    backgroundImageLineEdit->setText(Properties::Instance()->backgroundImage);

    backgroundModecomboBox->setCurrentIndex(Properties::Instance()->backgroundMode);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Properties::Instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0 );

    /* set the delegate of shortcut widget as well as its contents */
    Delegate *del = new Delegate(shortcutsWidget);
    shortcutsWidget->setItemDelegate(del);
    shortcutsWidget->sortByColumn(0, Qt::AscendingOrder);
    setupShortcuts();

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << tr("No scrollbar") << tr("Left") << tr("Right");
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(Properties::Instance()->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << tr("Top") << tr("Bottom") << tr("Left") << tr("Right");
    tabsPos_comboBox->addItems(tabsPosList);
    tabsPos_comboBox->setCurrentIndex(Properties::Instance()->tabsPos);

    /* fixed tabs width */
    fixedTabWidthCheckBox->setChecked(Properties::Instance()->fixedTabWidth);
    fixedTabWidthSpinBox->setValue(Properties::Instance()->fixedTabWidthValue);
    /* tabs features */
    closeTabButtonCheckBox->setChecked(Properties::Instance()->showCloseTabButton);
    closeTabOnMiddleClickCheckBox->setChecked(Properties::Instance()->closeTabOnMiddleClick);

    /* keyboard cursor shape */
    QStringList keyboardCursorShapeList;
    keyboardCursorShapeList << tr("BlockCursor") << tr("UnderlineCursor") << tr("IBeamCursor");
    keybCursorShape_comboBox->addItems(keyboardCursorShapeList);
    keybCursorShape_comboBox->setCurrentIndex(Properties::Instance()->keyboardCursorShape);

    hideTabBarCheckBox->setChecked(Properties::Instance()->hideTabBarWithOneTab);

    // bold font face for intense colors
    boldIntenseCheckBox->setChecked(Properties::Instance()->boldIntense);

    // main menu bar
    menuAccelCheckBox->setChecked(Properties::Instance()->noMenubarAccel);
    showMenuCheckBox->setChecked(Properties::Instance()->menuVisible);

    borderlessCheckBox->setChecked(Properties::Instance()->borderless);

    /* actions by motion after paste */

    QStringList motionAfter;
    motionAfter << tr("No move") << tr("Scrolling to top") << tr("Scrolling to bottom");
    motionAfterPasting_comboBox->addItems(motionAfter);
    motionAfterPasting_comboBox->setCurrentIndex(Properties::Instance()->m_motionAfterPaste);

    disableBracketedPasteModeCheckBox->setChecked(Properties::Instance()->m_disableBracketedPasteMode);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(Properties::Instance()->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);

    setFontSample(Properties::Instance()->font);

    terminalMarginSpinBox->setValue(Properties::Instance()->terminalMargin);

    appTransparencyBox->setValue(Properties::Instance()->appTransparency);

    termTransparencyBox->setValue(Properties::Instance()->termTransparency);

    highlightCurrentCheckBox->setChecked(Properties::Instance()->highlightCurrentTerminal);

    showTerminalSizeHintCheckBox->setChecked(Properties::Instance()->showTerminalSizeHint);

    askOnExitCheckBox->setChecked(Properties::Instance()->askOnExit);

    savePosOnExitCheckBox->setChecked(Properties::Instance()->savePosOnExit);
    saveSizeOnExitCheckBox->setChecked(Properties::Instance()->saveSizeOnExit);
    fixedWithSpinBox->setValue(Properties::Instance()->fixedWindowSize.width());
    fixedHeightSpinBox->setValue(Properties::Instance()->fixedWindowSize.height());

    useCwdCheckBox->setChecked(Properties::Instance()->useCWD);
    openNewTabRightToActiveTabCheckBox->setChecked(Properties::Instance()->m_openNewTabRightToActiveTab);

    termComboBox->setCurrentText(Properties::Instance()->term);

    handleHistoryLineEdit->setText(Properties::Instance()->handleHistoryCommand);

    historyLimited->setChecked(Properties::Instance()->historyLimited);
    historyUnlimited->setChecked(!Properties::Instance()->historyLimited);
    historyLimitedTo->setValue(Properties::Instance()->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(Properties::Instance()->dropShowOnStart);

    dropHeightSpinBox->setValue(Properties::Instance()->dropHeight);
    dropHeightSpinBox->setMaximum(100);
    dropWidthSpinBox->setValue(Properties::Instance()->dropWidht);
    dropWidthSpinBox->setMaximum(100);

    dropShortCutEdit->setText(Properties::Instance()->dropShortCut.toString());

    useBookmarksCheckBox->setChecked(Properties::Instance()->useBookmarks);
    bookmarksLineEdit->setText(Properties::Instance()->bookmarksFile);
    openBookmarksFile(Properties::Instance()->bookmarksFile);
    connect(bookmarksButton, &QPushButton::clicked,
            this, &PropertiesDialog::bookmarksButton_clicked);

    terminalPresetComboBox->setCurrentIndex(Properties::Instance()->terminalsPreset);

    changeWindowTitleCheckBox->setChecked(Properties::Instance()->changeWindowTitle);
    changeWindowIconCheckBox->setChecked(Properties::Instance()->changeWindowIcon);
    enabledBidiSupportCheckBox->setChecked(Properties::Instance()->enabledBidiSupport);
    useFontBoxDrawingCharsCheckBox->setChecked(Properties::Instance()->useFontBoxDrawingChars);

    trimPastedTrailingNewlinesCheckBox->setChecked(Properties::Instance()->trimPastedTrailingNewlines);
    confirmMultilinePasteCheckBox->setChecked(Properties::Instance()->confirmMultilinePaste);

    // save the size on canceling too (it's saved on accepting by apply())
    connect(this, &QDialog::rejected, [this] {
        Properties::Instance()->prefDialogSize = size();
        Properties::Instance()->saveSettings();
    });

    // restore its size while fitting it into available desktop geometry
    QSize s;
    if (!Properties::Instance()->prefDialogSize.isEmpty())
        s = Properties::Instance()->prefDialogSize;
    else
        s = size(); // fall back to the ui size
    if (!ag.isEmpty())
        resize(s.boundedTo(ag));
    else // never happens
        resize(s);
}


PropertiesDialog::~PropertiesDialog()
{
}

void PropertiesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PropertiesDialog::apply()
{
    Properties::Instance()->colorScheme = colorSchemaCombo->currentText();
    Properties::Instance()->font = fontSampleLabel->font();//fontComboBox->currentFont();
    Properties::Instance()->guiStyle = (styleComboBox->currentText() == tr("System Default")) ?
                                       QString() : styleComboBox->currentText();

    Properties::Instance()->emulation = emulationComboBox->currentText();

    /* do not allow to go above 99 or we lose transparency option */
    (appTransparencyBox->value() >= 100) ?
            Properties::Instance()->appTransparency = 99
                :
            Properties::Instance()->appTransparency = appTransparencyBox->value();

    Properties::Instance()->terminalMargin = terminalMarginSpinBox->value();
    Properties::Instance()->termTransparency = termTransparencyBox->value();
    Properties::Instance()->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();
    Properties::Instance()->showTerminalSizeHint = showTerminalSizeHintCheckBox->isChecked();
    Properties::Instance()->backgroundImage = backgroundImageLineEdit->text();
    Properties::Instance()->backgroundMode = qBound(0, backgroundModecomboBox->currentIndex(), 4);

    Properties::Instance()->askOnExit = askOnExitCheckBox->isChecked();

    Properties::Instance()->savePosOnExit = savePosOnExitCheckBox->isChecked();
    Properties::Instance()->saveSizeOnExit = saveSizeOnExitCheckBox->isChecked();
    Properties::Instance()->fixedWindowSize = QSize(fixedWithSpinBox->value(), fixedHeightSpinBox->value()).expandedTo(QSize(300, 200)); // FIXME: make Properties variables private and use public methods for setting/getting them
    Properties::Instance()->prefDialogSize = size();

    Properties::Instance()->useCWD = useCwdCheckBox->isChecked();
    Properties::Instance()->m_openNewTabRightToActiveTab = openNewTabRightToActiveTabCheckBox->isChecked();

    Properties::Instance()->term = termComboBox->currentText();
    Properties::Instance()->handleHistoryCommand = handleHistoryLineEdit->text();

    Properties::Instance()->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    Properties::Instance()->tabsPos = tabsPos_comboBox->currentIndex();
    Properties::Instance()->fixedTabWidth = fixedTabWidthCheckBox->isChecked();
    Properties::Instance()->fixedTabWidthValue = fixedTabWidthSpinBox->value();
    Properties::Instance()->keyboardCursorShape = keybCursorShape_comboBox->currentIndex();
    Properties::Instance()->showCloseTabButton = closeTabButtonCheckBox->isChecked();
    Properties::Instance()->closeTabOnMiddleClick = closeTabOnMiddleClickCheckBox->isChecked();
    Properties::Instance()->hideTabBarWithOneTab = hideTabBarCheckBox->isChecked();
    Properties::Instance()->boldIntense = boldIntenseCheckBox->isChecked();
    Properties::Instance()->noMenubarAccel = menuAccelCheckBox->isChecked();
    Properties::Instance()->menuVisible = showMenuCheckBox->isChecked();
    Properties::Instance()->borderless = borderlessCheckBox->isChecked();
    Properties::Instance()->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();
    Properties::Instance()->m_disableBracketedPasteMode = disableBracketedPasteModeCheckBox->isChecked();

    Properties::Instance()->historyLimited = historyLimited->isChecked();
    Properties::Instance()->historyLimitedTo = historyLimitedTo->value();

    saveShortcuts();

    Properties::Instance()->saveSettings();

    Properties::Instance()->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    Properties::Instance()->dropHeight = dropHeightSpinBox->value();
    Properties::Instance()->dropWidht = dropWidthSpinBox->value();
    Properties::Instance()->dropShortCut = QKeySequence(dropShortCutEdit->text());

    Properties::Instance()->useBookmarks = useBookmarksCheckBox->isChecked();
    Properties::Instance()->bookmarksFile = bookmarksLineEdit->text();
    saveBookmarksFile(Properties::Instance()->bookmarksFile);

    Properties::Instance()->terminalsPreset = terminalPresetComboBox->currentIndex();

    Properties::Instance()->changeWindowTitle = changeWindowTitleCheckBox->isChecked();
    Properties::Instance()->changeWindowIcon = changeWindowIconCheckBox->isChecked();
    Properties::Instance()->enabledBidiSupport = enabledBidiSupportCheckBox->isChecked();
    Properties::Instance()->useFontBoxDrawingChars = useFontBoxDrawingCharsCheckBox->isChecked();

    Properties::Instance()->trimPastedTrailingNewlines = trimPastedTrailingNewlinesCheckBox->isChecked();
    Properties::Instance()->confirmMultilinePaste = confirmMultilinePasteCheckBox->isChecked();

    emit propertiesChanged();
}

void PropertiesDialog::setFontSample(const QFont & f)
{
    fontSampleLabel->setFont(f);
    QString sample = QString::fromLatin1("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}

void PropertiesDialog::changeFontButton_clicked()
{
    FontDialog dia(fontSampleLabel->font());
    if (!dia.exec())
        return;
    QFont f = dia.getFont();
    if (QFontInfo(f).fixedPitch())
        setFontSample(f);
}

void PropertiesDialog::chooseBackgroundImageButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
                            this, tr("Choose a background image"),
                            QString(), tr("Images (*.bmp *.jpg *.png *.svg *.xpm)"));
    if (!filename.isNull())
        backgroundImageLineEdit->setText(filename);
}

void PropertiesDialog::saveShortcuts()
{
    QMap<QString, QAction*> actions = QTerminalApp::Instance()->getWindowList()[0]->leaseActions();
    QList< QString > shortcutKeys = actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        const QString& keyValue = shortcutKeys.at(x);
        QAction *keyAction = actions[keyValue];

        QTableWidgetItem *item = nullptr;
        QString txt = keyAction->text();
        Properties::removeAccelerator(txt);
        auto items = shortcutsWidget->findItems(txt, Qt::MatchExactly);
        if (!items.isEmpty())
            item = shortcutsWidget->item(shortcutsWidget->row(items.at(0)), 1);
        if (item == nullptr)
            continue;

        QList<QKeySequence> shortcuts;
        const auto sequences = item->text().split(QLatin1Char('|'));
        for (const QString& sequenceString : sequences)
            shortcuts.append(QKeySequence(sequenceString, QKeySequence::NativeText));
        keyAction->setShortcuts(shortcuts);
    }
    Properties::Instance()->saveSettings();
}

void PropertiesDialog::setupShortcuts()
{
    shortcutsWidget->setSortingEnabled(false);

    QMap<QString, QAction*> actions = QTerminalApp::Instance()->getWindowList()[0]->leaseActions();
    QList< QString > shortcutKeys = actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        const QString& keyValue = shortcutKeys.at(x);
        QAction *keyAction = actions[keyValue];
        QStringList sequenceStrings;

        const auto shortcuts = keyAction->shortcuts();
        for (const QKeySequence &shortcut : shortcuts)
            sequenceStrings.append(shortcut.toString(QKeySequence::NativeText));

        QString txt = keyAction->text();
        Properties::removeAccelerator(txt);
        QTableWidgetItem *itemName = new QTableWidgetItem(txt);
        QTableWidgetItem *itemShortcut = new QTableWidgetItem( sequenceStrings.join(QLatin1Char('|')) );

        itemName->setFlags( itemName->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable );

        shortcutsWidget->setItem(x, 0, itemName);
        shortcutsWidget->setItem(x, 1, itemShortcut);
    }

    shortcutsWidget->resizeColumnsToContents();

    shortcutsWidget->setSortingEnabled(true);

    // No shortcut validation is needed with QKeySequenceEdit.
}

void PropertiesDialog::bookmarksButton_clicked()
{
    QFileDialog dia(this, tr("Open or create bookmarks file"));
    dia.setOption(QFileDialog::DontConfirmOverwrite, true);
    dia.setFileMode(QFileDialog::AnyFile);
    if (!dia.exec())
        return;

    QString fname = dia.selectedFiles().count() ? dia.selectedFiles().at(0) : QString();
    if (fname.isNull())
        return;

    bookmarksLineEdit->setText(fname);
    openBookmarksFile(bookmarksLineEdit->text());
}

void PropertiesDialog::openBookmarksFile(const QString &fname)
{
    QFile f(fname);
    QString content;
    if (!f.open(QFile::ReadOnly))
        content = QString::fromLatin1("<qterminal>\n  <group name=\"group1\">\n    <command name=\"cmd1\" value=\"cd $HOME\"/>\n  </group>\n</qterminal>");
    else
        content = QString::fromUtf8(f.readAll());

    bookmarkPlainEdit->setPlainText(content);
    bookmarkPlainEdit->document()->setModified(false);
}

void PropertiesDialog::saveBookmarksFile(const QString &fname)
{
    if (!bookmarkPlainEdit->document()->isModified())
        return;

    QFile f(fname);
    if (!f.open(QFile::WriteOnly|QFile::Truncate))
        qDebug() << "Cannot write to file" << f.fileName();
    else
        f.write(bookmarkPlainEdit->toPlainText().toUtf8());
}

/*
void PropertiesDialog::setupShortcuts()
{
    QList< QString > shortcutKeys = Properties::Instance()->shortcuts.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);

        QLabel *lblShortcut = new QLabel( keyValue, this );
        QPushButton *btnLaunch = new QPushButton( Properties::Instance()->shortcuts.value( keyValue ), this );

        btnLaunch->setObjectName(keyValue);
        connect( btnLaunch, SIGNAL(clicked()), this, SLOT(shortcutPrompt()) );

        shortcutsWidget->setCellWidget( x, 0, lblShortcut );
        shortcutsWidget->setCellWidget( x, 1, btnLaunch );
    }
}

void PropertiesDialog::shortcutPrompt()
{
    QObject *objectSender = sender();

    if( !objectSender )
        return;

    QString name = objectSender->objectName();
    qDebug() << "shortcutPrompt(" << name << ")";

    DialogShortcut *dlgShortcut = new DialogShortcut(this);
    dlgShortcut->setTitle( tr("Select a key sequence for %1").arg(name) );

    QString sequenceString = Properties::Instance()->shortcuts[name];
    dlgShortcut->setKey(sequenceString);

    int result = dlgShortcut->exec();
    if( result == QDialog::Accepted )
    {
        sequenceString = dlgShortcut->getKey();
        Properties::Instance()->shortcuts[name] = sequenceString;
        Properties::Instance()->saveSettings();
    }
}
*/
