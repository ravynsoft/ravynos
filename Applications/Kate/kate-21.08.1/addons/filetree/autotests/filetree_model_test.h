/* This file is part of the KDE project
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef FILETREE_MODEL_TEST_H
#define FILETREE_MODEL_TEST_H

#include <QObject>

class KateFileTreeModel;
class ResultNode;

class FileTreeModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void basic();

    void buildTree_data();
    void buildTree();
    void buildTreeFullPath_data();
    void buildTreeFullPath();
    void buildTreeBatch_data();
    void buildTreeBatch();
    void buildTreeBatchPrefill_data();
    void buildTreeBatchPrefill();

    void listMode_data();
    void listMode();

    void deleteDocument_data();
    void deleteDocument();
    void deleteDocumentBatch_data();
    void deleteDocumentBatch();

    void rename_data();
    void rename();

private:
    void walkTree(KateFileTreeModel &model, const QModelIndex &i, ResultNode &node);
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
