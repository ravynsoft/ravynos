/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katesession.h"

#include "katedebug.h"
#include "katesessionmanager.h"

#include <KConfig>
#include <KConfigGroup>

#include <QCollator>
#include <QFile>
#include <QFileInfo>

static const QLatin1String opGroupName("Open Documents");
static const QLatin1String keyCount("Count");

KateSession::KateSession(const QString &file, const QString &name, const bool anonymous, const KConfig *_config)
    : m_name(name)
    , m_file(file)
    , m_anonymous(anonymous)
    , m_documents(0)
{
    Q_ASSERT(!m_file.isEmpty());

    if (_config) { // copy data from config instead
        m_config.reset(_config->copyTo(m_file));
    } else if (!QFile::exists(m_file)) { // given file exists, use it to load some stuff
        qCDebug(LOG_KATE) << "Warning, session file not found: " << m_file;
        return;
    }

    m_timestamp = QFileInfo(m_file).lastModified();

    // get the document count
    m_documents = config()->group(opGroupName).readEntry(keyCount, 0);
}

const QString &KateSession::file() const
{
    return m_file;
}

void KateSession::setDocuments(const unsigned int number)
{
    config()->group(opGroupName).writeEntry(keyCount, number);
    m_documents = number;
}

void KateSession::setFile(const QString &filename)
{
    if (m_config) {
        KConfig *cfg = m_config->copyTo(filename);
        m_config.reset(cfg);
    }

    m_file = filename;
}

void KateSession::setName(const QString &name)
{
    m_name = name;
}

KConfig *KateSession::config()
{
    if (!m_config) {
        // reread documents number?
        m_config = std::make_unique<KConfig>(m_file, KConfig::SimpleConfig);
    }

    return m_config.get();
}

KateSession::Ptr KateSession::create(const QString &file, const QString &name)
{
    return Ptr(new KateSession(file, name, false));
}

KateSession::Ptr KateSession::createFrom(const KateSession::Ptr &session, const QString &file, const QString &name)
{
    return Ptr(new KateSession(file, name, false, session->config()));
}

KateSession::Ptr KateSession::createAnonymous(const QString &file)
{
    return Ptr(new KateSession(file, QString(), true));
}

KateSession::Ptr KateSession::createAnonymousFrom(const KateSession::Ptr &session, const QString &file)
{
    return Ptr(new KateSession(file, QString(), true, session->config()));
}

bool KateSession::compareByName(const KateSession::Ptr &s1, const KateSession::Ptr &s2)
{
    return QCollator().compare(s1->name(), s2->name()) == -1;
}

bool KateSession::compareByTimeDesc(const KateSession::Ptr &s1, const KateSession::Ptr &s2)
{
    return s1->timestamp() > s2->timestamp();
}
