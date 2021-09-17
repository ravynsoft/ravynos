/***************************************************************************
 *   SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>                  *
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef _KATESESSIONSMODEL_H_
#define _KATESESSIONSMODEL_H_

/*
#include <plasma/popupapplet.h>

#include "ui_katesessionConfig.h"
*/

#include <QStandardItemModel>

class QModelIndex;
class KConfigDialog;
class QStringList;

class KateSessionsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    KateSessionsModel(QObject *parent);
    ~KateSessionsModel() override;
    QHash<int, QByteArray> roleNames() const override;
    enum SpecificRoles { Uuid = Qt::UserRole + 3, TypeRole = Qt::UserRole + 4 };

protected Q_SLOTS:
    //    void slotOnItemClicked(const QModelIndex &index);
    void slotUpdateSessionMenu();
    //    void slotSaveConfig();

protected:
    void initSessionFiles();
    /*    void createConfigurationInterface(KConfigDialog *parent);
        void configChanged();*/
private:
    QStringList m_sessions;
    QStringList m_fullList;
    QString m_sessionsDir;
    //    KateSessionConfigInterface *m_config;
};
#endif
