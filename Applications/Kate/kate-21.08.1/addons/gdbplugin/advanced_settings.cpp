// Description: Advanced settings dialog for gdb
//
//
// SPDX-FileCopyrightText: 2012 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "advanced_settings.h"

#ifdef WIN32
static const QLatin1Char pathSeparator(';');
#else
static const QLatin1Char pathSeparator(':');
#endif

#include <QFileDialog>

AdvancedGDBSettings::AdvancedGDBSettings(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    u_gdbBrowse->setIcon(QIcon::fromTheme(QStringLiteral("application-x-ms-dos-executable")));
    connect(u_gdbBrowse, &QToolButton::clicked, this, &AdvancedGDBSettings::slotBrowseGDB);

    u_setSoPrefix->setIcon(QIcon::fromTheme(QStringLiteral("folder")));
    connect(u_setSoPrefix, &QToolButton::clicked, this, &AdvancedGDBSettings::slotSetSoPrefix);

    u_addSoSearchPath->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    u_delSoSearchPath->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(u_addSoSearchPath, &QToolButton::clicked, this, &AdvancedGDBSettings::slotAddSoPath);
    connect(u_delSoSearchPath, &QToolButton::clicked, this, &AdvancedGDBSettings::slotDelSoPath);

    u_addSrcPath->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    u_delSrcPath->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(u_addSrcPath, &QToolButton::clicked, this, &AdvancedGDBSettings::slotAddSrcPath);
    connect(u_delSrcPath, &QToolButton::clicked, this, &AdvancedGDBSettings::slotDelSrcPath);

    connect(u_buttonBox, &QDialogButtonBox::accepted, this, &AdvancedGDBSettings::accept);
    connect(u_buttonBox, &QDialogButtonBox::rejected, this, &AdvancedGDBSettings::reject);

    connect(u_localRemote, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &AdvancedGDBSettings::slotLocalRemoteChanged);
}

AdvancedGDBSettings::~AdvancedGDBSettings()
{
}

const QStringList AdvancedGDBSettings::configs() const
{
    QStringList tmp;

    tmp << u_gdbCmd->text();
    switch (u_localRemote->currentIndex()) {
    case 1:
        tmp << QStringLiteral("target remote %1:%2").arg(u_tcpHost->text(), u_tcpPort->text());
        tmp << QString();
        break;
    case 2:
        tmp << QStringLiteral("target remote %1").arg(u_ttyPort->text());
        tmp << QStringLiteral("set remotebaud %1").arg(u_baudCombo->currentText());
        break;
    default:
        tmp << QString();
        tmp << QString();
    }
    if (!u_soAbsPrefix->text().isEmpty()) {
        tmp << QStringLiteral("set solib-absolute-prefix %1").arg(u_soAbsPrefix->text());
    } else {
        tmp << QString();
    }

    if (u_soSearchPaths->count() > 0) {
        QString paths = QStringLiteral("set solib-search-path ");
        for (int i = 0; i < u_soSearchPaths->count(); ++i) {
            if (i != 0) {
                paths += pathSeparator;
            }
            paths += u_soSearchPaths->item(i)->text();
        }
        tmp << paths;
    } else {
        tmp << QString();
    }

    if (u_srcPaths->count() > 0) {
        QString paths = QStringLiteral("set directories ");
        for (int i = 0; i < u_srcPaths->count(); ++i) {
            if (i != 0) {
                paths += pathSeparator;
            }
            paths += u_srcPaths->item(i)->text();
        }
        tmp << paths;
    } else {
        tmp << QString();
    }
    tmp << u_customInit->toPlainText().split(QLatin1Char('\n'));

    return tmp;
}

void AdvancedGDBSettings::setConfigs(const QStringList &cfgs)
{
    // clear all info
    u_gdbCmd->setText(QStringLiteral("gdb"));
    u_localRemote->setCurrentIndex(0);
    u_soAbsPrefix->clear();
    u_soSearchPaths->clear();
    u_srcPaths->clear();
    u_customInit->clear();
    u_tcpHost->setText(QString());
    u_tcpPort->setText(QString());
    u_ttyPort->setText(QString());
    u_baudCombo->setCurrentIndex(0);

    // GDB
    if (cfgs.count() <= GDBIndex) {
        return;
    }
    u_gdbCmd->setText(cfgs[GDBIndex]);

    // Local / Remote
    if (cfgs.count() <= LocalRemoteIndex) {
        return;
    }

    int start;
    int end;
    if (cfgs[LocalRemoteIndex].isEmpty()) {
        u_localRemote->setCurrentIndex(0);
        u_remoteStack->setCurrentIndex(0);
    } else if (cfgs[LocalRemoteIndex].contains(pathSeparator)) {
        u_localRemote->setCurrentIndex(1);
        u_remoteStack->setCurrentIndex(1);
        start = cfgs[LocalRemoteIndex].lastIndexOf(QLatin1Char(' '));
        end = cfgs[LocalRemoteIndex].indexOf(pathSeparator);
        u_tcpHost->setText(cfgs[LocalRemoteIndex].mid(start + 1, end - start - 1));
        u_tcpPort->setText(cfgs[LocalRemoteIndex].mid(end + 1));
    } else {
        u_localRemote->setCurrentIndex(2);
        u_remoteStack->setCurrentIndex(2);
        start = cfgs[LocalRemoteIndex].lastIndexOf(QLatin1Char(' '));
        u_ttyPort->setText(cfgs[LocalRemoteIndex].mid(start + 1));

        start = cfgs[RemoteBaudIndex].lastIndexOf(QLatin1Char(' '));
        setComboText(u_baudCombo, cfgs[RemoteBaudIndex].mid(start + 1));
    }

    // Solib absolute path
    if (cfgs.count() <= SoAbsoluteIndex) {
        return;
    }
    start = 26; // "set solib-absolute-prefix "
    u_soAbsPrefix->setText(cfgs[SoAbsoluteIndex].mid(start));

    // Solib search path
    if (cfgs.count() <= SoRelativeIndex) {
        return;
    }
    start = 22; // "set solib-search-path "
    QString tmp = cfgs[SoRelativeIndex].mid(start);
    u_soSearchPaths->addItems(tmp.split(pathSeparator));

    if (cfgs.count() <= SrcPathsIndex) {
        return;
    }
    start = 16; // "set directories "
    tmp = cfgs[SrcPathsIndex].mid(start);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    u_srcPaths->addItems(tmp.split(pathSeparator, QString::SkipEmptyParts));
#else
    u_srcPaths->addItems(tmp.split(pathSeparator, Qt::SkipEmptyParts));
#endif

    // Custom init
    for (int i = CustomStartIndex; i < cfgs.count(); i++) {
        u_customInit->appendPlainText(cfgs[i]);
    }

    slotLocalRemoteChanged();
}

void AdvancedGDBSettings::slotBrowseGDB()
{
    u_gdbCmd->setText(QFileDialog::getOpenFileName(this, QString(), u_gdbCmd->text(), QStringLiteral("application/x-executable")));
    if (u_gdbCmd->text().isEmpty()) {
        u_gdbCmd->setText(QStringLiteral("gdb"));
    }
}

void AdvancedGDBSettings::setComboText(QComboBox *combo, const QString &str)
{
    if (!combo) {
        return;
    }

    for (int i = 0; i < combo->count(); i++) {
        if (combo->itemText(i) == str) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    // The string was not found -> add it
    combo->addItem(str);
    combo->setCurrentIndex(combo->count() - 1);
}

void AdvancedGDBSettings::slotSetSoPrefix()
{
    QString prefix = QFileDialog::getExistingDirectory(this);
    if (prefix.isEmpty()) {
        return;
    }

    u_soAbsPrefix->setText(prefix);
}

void AdvancedGDBSettings::slotAddSoPath()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path.isEmpty()) {
        return;
    }

    u_soSearchPaths->addItem(path);
}

void AdvancedGDBSettings::slotDelSoPath()
{
    QListWidgetItem *item = u_soSearchPaths->takeItem(u_soSearchPaths->currentRow());
    delete item;
}

void AdvancedGDBSettings::slotAddSrcPath()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path.isEmpty()) {
        return;
    }

    u_srcPaths->addItem(path);
}

void AdvancedGDBSettings::slotDelSrcPath()
{
    QListWidgetItem *item = u_srcPaths->takeItem(u_srcPaths->currentRow());
    delete item;
}

void AdvancedGDBSettings::slotLocalRemoteChanged()
{
    u_soAbsPrefixLabel->setEnabled(u_localRemote->currentIndex() != 0);
    u_soSearchLabel->setEnabled(u_localRemote->currentIndex() != 0);
    u_soAbsPrefix->setEnabled(u_localRemote->currentIndex() != 0);
    u_soSearchPaths->setEnabled(u_localRemote->currentIndex() != 0);
    u_setSoPrefix->setEnabled(u_localRemote->currentIndex() != 0);
    u_addDelSoPaths->setEnabled(u_localRemote->currentIndex() != 0);
}
