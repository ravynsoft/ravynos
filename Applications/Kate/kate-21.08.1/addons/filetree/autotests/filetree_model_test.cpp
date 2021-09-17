/* This file is part of the KDE project
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "filetree_model_test.h"

#include "katefiletreemodel.h"

#include "document_dummy.h"

#include <QTest>
QTEST_MAIN(FileTreeModelTest)

// BEGIN ResultNode
class ResultNode
{
public:
    ResultNode() = default; // root node
    ResultNode(const ResultNode &other)
        : name(other.name)
        , dir(other.dir)
        , children(other.children)
    {
    }
    ResultNode(const char *_name, const bool _dir = false)
        : ResultNode(QString::fromLatin1(_name), _dir)
    {
    }
    ResultNode(const QString &_name, const bool _dir = false)
        : name(_name)
        , dir(_dir)
    {
    }

    ResultNode &operator<<(const ResultNode &node)
    {
        children << node;
        return *this;
    }

    bool operator!=(const ResultNode &other) const
    {
        return !(*this == other);
    }
    bool operator==(const ResultNode &other) const
    {
        return (other.name == name) && (other.dir == dir) && (other.children == children);
    }

    friend QDebug operator<<(QDebug s, const ResultNode &node)
    {
        s << node.toString();
        return s;
    }

    friend void debugOutput(QString &s, const ResultNode &rootNode, const int level = 0)
    {
        for (int i = 0; i < level; i++) {
            s += QLatin1String("  ");
        }

        const QString name = rootNode.name.isEmpty() ? QStringLiteral("ROOT") : rootNode.name;

        s += QLatin1String("( ") + name;
        if (rootNode.dir) {
            s += QLatin1String(", {D}");
        }

        if (rootNode.children.isEmpty()) {
            s += QLatin1String(" )");
        } else {
            s += QLatin1String(",\n");

            for (int i = 0; i < rootNode.children.size(); i++) {
                const ResultNode &node = rootNode.children[i];

                debugOutput(s, node, level + 1);

                if ((i + 1) < rootNode.children.size()) {
                    s += QLatin1Char('\n');
                }
            }
            s += (level == 0) ? QLatin1String("\n);") : QLatin1String(")");
        }
    }

    QString toString() const
    {
        QString out;
        debugOutput(out, *this, 0);
        return out;
    }

    QString name;
    bool dir = true;
    QList<ResultNode> children;
};

Q_DECLARE_METATYPE(ResultNode)

namespace QTest
{
inline bool qCompare(const ResultNode &t1, const ResultNode &t2, const char *actual, const char *expected, const char *file, int line)
{
    /* compare_helper is not helping that much, we need to prepare copy of data */
    const QByteArray a = t1.toString().toLatin1();
    const QByteArray b = t2.toString().toLatin1();

    char *val1 = new char[a.size() + 1];
    char *val2 = new char[b.size() + 1];

    memcpy(val1, a.constData(), a.size() + 1);
    memcpy(val2, b.constData(), b.size() + 1);

    return compare_helper(t1 == t2, "Compared ResultNode trees are not the same", val1, val2, actual, expected, file, line);
}
}
// END ResultNode

void FileTreeModelTest::initTestCase()
{
}

void FileTreeModelTest::cleanupTestCase()
{
}

void FileTreeModelTest::init()
{
}

void FileTreeModelTest::cleanup()
{
}

void FileTreeModelTest::basic()
{
    QScopedPointer<DummyDocument> d1(new DummyDocument());
    QScopedPointer<DummyDocument> d2(new DummyDocument());

    KateFileTreeModel m(this);
    QCOMPARE(m.rowCount(QModelIndex()), 0);

    m.documentOpened(d1.data());
    QCOMPARE(m.rowCount(QModelIndex()), 1);

    m.documentOpened(d2.data());
    QCOMPARE(m.rowCount(QModelIndex()), 2);
}

void FileTreeModelTest::buildTree_data()
{
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("easy") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt"))
                          << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")));

    QTest::newRow("two") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///a/bar.txt"))
                         << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt") << ResultNode("bar.txt")));

    QTest::newRow("strangers") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///b/bar.txt"))
                               << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")) << (ResultNode("b", true) << ResultNode("bar.txt")));

    QTest::newRow("lvl1 strangers") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt"))
                                    << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")) << (ResultNode("b", true) << ResultNode("bar.txt")));

    QTest::newRow("multiples") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                            << new DummyDocument("file:///c/a/bar.txt"))
                               << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt") << ResultNode("bar.txt"))
                                                << (ResultNode("b", true) << ResultNode("bar.txt")));

    QTest::newRow("stairs") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/bar.txt"))
                            << (ResultNode() << (ResultNode("c", true) << (ResultNode("a", true) << ResultNode("foo.txt")) << ResultNode("bar.txt")));

    QTest::newRow("reverse stairs") << (QList<DummyDocument *>() << new DummyDocument("file:///c/bar.txt") << new DummyDocument("file:///c/a/foo.txt"))
                                    << (ResultNode() << (ResultNode("c", true) << ResultNode("bar.txt") << (ResultNode("a", true) << ResultNode("foo.txt"))));

    QTest::newRow("matching") << (QList<DummyDocument *>() << new DummyDocument("file:///a/x/foo.txt") << new DummyDocument("file:///b/x/bar.txt"))
                              << (ResultNode() << (ResultNode("a", true) << (ResultNode("x", true) << ResultNode("foo.txt")))
                                               << (ResultNode("b", true) << (ResultNode("x", true) << ResultNode("bar.txt"))));

    QTest::newRow("matching even more")
        << (QList<DummyDocument *>() << new DummyDocument("file:///a/x/y/z/foo.txt") << new DummyDocument("file:///b/x/y/z/bar.txt"))
        << (ResultNode() << (ResultNode("a", true) << (ResultNode("x", true) << (ResultNode("y", true) << (ResultNode("z", true) << ResultNode("foo.txt")))))
                         << (ResultNode("b", true) << (ResultNode("x", true) << (ResultNode("y", true) << (ResultNode("z", true) << ResultNode("bar.txt"))))));

    QTest::newRow("matching with booby trap") << (QList<DummyDocument *>()
                                                  << new DummyDocument("file:///x/y/foo.txt") << new DummyDocument("file:///c/x/bar.txt")
                                                  << new DummyDocument("file:///d/y/baz.txt"))
                                              << (ResultNode() << (ResultNode("x", true) << (ResultNode("y", true) << ResultNode("foo.txt")))
                                                               << (ResultNode("d", true) << (ResultNode("y", true) << ResultNode("baz.txt")))
                                                               << (ResultNode("c", true) << (ResultNode("x", true) << ResultNode("bar.txt"))));

    QTest::newRow("branches") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                           << new DummyDocument("file:///d/a/foo.txt"))
                              << (ResultNode() << (ResultNode("c", true)
                                                   << (ResultNode("a", true) << ResultNode("foo.txt")) << (ResultNode("b", true) << ResultNode("bar.txt")))
                                               << (ResultNode("d", true) << (ResultNode("a", true) << ResultNode("foo.txt"))));

    QTest::newRow("branches (more)") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                                  << new DummyDocument("file:///c/c/bar.txt") << new DummyDocument("file:///d/a/foo.txt"))
                                     << (ResultNode() << (ResultNode("c", true) << (ResultNode("a", true) << ResultNode("foo.txt"))
                                                                                << (ResultNode("b", true) << ResultNode("bar.txt"))
                                                                                << (ResultNode("c", true) << ResultNode("bar.txt")))
                                                      << (ResultNode("d", true) << (ResultNode("a", true) << ResultNode("foo.txt"))));

    QTest::newRow("bug347578") << (QList<DummyDocument *>()
                                   << new DummyDocument("file:///f/g/a/b/c/d/e.txt") << new DummyDocument("file:///f/g/a/t/b/c/d/e.txt"))
                               << (ResultNode() << (ResultNode("a", true)
                                                    << (ResultNode("b", true) << (ResultNode("c", true) << (ResultNode("d", true) << ResultNode("e.txt"))))
                                                    << (ResultNode("t", true)
                                                        << (ResultNode("b", true)
                                                            << (ResultNode("c", true) << (ResultNode("d", true) << ResultNode("e.txt")))))));

    QTest::newRow("levels") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                         << new DummyDocument("file:///d/foo.txt"))
                            << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")) << (ResultNode("b", true) << ResultNode("bar.txt"))
                                             << (ResultNode("d", true) << ResultNode("foo.txt")));

    QTest::newRow("remote simple") << (QList<DummyDocument *>() << new DummyDocument("http://example.org/foo.txt"))
                                   << (ResultNode() << (ResultNode("[example.org]", true) << ResultNode("foo.txt")));

    QTest::newRow("remote nested") << (QList<DummyDocument *>() << new DummyDocument("http://example.org/a/foo.txt"))
                                   << (ResultNode() << (ResultNode("[example.org]a", true) << ResultNode("foo.txt")));

    /* NOTE: this one is also not completely ok, is it?
     * on other hand, it would get confusing or overly leveled if opening
     * something like http://example.org/a/b/c/d/e/f/g.txt
     */
    QTest::newRow("remote diverge") << (QList<DummyDocument *>()
                                        << new DummyDocument("http://example.org/a/foo.txt") << new DummyDocument("http://example.org/b/foo.txt"))
                                    << (ResultNode() << (ResultNode("[example.org]a", true) << ResultNode("foo.txt"))
                                                     << (ResultNode("[example.org]b", true) << ResultNode("foo.txt")));
}

void FileTreeModelTest::buildTree()
{
    KateFileTreeModel m(this);
    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : documents) {
        m.documentOpened(doc);
    }

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

void FileTreeModelTest::buildTreeBatch_data()
{
    // the easiest way to verify the equality of those two calls:)
    buildTree_data();
}

void FileTreeModelTest::buildTreeBatch()
{
    KateFileTreeModel m(this);
    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(ResultNode, nodes);

    QList<KTextEditor::Document *> list;

    for (DummyDocument *doc : documents) {
        list << doc;
    }

    m.documentsOpened(list);

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

void FileTreeModelTest::buildTreeBatchPrefill_data()
{
    QTest::addColumn<QList<DummyDocument *>>("prefill");
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("easy") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt"))
                          << (QList<DummyDocument *>() << new DummyDocument("file:///a/bar.txt"))
                          << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt") << ResultNode("bar.txt")));

    QTest::newRow("split") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt"))
                           << (QList<DummyDocument *>() << new DummyDocument("file:///b/foo.txt"))
                           << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")) << (ResultNode("b", true) << ResultNode("foo.txt")));
}

void FileTreeModelTest::buildTreeBatchPrefill()
{
    KateFileTreeModel m(this);
    QFETCH(const QList<DummyDocument *>, prefill);
    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : prefill) {
        m.documentOpened(doc);
    }

    QList<KTextEditor::Document *> list;

    for (DummyDocument *doc : documents) {
        list << doc;
    }

    m.documentsOpened(list);

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(prefill);
    qDeleteAll(documents);
}

void FileTreeModelTest::walkTree(KateFileTreeModel &model, const QModelIndex &rootIndex, ResultNode &rootNode)
{
    if (!model.hasChildren(rootIndex)) {
        return;
    }

    const int rows = model.rowCount(rootIndex);
    for (int i = 0; i < rows; i++) {
        const QModelIndex idx = model.index(i, 0, rootIndex);
        ResultNode node(model.data(idx).toString(), model.isDir(idx));
        walkTree(model, idx, node);
        rootNode << node;
    }
}

void FileTreeModelTest::buildTreeFullPath_data()
{
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("two") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///a/bar.txt"))
                         << (ResultNode() << (ResultNode("/a", true) << ResultNode("foo.txt") << ResultNode("bar.txt")));

    QTest::newRow("multiples") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                            << new DummyDocument("file:///c/a/bar.txt"))
                               << (ResultNode() << (ResultNode("/c/a", true) << ResultNode("foo.txt") << ResultNode("bar.txt"))
                                                << (ResultNode("/c/b", true) << ResultNode("bar.txt")));

    /* This one and the case after can get a little bit tricky and
     * in some situation could end up in little bit confusing layout.
     * current root merge algorithm sees the divergent paths, so it
     * doesn't invoke merging.
    QTest::newRow("branches") << ( QList<DummyDocument *>()
      << new DummyDocument("file:///c/a/foo.txt")
      << new DummyDocument("file:///c/b/bar.txt")
    ) << (
      ResultNode()
        << (ResultNode("/c", true)
          << (ResultNode("a", true)
            << ResultNode("foo.txt"))
          << (ResultNode("b", true)
            << ResultNode("bar.txt")))
    );
    */

    /*
    QTest::newRow("levels") << ( QList<DummyDocument *>()
      << new DummyDocument("file:///c/a/foo.txt")
      << new DummyDocument("file:///c/b/bar.txt")
      << new DummyDocument("file:///d/foo.txt")
    ) << (
      ResultNode()
        << (ResultNode("/c", true)
          << (ResultNode("a", true)
            << ResultNode("foo.txt"))
          << (ResultNode("b", true)
            << ResultNode("bar.txt")))
        << (ResultNode("/d", true)
          << ResultNode("foo.txt"))
    );
    */

    QTest::newRow("remote simple") << (QList<DummyDocument *>() << new DummyDocument("http://example.org/foo.txt"))
                                   << (ResultNode() << (ResultNode("[example.org]", true) << ResultNode("foo.txt")));

    QTest::newRow("remote nested") << (QList<DummyDocument *>() << new DummyDocument("http://example.org/a/b/foo.txt"))
                                   << (ResultNode() << (ResultNode("[example.org]/a/b", true) << ResultNode("foo.txt")));

    /* NOTE: see the similar testcase in buildTree */
    QTest::newRow("remote diverge") << (QList<DummyDocument *>()
                                        << new DummyDocument("http://example.org/c/a/foo.txt") << new DummyDocument("http://example.org/c/b/foo.txt"))
                                    << (ResultNode() << (ResultNode("[example.org]/c/a", true) << ResultNode("foo.txt"))
                                                     << (ResultNode("[example.org]/c/b", true) << ResultNode("foo.txt")));
}

void FileTreeModelTest::buildTreeFullPath()
{
    KateFileTreeModel m(this);
    m.setShowFullPathOnRoots(true);

    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : documents) {
        m.documentOpened(doc);
    }

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

void FileTreeModelTest::listMode_data()
{
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("easy") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt")) << (ResultNode() << ResultNode("foo.txt"));

    QTest::newRow("two") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///a/bar.txt"))
                         << (ResultNode() << ResultNode("foo.txt") << ResultNode("bar.txt"));

    QTest::newRow("multiples") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                            << new DummyDocument("file:///c/a/bar.txt"))
                               << (ResultNode() << ResultNode("foo.txt") << ResultNode("bar.txt") << ResultNode("bar.txt"));

    QTest::newRow("remote diverge") << (QList<DummyDocument *>()
                                        << new DummyDocument("http://example.org/a/foo.txt") << new DummyDocument("http://example.org/b/foo.txt"))
                                    << (ResultNode() << ResultNode("[example.org]foo.txt") << ResultNode("[example.org]foo.txt"));
}

void FileTreeModelTest::listMode()
{
    KateFileTreeModel m(this);
    m.setListMode(true);

    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : documents) {
        m.documentOpened(doc);
    }

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

void FileTreeModelTest::deleteDocument_data()
{
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<QList<int>>("remove");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("empty") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt")) << (QList<int>() << 0) << (ResultNode());

    QTest::newRow("two") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///a/bar.txt"))
                         << (QList<int>() << 0) << (ResultNode() << (ResultNode("a", true) << ResultNode("bar.txt")));

    QTest::newRow("multiple") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo0.txt") << new DummyDocument("file:///a/foo1.txt")
                                                           << new DummyDocument("file:///a/foo2.txt") << new DummyDocument("file:///a/foo3.txt")
                                                           << new DummyDocument("file:///a/foo4.txt") << new DummyDocument("file:///a/foo5.txt")
                                                           << new DummyDocument("file:///a/foo6.txt") << new DummyDocument("file:///a/foo7.txt"))
                              << (QList<int>() << 1 << 2 << 4 << 6)
                              << (ResultNode() << (ResultNode("a", true)
                                                   << ResultNode("foo0.txt") << ResultNode("foo3.txt") << ResultNode("foo5.txt") << ResultNode("foo7.txt")));

    QTest::newRow("strangers") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///b/bar.txt"))
                               << (QList<int>() << 1) << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")));

    QTest::newRow("branches") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                           << new DummyDocument("file:///d/a/foo.txt"))
                              << (QList<int>() << 1)
                              << (ResultNode() << (ResultNode("c", true) << (ResultNode("a", true) << ResultNode("foo.txt")))
                                               << (ResultNode("d", true) << (ResultNode("a", true) << ResultNode("foo.txt"))));

    QTest::newRow("levels") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                         << new DummyDocument("file:///d/foo.txt"))
                            << (QList<int>() << 0)
                            << (ResultNode() << (ResultNode("b", true) << ResultNode("bar.txt")) << (ResultNode("d", true) << ResultNode("foo.txt")));

    QTest::newRow("levels extra") << (QList<DummyDocument *>() << new DummyDocument("file:///c/a/foo.txt") << new DummyDocument("file:///c/b/bar.txt")
                                                               << new DummyDocument("file:///d/foo.txt"))
                                  << (QList<int>() << 2)
                                  << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")) << (ResultNode("b", true) << ResultNode("bar.txt")));

    QTest::newRow("remote diverge") << (QList<DummyDocument *>()
                                        << new DummyDocument("http://example.org/a/foo.txt") << new DummyDocument("http://example.org/b/foo.txt"))
                                    << (QList<int>() << 1) << (ResultNode() << (ResultNode("[example.org]a", true) << ResultNode("foo.txt")));
}

void FileTreeModelTest::deleteDocument()
{
    KateFileTreeModel m(this);
    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(const QList<int>, remove);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : documents) {
        m.documentOpened(doc);
    }

    for (const int &index : remove) {
        m.documentClosed(documents[index]);
    }

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

void FileTreeModelTest::deleteDocumentBatch_data()
{
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<QList<int>>("remove");
    QTest::addColumn<QList<int>>("fail");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("neo") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo0.txt") << new DummyDocument("file:///a/foo1.txt")
                                                      << new DummyDocument("file:///a/foo2.txt") << new DummyDocument("file:///a/foo3.txt")
                                                      << new DummyDocument("file:///a/foo4.txt") << new DummyDocument("file:///a/foo5.txt")
                                                      << new DummyDocument("file:///a/foo6.txt") << new DummyDocument("file:///a/foo7.txt"))
                         << (QList<int>() << 1 << 2 << 4 << 6) << (QList<int>() << 2 << 4)
                         << (ResultNode() << (ResultNode("a", true) << ResultNode("foo0.txt") << ResultNode("foo2.txt") << ResultNode("foo3.txt")
                                                                    << ResultNode("foo4.txt") << ResultNode("foo5.txt") << ResultNode("foo7.txt")));
}

void FileTreeModelTest::deleteDocumentBatch()
{
    KateFileTreeModel m(this);
    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(const QList<int>, remove);
    QFETCH(const QList<int>, fail);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : documents) {
        m.documentOpened(doc);
    }

    for (const int &index : remove) {
        if (!fail.contains(index)) {
            m.documentClosed(documents[index]);
        }
    }

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

void FileTreeModelTest::rename_data()
{
    QTest::addColumn<QList<DummyDocument *>>("documents");
    QTest::addColumn<int>("rename_idx");
    QTest::addColumn<QString>("rename_url");
    QTest::addColumn<ResultNode>("nodes");

    QTest::newRow("empty") << (QList<DummyDocument *>() << new DummyDocument()) << 0 << QStringLiteral("file:///a/foo.txt")
                           << (ResultNode() << (ResultNode("a", true) << ResultNode("foo.txt")));

    QTest::newRow("moving") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt")) << 0 << QStringLiteral("file:///b/foo.txt")
                            << (ResultNode() << (ResultNode("b", true) << ResultNode("foo.txt")));

    QTest::newRow("splitting") << (QList<DummyDocument *>() << new DummyDocument("file:///a/foo.txt") << new DummyDocument("file:///a/bar.txt")) << 0
                               << QStringLiteral("file:///b/foo.txt")
                               << (ResultNode() << (ResultNode("a", true) << ResultNode("bar.txt")) << (ResultNode("b", true) << ResultNode("foo.txt")));
}

void FileTreeModelTest::rename()
{
    KateFileTreeModel m(this);
    QFETCH(const QList<DummyDocument *>, documents);
    QFETCH(int, rename_idx);
    QFETCH(QString, rename_url);
    QFETCH(ResultNode, nodes);

    for (DummyDocument *doc : documents) {
        m.documentOpened(doc);
    }

    documents[rename_idx]->setUrl(rename_url);
    m.documentNameChanged(documents[rename_idx]);

    ResultNode root;
    walkTree(m, QModelIndex(), root);

    QCOMPARE(root, nodes);
    qDeleteAll(documents);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
