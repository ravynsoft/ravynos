/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2018 Gregor Mi <codestruct@posteo.org>
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "tstestapp.h"
#include "../tabswitcherfilesmodel.h"

#include <QAbstractTableModel>
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QStandardItem>
#include <QStringRef>
#include <QTreeView>
#include <QVBoxLayout>

#include <KTextEditor/Document>
#include <KTextEditor/Editor>

static KTextEditor::Document *addDoc(const QString &path)
{
    auto doc = KTextEditor::Editor::instance()->createDocument(nullptr);
    doc->openUrl(QUrl::fromLocalFile(path));
    return doc;
}

class TsTestApp::Impl
{
public:
    void insert_1_item()
    {
        model.insertDocument(0, addDoc(QStringLiteral("/home/user2/folder1/abc.d")));
        treeview1->resizeColumnToContents(0);
    }

    void remove_1_item()
    {
        model.removeRow(0);
        treeview1->resizeColumnToContents(0);
    }

    void set_items_cutoff_bug()
    {
        model.clear();

        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/logs/notifications/multimedia-system.log")));
        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/dev/src/kservicemenueditor-0.2a/servicemenueditor")));
        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/kde/src/kdesrc-build/kdesrc-build")));
        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/node_modules/autolinker/README.md")));
        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/node_modules/autolinker/package.json")));
        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/node_modules/autolinker/LICENSE")));
        model.insertDocument(model.rowCount(), addDoc(QStringLiteral("/home/gregor/node_modules/asynckit/package.json")));

        treeview1->resizeColumnToContents(0);
    }

public:
    detail::TabswitcherFilesModel model;
    QTreeView *treeview1 = nullptr;
};

TsTestApp::TsTestApp(QWidget *parent)
    : QMainWindow(parent)
    , impl_(new TsTestApp::Impl)
{
    setGeometry(0, 0, 1024, 800);
    setCentralWidget(new QWidget(this));
    auto l = new QVBoxLayout();
    centralWidget()->setLayout(l);

    auto hl = new QHBoxLayout();
    l->addLayout(hl);

    auto buttonInsert1 = new QPushButton(QStringLiteral("Ins 1 item"), this);
    connect(buttonInsert1, &QPushButton::clicked, this, [=] {
        impl_->insert_1_item();
    });
    hl->addWidget(buttonInsert1);

    auto buttonRemove1 = new QPushButton(QStringLiteral("Del 1 item"), this);
    connect(buttonRemove1, &QPushButton::clicked, this, [=] {
        impl_->remove_1_item();
    });
    hl->addWidget(buttonRemove1);

    auto buttonSetTestSet1 = new QPushButton(QStringLiteral("set_items_cutoff_bug"), this);
    connect(buttonSetTestSet1, &QPushButton::clicked, this, [=] {
        impl_->set_items_cutoff_bug();
    });
    hl->addWidget(buttonSetTestSet1);

    impl_->treeview1 = new QTreeView(this);
    l->addWidget(impl_->treeview1);
    impl_->treeview1->setHeaderHidden(true);
    impl_->treeview1->setRootIsDecorated(false);

    impl_->model.insertDocument(impl_->model.rowCount(), addDoc(QStringLiteral("/home/gm/projects/proj1/src/file1.h")));
    impl_->model.insertDocument(impl_->model.rowCount(), addDoc(QStringLiteral("/home/gm/projects/proj1/src/file2.cpp")));
    impl_->model.insertDocument(impl_->model.rowCount(), addDoc(QStringLiteral("/home/gm/dev/file3.py")));
    impl_->model.insertDocument(impl_->model.rowCount(), addDoc(QStringLiteral("/home/gm/dev/file3kjaskdfkljasdfklj089asdfkjklasdjf90asdfsdfkj.py")));
    impl_->model.insertDocument(impl_->model.rowCount(), addDoc(QStringLiteral("/home/gm/dev/proj2/asldfkjasdfk/asdlfkjasd;faf/;ajsdkfgjaskdfgasdf/file3.py")));
    // impl_->insert_a_item();
    // impl_->remove_a_item();

    impl_->model.rowCount();
    impl_->model.item(0);
    impl_->model.index(0, 0);

    impl_->treeview1->setModel(&impl_->model);
    impl_->treeview1->resizeColumnToContents(0);
    impl_->treeview1->resizeColumnToContents(1);

    auto listview1 = new QListView(this);
    l->addWidget(listview1);
    listview1->setModel(&impl_->model);

    auto treeview2 = new QTreeView(this);
    l->addWidget(treeview2);
}

TsTestApp::~TsTestApp()
{
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TsTestApp w;
    w.show();

    return app.exec();
}
