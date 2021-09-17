/**
 * \file
 *
 * \brief Class \c kate::CloseConfirmDialog (interface)
 *
 * SPDX-FileCopyrightText: 2012 Alex Turbov <i.zaufi@gmail.com>
 *
 * \date Sun Jun 24 16:29:13 MSK 2012 -- Initial design
 */
/*
 * KateCloseExceptPlugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KateCloseExceptPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SRC__CLOSE_CONFIRM_DIALOG_H__
#define __SRC__CLOSE_CONFIRM_DIALOG_H__

// Project specific includes

// Standard includes
#include "ui_close_confirm_dialog.h"
#include <KTextEditor/Document>
#include <KToggleAction>
#include <KWindowConfig>
#include <QCheckBox>
#include <QDialog>
#include <QList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace kate
{
/**
 * \brief [Type brief class description here]
 *
 * [More detailed description here]
 *
 */
class CloseConfirmDialog : public QDialog, public Ui::CloseConfirmDialog
{
    Q_OBJECT
public:
    /// Default constructor
    explicit CloseConfirmDialog(QList<KTextEditor::Document *> &, KToggleAction *, QWidget *const = nullptr);
    ~CloseConfirmDialog() override;

private Q_SLOTS:
    void updateDocsList();

private:
    QList<KTextEditor::Document *> &m_docs;
};

} // namespace kate
#endif // __SRC__CLOSE_CONFIRM_DIALOG_H__
