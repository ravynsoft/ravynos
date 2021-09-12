/***************************************************************************
 *   Copyright (C) 2014 by Petr Vanek                                      *
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

#include "fontdialog.h"

FontDialog::FontDialog(const QFont &f)
    : QDialog(nullptr)
{
    setupUi(this);

    fontComboBox->setFontFilters(QFontComboBox::MonospacedFonts
                                 | QFontComboBox::NonScalableFonts
                                 | QFontComboBox::ScalableFonts);

    fontComboBox->setCurrentFont(f);
    fontComboBox->setEditable(false);

    sizeSpinBox->setValue(f.pointSize());

    setFontSample(f);

    connect(fontComboBox, &QFontComboBox::currentFontChanged,
            this, &FontDialog::setFontSample);
    connect(sizeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &FontDialog::setFontSize);
}

QFont FontDialog::getFont()
{
    QFont f = fontComboBox->currentFont();
    f.setPointSize(sizeSpinBox->value());
    return f;
}

void FontDialog::setFontSample(const QFont &f)
{
    previewLabel->setFont(f);
    QString sample(QLatin1String("%1 %2 pt"));
    previewLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}

void FontDialog::setFontSize()
{
    const QFont &f = getFont();
    previewLabel->setFont(f);
    QString sample(QLatin1String("%1 %2 pt"));
    previewLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}
