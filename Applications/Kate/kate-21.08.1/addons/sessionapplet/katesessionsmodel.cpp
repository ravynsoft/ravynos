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

#include "katesessionsmodel.h"
#include <KDirWatch>
#include <KLocalizedString>
#include <KStringHandler>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QIcon>
#include <QListWidgetItem>
#include <QStandardPaths>
#include <QStyleOptionGraphicsItem>
#include <QTreeView>
#include <QVBoxLayout>

bool katesessions_compare_sessions(const QString &s1, const QString &s2)
{
    return QString::localeAwareCompare(s1, s2) == -1;
}

KateSessionsModel::KateSessionsModel(QObject *parent)
    : QStandardItemModel(parent) /*, m_config(0)*/
{
    KDirWatch *dirwatch = new KDirWatch(this);
    m_sessionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kate/sessions");

    dirwatch->addDir(m_sessionsDir);

    connect(dirwatch, &KDirWatch::dirty, this, &KateSessionsModel::slotUpdateSessionMenu);
    slotUpdateSessionMenu();
}

KateSessionsModel::~KateSessionsModel()
{
}

void KateSessionsModel::slotUpdateSessionMenu()
{
    clear();
    m_sessions.clear();
    m_fullList.clear();
    initSessionFiles();
}

void KateSessionsModel::initSessionFiles()
{
    QStandardItem *item = new QStandardItem();
    item->setData(i18n("Start Kate (no arguments)"), Qt::DisplayRole);
    item->setData(QIcon::fromTheme(QStringLiteral("kate")), Qt::DecorationRole);
    item->setData(QStringLiteral("_kate_noargs"), Uuid);
    item->setData(0, TypeRole);
    m_fullList << item->data(Qt::DisplayRole).toString();
    appendRow(item);

    item = new QStandardItem();
    item->setData(i18n("New Kate Session"), Qt::DisplayRole);
    item->setData(QIcon::fromTheme(QStringLiteral("document-new")), Qt::DecorationRole);
    qDebug() << QIcon::fromTheme(QStringLiteral("document-new"));
    item->setData(QStringLiteral("_kate_newsession"), Uuid);
    item->setData(1, TypeRole);
    m_fullList << item->data(Qt::DisplayRole).toString();
    appendRow(item);

    QDir dir(m_sessionsDir, QStringLiteral("*.katesession"));

    for (unsigned int i = 0; i < dir.count(); ++i) {
        QString name = dir[i];
        name.chop(12); // .katesession
        m_sessions << QUrl::fromPercentEncoding(name.toLatin1());
    }

    std::sort(m_sessions.begin(), m_sessions.end(), katesessions_compare_sessions);
    QLatin1String ext(".katesession");
    for (QStringList::ConstIterator it = m_sessions.constBegin(); it != m_sessions.constEnd(); ++it) {
        m_fullList << *it;
        item = new QStandardItem();
        item->setData(*it, Qt::DisplayRole);
        item->setData(QString((*it) + ext), Uuid);
        item->setData(QIcon::fromTheme(QStringLiteral("document-open")), Qt::DecorationRole);
        item->setData(2, TypeRole);
        appendRow(item);
    }
}

QHash<int, QByteArray> KateSessionsModel::roleNames() const
{
    QHash<int, QByteArray> hash;
    hash.insert(Qt::DisplayRole, QByteArrayLiteral("DisplayRole"));
    hash.insert(Qt::DecorationRole, QByteArrayLiteral("DecorationRole"));
    hash.insert(Qt::UserRole + 3, QByteArrayLiteral("UuidRole"));
    hash.insert(Qt::UserRole + 4, QByteArrayLiteral("TypeRole"));
    return hash;
}
