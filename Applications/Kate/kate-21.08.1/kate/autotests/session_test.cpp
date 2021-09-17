/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "session_test.h"
#include "katesession.h"

#include <KConfig>
#include <KConfigGroup>

#include <QFileInfo>
#include <QTemporaryFile>
#include <QtTestWidgets>

QTEST_MAIN(KateSessionTest)

void KateSessionTest::initTestCase()
{
}

void KateSessionTest::cleanupTestCase()
{
}

void KateSessionTest::init()
{
    m_tmpfile = new QTemporaryFile;
    QVERIFY(m_tmpfile->open());
}

void KateSessionTest::cleanup()
{
    delete m_tmpfile;
}

void KateSessionTest::create()
{
    const QString name = QStringLiteral("session name");
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), name);
    QCOMPARE(s->name(), name);
    QCOMPARE((int)s->documents(), 0);
    QCOMPARE(s->isAnonymous(), false);
    QCOMPARE(s->config()->name(), m_tmpfile->fileName());
}

void KateSessionTest::createAnonymous()
{
    KateSession::Ptr s = KateSession::createAnonymous(m_tmpfile->fileName());
    QCOMPARE(s->name(), QString());
    QCOMPARE((int)s->documents(), 0);
    QCOMPARE(s->isAnonymous(), true);
    QCOMPARE(s->config()->name(), m_tmpfile->fileName());
}

void KateSessionTest::createAnonymousFrom()
{
    // Regular
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), QStringLiteral("session name"));

    const QString groupName = QStringLiteral("test group");
    QTemporaryFile newFile;
    newFile.open();
    KateSession::Ptr ns;

    s->config()->group(groupName).writeEntry("foo", "bar");

    // Create Anon from Other
    ns = KateSession::createAnonymousFrom(s, newFile.fileName());
    QCOMPARE(ns->name(), QString());
    QCOMPARE((int)ns->documents(), 0);
    QCOMPARE(ns->isAnonymous(), true);
    QCOMPARE(ns->config()->name(), newFile.fileName());
    QCOMPARE(ns->config()->group(groupName).readEntry("foo"), QLatin1String("bar"));
}

void KateSessionTest::createFrom()
{
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), QStringLiteral("session name"));

    const QString newName = QStringLiteral("new session name");
    const QString groupName = QStringLiteral("test group");

    QTemporaryFile newFile;
    newFile.open();
    KateSession::Ptr ns;

    s->config()->group(groupName).writeEntry("foo", "bar");

    ns = KateSession::createFrom(s, newFile.fileName(), newName);
    QCOMPARE(ns->name(), newName);
    QCOMPARE((int)ns->documents(), 0);
    QCOMPARE(ns->isAnonymous(), false);
    QCOMPARE(ns->config()->name(), newFile.fileName());
    QCOMPARE(ns->config()->group(groupName).readEntry("foo"), QLatin1String("bar"));
}

void KateSessionTest::documents()
{
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), QStringLiteral("session name"));

    s->setDocuments(42);
    QCOMPARE((int)s->documents(), 42);

    s->config()->sync();
    KConfig c(m_tmpfile->fileName());
    QCOMPARE(c.group("Open Documents").readEntry<int>("Count", 0), 42);
}

void KateSessionTest::setFile()
{
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), QStringLiteral("session name"));
    s->config()->group("test").writeEntry("foo", "bar");

    QTemporaryFile file2;
    file2.open();

    s->setFile(file2.fileName());
    QCOMPARE(s->config()->name(), file2.fileName());
    QCOMPARE(s->config()->group("test").readEntry("foo"), QLatin1String("bar"));
}

void KateSessionTest::timestamp()
{
    QFileInfo i(m_tmpfile->fileName());
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), QStringLiteral("session name"));

    QCOMPARE(s->timestamp(), i.lastModified());
}

void KateSessionTest::setName()
{
    KateSession::Ptr s = KateSession::create(m_tmpfile->fileName(), QStringLiteral("session name"));
    const QString newName = QStringLiteral("bar");
    s->setName(newName);
    QCOMPARE(s->name(), newName);
    QCOMPARE(s->file(), m_tmpfile->fileName()); // on purpose, orthogonal
}
