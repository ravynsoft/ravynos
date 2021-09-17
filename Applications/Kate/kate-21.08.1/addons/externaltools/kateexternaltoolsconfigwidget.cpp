/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kateexternaltoolsconfigwidget.h"
#include "externaltoolsplugin.h"
#include "kateexternaltool.h"
#include "katetoolrunner.h"

#include <KLineEdit>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <KConfig>
#include <KConfigGroup>
#include <KIconButton>
#include <KIconLoader>
#include <KMimeTypeChooser>
#include <KSharedConfig>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>
#include <QBitmap>
#include <QComboBox>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStandardItem>
#include <QStandardPaths>
#include <QTreeView>

namespace
{
constexpr int ToolRole = Qt::UserRole + 1;

/**
 * Helper function to create a QStandardItem that internally stores a pointer to a KateExternalTool.
 */
QStandardItem *newToolItem(const QIcon &icon, KateExternalTool *tool)
{
    auto item = new QStandardItem(icon, tool->translatedName());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    item->setData(QVariant::fromValue(reinterpret_cast<quintptr>(tool)), ToolRole);
    return item;
}

/**
 * Helper function to return an internally stored KateExternalTool for a QStandardItem.
 * If a nullptr is returned, it means the QStandardItem is a category.
 */
KateExternalTool *toolForItem(QStandardItem *item)
{
    return item ? reinterpret_cast<KateExternalTool *>(item->data(ToolRole).value<quintptr>()) : nullptr;
}

QIcon blankIcon()
{
    QPixmap pm(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    pm.fill();
    pm.setMask(pm.createHeuristicMask());
    return QIcon(pm);
}

//! Helper that ensures that tool->actionName is unique
static void makeActionNameUnique(KateExternalTool *tool, const std::vector<KateExternalTool *> &tools)
{
    QString name = tool->actionName;
    int i = 1;
    while (true) {
        auto it = std::find_if(tools.cbegin(), tools.cend(), [tool, &name](const KateExternalTool *t) {
            return (t != tool) && (t->actionName == name);
        });
        if (it == tools.cend()) {
            break;
        }
        name = tool->actionName + QString::number(i);
        ++i;
    }
    tool->actionName = name;
}

/**
 * Helper that ensures that the tool->cmdname is unique
 */
void makeEditorCommandUnique(KateExternalTool *tool, const std::vector<KateExternalTool *> &tools)
{
    // empty command line name is OK
    if (tool->cmdname.isEmpty()) {
        return;
    }

    QString cmdname = tool->cmdname;
    int i = 1;
    while (true) {
        auto it = std::find_if(tools.cbegin(), tools.cend(), [tool, &cmdname](const KateExternalTool *t) {
            return (t != tool) && (t->cmdname == cmdname);
        });
        if (it == tools.cend()) {
            break;
        }
        cmdname = tool->cmdname + QString::number(i);
        ++i;
    }
    tool->cmdname = cmdname;
}

static KateExternalTool defaultTool(const QString &actionName, const QVector<KateExternalTool> &defaultTools)
{
    auto it = std::find_if(defaultTools.cbegin(), defaultTools.cend(), [actionName](const KateExternalTool &defaultTool) {
        return actionName == defaultTool.actionName;
    });
    return (it != defaultTools.cend()) ? *it : KateExternalTool();
}

static bool isDefaultTool(KateExternalTool *tool, const QVector<KateExternalTool> &defaultTools)
{
    return tool && !defaultTool(tool->actionName, defaultTools).actionName.isEmpty();
}
}

// BEGIN KateExternalToolServiceEditor
KateExternalToolServiceEditor::KateExternalToolServiceEditor(KateExternalTool *tool, KateExternalToolsPlugin *plugin, QWidget *parent)
    : QDialog(parent)
    , m_plugin(plugin)
    , m_tool(tool)
{
    setWindowTitle(i18n("Edit External Tool"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("system-run")));

    ui.setupUi(this);
    ui.btnIcon->setIconSize(KIconLoader::SizeSmall);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &KateExternalToolServiceEditor::slotOKClicked);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui.btnMimeType, &QToolButton::clicked, this, &KateExternalToolServiceEditor::showMTDlg);

    Q_ASSERT(m_tool != nullptr);
    ui.edtName->setText(m_tool->translatedName());
    if (!m_tool->icon.isEmpty()) {
        ui.btnIcon->setIcon(m_tool->icon);
    }

    ui.edtExecutable->setText(m_tool->executable);
    ui.edtArgs->setText(m_tool->arguments);
    ui.edtInput->setText(m_tool->input);
    ui.edtWorkingDir->setText(m_tool->workingDir);
    ui.edtMimeType->setText(m_tool->mimetypes.join(QStringLiteral("; ")));
    ui.cmbSave->setCurrentIndex(static_cast<int>(m_tool->saveMode));
    ui.chkReload->setChecked(m_tool->reload);
    ui.cmbOutput->setCurrentIndex(static_cast<int>(m_tool->outputMode));
    ui.edtCommand->setText(m_tool->cmdname);

    static const QRegularExpressionValidator cmdLineValidator(QRegularExpression(QStringLiteral("[\\w-]*")));
    ui.edtCommand->setValidator(&cmdLineValidator);

    if (isDefaultTool(tool, m_plugin->defaultTools())) {
        ui.buttonBox->setStandardButtons(ui.buttonBox->standardButtons() | QDialogButtonBox::RestoreDefaults);
        ui.buttonBox->setToolTip(i18n("Revert tool to default settings"));
        connect(ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [this, tool]() {
            const auto t = defaultTool(tool->actionName, m_plugin->defaultTools());
            ui.edtName->setText(t.translatedName());
            ui.btnIcon->setIcon(t.icon);
            ui.edtExecutable->setText(t.executable);
            ui.edtArgs->setText(t.arguments);
            ui.edtInput->setText(t.input);
            ui.edtWorkingDir->setText(t.workingDir);
            ui.edtMimeType->setText(t.mimetypes.join(QStringLiteral("; ")));
            ui.cmbSave->setCurrentIndex(static_cast<int>(t.saveMode));
            ui.chkReload->setChecked(t.reload);
            ui.cmbOutput->setCurrentIndex(static_cast<int>(t.outputMode));
            ui.edtCommand->setText(t.cmdname);
        });
    }

    // add support for variable expansion
    KTextEditor::Editor::instance()->addVariableExpansion({ui.edtExecutable->lineEdit(), ui.edtArgs, ui.edtInput, ui.edtWorkingDir->lineEdit()});
}

void KateExternalToolServiceEditor::slotOKClicked()
{
    if (ui.edtName->text().isEmpty() || ui.edtExecutable->text().isEmpty()) {
        QMessageBox::information(this, i18n("External Tool"), i18n("You must specify at least a name and an executable"));
        return;
    }

    accept();
}

void KateExternalToolServiceEditor::showMTDlg()
{
    QString text = i18n("Select the MimeTypes for which to enable this tool.");
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList list = ui.edtMimeType->text().split(QRegularExpression(QStringLiteral("\\s*;\\s*")), QString::SkipEmptyParts);
#else
    QStringList list = ui.edtMimeType->text().split(QRegularExpression(QStringLiteral("\\s*;\\s*")), Qt::SkipEmptyParts);
#endif
    KMimeTypeChooserDialog d(i18n("Select Mime Types"), text, list, QStringLiteral("text"), this);
    if (d.exec() == QDialog::Accepted) {
        ui.edtMimeType->setText(d.chooser()->mimeTypes().join(QStringLiteral(";")));
    }
}
// END KateExternalToolServiceEditor

static std::vector<QStandardItem *> childItems(const QStandardItem *item)
{
    // collect all KateExternalTool items
    std::vector<QStandardItem *> children;
    for (int i = 0; i < item->rowCount(); ++i) {
        children.push_back(item->child(i));
    }
    return children;
}

static std::vector<KateExternalTool *> collectTools(const QStandardItemModel &model)
{
    std::vector<KateExternalTool *> tools;
    for (auto categoryItem : childItems(model.invisibleRootItem())) {
        for (auto child : childItems(categoryItem)) {
            auto tool = toolForItem(child);
            Q_ASSERT(tool != nullptr);
            tools.push_back(tool);
        }
    }
    return tools;
}

// BEGIN KateExternalToolsConfigWidget
KateExternalToolsConfigWidget::KateExternalToolsConfigWidget(QWidget *parent, KateExternalToolsPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    lbTools->setModel(&m_toolsModel);
    lbTools->setSelectionMode(QAbstractItemView::SingleSelection);
    lbTools->setDragEnabled(true);
    lbTools->setAcceptDrops(true);
    lbTools->setDefaultDropAction(Qt::MoveAction);
    lbTools->setDropIndicatorShown(true);
    lbTools->setDragDropOverwriteMode(false);
    lbTools->setDragDropMode(QAbstractItemView::InternalMove);

    // Add... button popup menu
    auto addMenu = new QMenu(btnAdd);
    auto addToolAction = addMenu->addAction(i18n("Add Tool..."));
    auto addDefaultsMenu = addMenu->addMenu(i18n("Add Tool from Defaults"));
    addMenu->addSeparator();
    auto addCategoryAction = addMenu->addAction(i18n("Add Category"));
    btnAdd->setMenu(addMenu);
    connect(addDefaultsMenu, &QMenu::aboutToShow, [this, addDefaultsMenu]() {
        lazyInitDefaultsMenu(addDefaultsMenu);
    });

    connect(addCategoryAction, &QAction::triggered, this, &KateExternalToolsConfigWidget::slotAddCategory);
    connect(addToolAction, &QAction::triggered, this, &KateExternalToolsConfigWidget::slotAddTool);
    connect(btnRemove, &QPushButton::clicked, this, &KateExternalToolsConfigWidget::slotRemove);
    connect(btnEdit, &QPushButton::clicked, this, &KateExternalToolsConfigWidget::slotEdit);
    connect(lbTools->selectionModel(), &QItemSelectionModel::currentChanged, [this]() {
        slotSelectionChanged();
    });
    connect(lbTools, &QTreeView::doubleClicked, this, &KateExternalToolsConfigWidget::slotEdit);

    m_config = new KConfig(QStringLiteral("externaltools"), KConfig::NoGlobals, QStandardPaths::ApplicationsLocation);

    // reset triggers a reload of the existing tools
    reset();
    slotSelectionChanged();

    connect(&m_toolsModel, &QStandardItemModel::itemChanged, [this]() {
        m_changed = true;
        Q_EMIT changed();
    });
}

KateExternalToolsConfigWidget::~KateExternalToolsConfigWidget()
{
    clearTools();

    delete m_config;
}

QString KateExternalToolsConfigWidget::name() const
{
    return i18n("External Tools");
}

QString KateExternalToolsConfigWidget::fullName() const
{
    return i18n("External Tools");
}

QIcon KateExternalToolsConfigWidget::icon() const
{
    return QIcon::fromTheme(QStringLiteral("system-run"));
}

void KateExternalToolsConfigWidget::reset()
{
    clearTools();
    m_toolsModel.invisibleRootItem()->setFlags(Qt::NoItemFlags);

    // the "Uncategorized" category always exists
    m_noCategory = addCategory(i18n("Uncategorized"));
    m_noCategory->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);

    // create other tools and categories
    const auto tools = m_plugin->tools();
    for (auto tool : tools) {
        auto clone = new KateExternalTool(*tool);
        auto item = newToolItem(clone->icon.isEmpty() ? blankIcon() : QIcon::fromTheme(clone->icon), clone);
        auto category = clone->category.isEmpty() ? m_noCategory : addCategory(clone->category);
        category->appendRow(item);
    }
    lbTools->expandAll();
    m_changed = false;
}

void KateExternalToolsConfigWidget::apply()
{
    if (!m_changed) {
        return;
    }
    m_changed = false;

    // collect all KateExternalTool items
    std::vector<KateExternalTool *> tools;
    for (auto categoryItem : childItems(m_toolsModel.invisibleRootItem())) {
        const QString category = (categoryItem == m_noCategory) ? QString() : categoryItem->text();
        for (auto child : childItems(categoryItem)) {
            auto tool = toolForItem(child);
            Q_ASSERT(tool != nullptr);
            // at this point, we have to overwrite the category, since it may have changed (and we never tracked this)
            tool->category = category;
            tools.push_back(tool);
        }
    }

    // write tool configuration to disk
    m_config->group("Global").writeEntry("firststart", false);
    m_config->group("Global").writeEntry("tools", static_cast<int>(tools.size()));
    for (size_t i = 0; i < tools.size(); i++) {
        const QString section = QStringLiteral("Tool ") + QString::number(i);
        KConfigGroup cg(m_config, section);
        tools[i]->save(cg);
    }

    m_config->sync();
    m_plugin->reload();
}

void KateExternalToolsConfigWidget::slotSelectionChanged()
{
    // update button state
    auto item = m_toolsModel.itemFromIndex(lbTools->currentIndex());
    const bool isToolItem = toolForItem(item) != nullptr;
    const bool isCategory = item && !isToolItem;

    btnEdit->setEnabled(isToolItem || isCategory);
    btnRemove->setEnabled(isToolItem);
}

bool KateExternalToolsConfigWidget::editTool(KateExternalTool *tool)
{
    bool changed = false;

    KateExternalToolServiceEditor editor(tool, m_plugin, this);
    editor.resize(m_config->group("Editor").readEntry("Size", QSize()));
    if (editor.exec() == QDialog::Accepted) {
        tool->name = editor.ui.edtName->text().trimmed();
        tool->icon = editor.ui.btnIcon->icon();
        tool->executable = editor.ui.edtExecutable->text().trimmed();
        tool->arguments = editor.ui.edtArgs->text();
        tool->input = editor.ui.edtInput->toPlainText();
        tool->workingDir = editor.ui.edtWorkingDir->text();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        tool->mimetypes = editor.ui.edtMimeType->text().split(QRegularExpression(QStringLiteral("\\s*;\\s*")), QString::SkipEmptyParts);
#else
        tool->mimetypes = editor.ui.edtMimeType->text().split(QRegularExpression(QStringLiteral("\\s*;\\s*")), Qt::SkipEmptyParts);
#endif
        tool->saveMode = static_cast<KateExternalTool::SaveMode>(editor.ui.cmbSave->currentIndex());
        tool->reload = editor.ui.chkReload->isChecked();
        tool->outputMode = static_cast<KateExternalTool::OutputMode>(editor.ui.cmbOutput->currentIndex());
        tool->cmdname = editor.ui.edtCommand->text().trimmed();

        // sticky action collection name, never changes again, so that shortcuts stay
        if (tool->actionName.isEmpty()) {
            tool->actionName = QStringLiteral("externaltool_") + QString(tool->name).remove(QRegularExpression(QStringLiteral("\\W+")));
        }

        const auto tools = collectTools(m_toolsModel);
        makeActionNameUnique(tool, tools);
        makeEditorCommandUnique(tool, tools);

        changed = true;
    }

    m_config->group("Editor").writeEntry("Size", editor.size());
    m_config->sync();

    return changed;
}

void KateExternalToolsConfigWidget::lazyInitDefaultsMenu(QMenu *defaultsMenu)
{
    if (!defaultsMenu->isEmpty()) {
        return;
    }

    // create tool actions
    std::map<QString, QMenu *> categories;

    // first add categorized actions, such that the submenus appear at the top
    int defaultToolsIndex = 0;
    for (const auto &tool : m_plugin->defaultTools()) {
        const QString category = tool.category.isEmpty() ? i18n("Uncategorized") : tool.translatedCategory();
        auto categoryMenu = categories[category];
        if (!categoryMenu) {
            categoryMenu = new QMenu(category, this);
            categories[category] = categoryMenu;
            defaultsMenu->addMenu(categoryMenu);
        }

        auto a = categoryMenu->addAction(QIcon::fromTheme(tool.icon), tool.translatedName());
        a->setData(defaultToolsIndex);

        connect(a, &QAction::triggered, [this, a]() {
            slotAddDefaultTool(a->data().toInt());
        });
        ++defaultToolsIndex;
    }
}

void KateExternalToolsConfigWidget::slotAddDefaultTool(int defaultToolsIndex)
{
    const auto &defaultTools = m_plugin->defaultTools();
    if (defaultToolsIndex < 0 || defaultToolsIndex > defaultTools.size()) {
        return;
    }

    addNewTool(new KateExternalTool(defaultTools[defaultToolsIndex]));
}

void KateExternalToolsConfigWidget::addNewTool(KateExternalTool *tool)
{
    const auto tools = collectTools(m_toolsModel);
    makeActionNameUnique(tool, tools);
    makeEditorCommandUnique(tool, tools);

    auto item = newToolItem(tool->icon.isEmpty() ? blankIcon() : QIcon::fromTheme(tool->icon), tool);
    auto category = addCategory(tool->translatedCategory());
    category->appendRow(item);
    lbTools->setCurrentIndex(item->index());

    Q_EMIT changed();
    m_changed = true;
}

QStandardItem *KateExternalToolsConfigWidget::addCategory(const QString &translatedCategory)
{
    if (translatedCategory.isEmpty() || (m_noCategory && translatedCategory == i18n("Uncategorized"))) {
        return m_noCategory;
    }

    // search for existing category
    auto items = m_toolsModel.findItems(translatedCategory);
    if (!items.empty()) {
        return items.front();
    }

    // ...otherwise, create it
    auto item = new QStandardItem(translatedCategory);

    // for now, categories are not movable, otherwise, the use can move a
    // category into another category, which is not supported right now
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsEditable);

    m_toolsModel.appendRow(item);
    return item;
}

QStandardItem *KateExternalToolsConfigWidget::currentCategory() const
{
    auto index = lbTools->currentIndex();
    if (!index.isValid()) {
        return m_noCategory;
    }

    auto item = m_toolsModel.itemFromIndex(index);
    auto tool = toolForItem(item);
    if (tool) {
        // the parent of a ToolItem is always a category
        return item->parent();
    }

    // item is no ToolItem, so we must have a category at hand
    return item;
}

void KateExternalToolsConfigWidget::clearTools()
{
    // collect all KateExternalTool items and delete them, since they are copies
    std::vector<KateExternalTool *> tools = collectTools(m_toolsModel);
    qDeleteAll(tools);
    tools.clear();
    m_toolsModel.clear();
}

void KateExternalToolsConfigWidget::slotAddCategory()
{
    // find unique name
    QString name = i18n("New Category");
    int i = 1;
    while (!m_toolsModel.findItems(name, Qt::MatchFixedString).isEmpty()) {
        name = (i18n("New Category %1", i++));
    }

    // add category and switch to edit mode
    auto item = addCategory(name);
    lbTools->edit(item->index());
}

void KateExternalToolsConfigWidget::slotAddTool()
{
    auto t = new KateExternalTool();
    if (editTool(t)) {
        addNewTool(t);
    } else {
        delete t;
    }
}

void KateExternalToolsConfigWidget::slotRemove()
{
    auto item = m_toolsModel.itemFromIndex(lbTools->currentIndex());
    auto tool = toolForItem(item);

    if (tool) {
        item->parent()->removeRow(item->index().row());
        delete tool;
        Q_EMIT changed();
        m_changed = true;
    }
}

void KateExternalToolsConfigWidget::slotEdit()
{
    auto item = m_toolsModel.itemFromIndex(lbTools->currentIndex());
    auto tool = toolForItem(item);
    if (!tool) {
        if (item) {
            lbTools->edit(item->index());
        }
        return;
    }
    // show the item in an editor
    if (editTool(tool)) {
        // renew the icon and name
        item->setText(tool->name);
        item->setIcon(tool->icon.isEmpty() ? blankIcon() : QIcon::fromTheme(tool->icon));

        Q_EMIT changed();
        m_changed = true;
    }
}
// END KateExternalToolsConfigWidget

// kate: space-indent on; indent-width 4; replace-tabs on;
