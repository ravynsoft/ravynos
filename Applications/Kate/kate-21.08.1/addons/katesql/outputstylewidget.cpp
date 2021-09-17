/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "outputstylewidget.h"

#include <KSharedConfig>
#include <QBrush>
#include <QCheckBox>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QVariant>

#include <KColorButton>
#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

OutputStyleWidget::OutputStyleWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(7);
    setRootIsDecorated(false);

    QStringList headerLabels;

    headerLabels << i18nc("@title:column", "Context") << QString() << QString() << QString() << QString() << i18nc("@title:column", "Text Color")
                 << i18nc("@title:column", "Background Color");

    setHeaderLabels(headerLabels);

    headerItem()->setIcon(1, QIcon::fromTheme(QStringLiteral("format-text-bold")));
    headerItem()->setIcon(2, QIcon::fromTheme(QStringLiteral("format-text-italic")));
    headerItem()->setIcon(3, QIcon::fromTheme(QStringLiteral("format-text-underline")));
    headerItem()->setIcon(4, QIcon::fromTheme(QStringLiteral("format-text-strikethrough")));

    addContext(QStringLiteral("text"), i18nc("@item:intable", "Text"));
    addContext(QStringLiteral("number"), i18nc("@item:intable", "Number"));
    addContext(QStringLiteral("bool"), i18nc("@item:intable", "Bool"));
    addContext(QStringLiteral("datetime"), i18nc("@item:intable", "Date & Time"));
    addContext(QStringLiteral("null"), i18nc("@item:intable", "NULL"));
    addContext(QStringLiteral("blob"), i18nc("@item:intable", "BLOB"));

    for (int i = 0; i < columnCount(); ++i) {
        resizeColumnToContents(i);
    }

    updatePreviews();
}

OutputStyleWidget::~OutputStyleWidget()
{
}

QTreeWidgetItem *OutputStyleWidget::addContext(const QString &key, const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(this);

    item->setText(0, name);
    item->setData(0, Qt::UserRole, key);

    QCheckBox *boldCheckBox = new QCheckBox(this);
    QCheckBox *italicCheckBox = new QCheckBox(this);
    QCheckBox *underlineCheckBox = new QCheckBox(this);
    QCheckBox *strikeOutCheckBox = new QCheckBox(this);
    KColorButton *foregroundColorButton = new KColorButton(this);
    KColorButton *backgroundColorButton = new KColorButton(this);

    const KColorScheme scheme(QPalette::Active, KColorScheme::View);

    foregroundColorButton->setDefaultColor(scheme.foreground().color());
    backgroundColorButton->setDefaultColor(scheme.background().color());

    setItemWidget(item, 1, boldCheckBox);
    setItemWidget(item, 2, italicCheckBox);
    setItemWidget(item, 3, underlineCheckBox);
    setItemWidget(item, 4, strikeOutCheckBox);
    setItemWidget(item, 5, foregroundColorButton);
    setItemWidget(item, 6, backgroundColorButton);

    readConfig(item);

    connect(boldCheckBox, &QCheckBox::toggled, this, &OutputStyleWidget::slotChanged);
    connect(italicCheckBox, &QCheckBox::toggled, this, &OutputStyleWidget::slotChanged);
    connect(underlineCheckBox, &QCheckBox::toggled, this, &OutputStyleWidget::slotChanged);
    connect(strikeOutCheckBox, &QCheckBox::toggled, this, &OutputStyleWidget::slotChanged);
    connect(foregroundColorButton, &KColorButton::changed, this, &OutputStyleWidget::slotChanged);
    connect(backgroundColorButton, &KColorButton::changed, this, &OutputStyleWidget::slotChanged);

    return item;
}

void OutputStyleWidget::readConfig(QTreeWidgetItem *item)
{
    KConfigGroup config(KSharedConfig::openConfig(), "KateSQLPlugin");
    KConfigGroup g = config.group("OutputCustomization").group(item->data(0, Qt::UserRole).toString());

    QCheckBox *boldCheckBox = static_cast<QCheckBox *>(itemWidget(item, 1));
    QCheckBox *italicCheckBox = static_cast<QCheckBox *>(itemWidget(item, 2));
    QCheckBox *underlineCheckBox = static_cast<QCheckBox *>(itemWidget(item, 3));
    QCheckBox *strikeOutCheckBox = static_cast<QCheckBox *>(itemWidget(item, 4));
    KColorButton *foregroundColorButton = static_cast<KColorButton *>(itemWidget(item, 5));
    KColorButton *backgroundColorButton = static_cast<KColorButton *>(itemWidget(item, 6));

    const QFont font = g.readEntry("font", QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    boldCheckBox->setChecked(font.bold());
    italicCheckBox->setChecked(font.italic());
    underlineCheckBox->setChecked(font.underline());
    strikeOutCheckBox->setChecked(font.strikeOut());

    foregroundColorButton->setColor(g.readEntry("foregroundColor", foregroundColorButton->defaultColor()));
    backgroundColorButton->setColor(g.readEntry("backgroundColor", backgroundColorButton->defaultColor()));
}

void OutputStyleWidget::writeConfig(QTreeWidgetItem *item)
{
    KConfigGroup config(KSharedConfig::openConfig(), "KateSQLPlugin");

    KConfigGroup g = config.group("OutputCustomization").group(item->data(0, Qt::UserRole).toString());

    QCheckBox *boldCheckBox = static_cast<QCheckBox *>(itemWidget(item, 1));
    QCheckBox *italicCheckBox = static_cast<QCheckBox *>(itemWidget(item, 2));
    QCheckBox *underlineCheckBox = static_cast<QCheckBox *>(itemWidget(item, 3));
    QCheckBox *strikeOutCheckBox = static_cast<QCheckBox *>(itemWidget(item, 4));
    KColorButton *foregroundColorButton = static_cast<KColorButton *>(itemWidget(item, 5));
    KColorButton *backgroundColorButton = static_cast<KColorButton *>(itemWidget(item, 6));

    QFont f(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    f.setBold(boldCheckBox->isChecked());
    f.setItalic(italicCheckBox->isChecked());
    f.setUnderline(underlineCheckBox->isChecked());
    f.setStrikeOut(strikeOutCheckBox->isChecked());

    g.writeEntry("font", f);
    g.writeEntry("foregroundColor", foregroundColorButton->color());
    g.writeEntry("backgroundColor", backgroundColorButton->color());
}

void OutputStyleWidget::readConfig()
{
    QTreeWidgetItem *root = invisibleRootItem();

    for (int i = 0; i < root->childCount(); ++i) {
        readConfig(root->child(i));
    }
}

void OutputStyleWidget::writeConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(), "KateSQLPlugin");
    config.deleteGroup("OutputCustomization");

    QTreeWidgetItem *root = invisibleRootItem();

    for (int i = 0; i < root->childCount(); ++i) {
        writeConfig(root->child(i));
    }
}

void OutputStyleWidget::updatePreviews()
{
    QTreeWidgetItem *root = invisibleRootItem();

    for (int i = 0; i < root->childCount(); ++i) {
        QTreeWidgetItem *item = root->child(i);

        const QCheckBox *boldCheckBox = static_cast<QCheckBox *>(itemWidget(item, 1));
        const QCheckBox *italicCheckBox = static_cast<QCheckBox *>(itemWidget(item, 2));
        const QCheckBox *underlineCheckBox = static_cast<QCheckBox *>(itemWidget(item, 3));
        const QCheckBox *strikeOutCheckBox = static_cast<QCheckBox *>(itemWidget(item, 4));
        const KColorButton *foregroundColorButton = static_cast<KColorButton *>(itemWidget(item, 5));
        const KColorButton *backgroundColorButton = static_cast<KColorButton *>(itemWidget(item, 6));

        QFont f(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

        f.setBold(boldCheckBox->isChecked());
        f.setItalic(italicCheckBox->isChecked());
        f.setUnderline(underlineCheckBox->isChecked());
        f.setStrikeOut(strikeOutCheckBox->isChecked());

        item->setFont(0, f);
        item->setForeground(0, foregroundColorButton->color());
        item->setBackground(0, backgroundColorButton->color());
    }
}

void OutputStyleWidget::slotChanged()
{
    updatePreviews();

    Q_EMIT changed();
}
