#include "doctree.h"
#include "editwindow.h"
#include "app.h"
#include <utility>
#include <cassert>
#include <turbo/tpath.h>
using Node = DocumentTreeView::Node;

Node::Node(Node *parent, std::string_view p) noexcept :
    TNode(TPath::basename(p)),
    ptr(nullptr),
    parent(parent),
    data(std::string {p})
{
}

Node::Node(Node *parent, EditorWindow *w) noexcept :
    TNode(w->getTitle()),
    ptr(nullptr),
    parent(parent),
    data(w)
{
}

bool Node::hasEditor() const noexcept
{
    return std::holds_alternative<EditorWindow *>(data);
}

EditorWindow* Node::getEditor() noexcept
{
    if (auto **pw = std::get_if<EditorWindow *>(&data))
        return *pw;
    return nullptr;
}

void Node::remove() noexcept
{
    if (next)
        ((Node *) next)->ptr = ptr;
    if (ptr)
        *ptr = next;
    next = nullptr;
    ptr = nullptr;
    if (parent && !parent->childList)
        parent->dispose();
    parent = nullptr;
}

void Node::dispose() noexcept
{
    assert(!childList);
    remove();
    delete this;
}

// Directories shall appear before files.
enum NodeType { ntDir, ntEditor };
using NodeKey = std::pair<NodeType, std::string_view>;

static NodeKey getKey(Node *node) noexcept
{
    if (node->hasEditor())
        return {ntEditor, node->text};
    return {ntDir, node->text};
}

static void putNode(TNode **indirect, Node *node) noexcept
// Pre: node->parent is properly set.
{
    auto key = getKey(node);
    TNode *other;
    while ((other = *indirect))
    {
        if (key < getKey((Node *) other))
        {
            node->next = other;
            ((Node *) other)->ptr = &node->next;
            break;
        }
        indirect = &other->next;
    }
    *indirect = node;
    node->ptr = indirect;
}

static void setParent(Node *node, Node *parent) noexcept
{
    if (!parent)
        node->dispose();
    else if (parent != node->parent)
    {
        node->remove(); // May free the parent, hence the 'parent != node->parent' check.
        node->parent = parent;
        putNode(&parent->childList, node);
    }
}

void DocumentTreeView::focused(int i) noexcept
{
    // Avoid reentrancy on focus:
    // focused() => w->focus() => app->setFocusedEditor() => focusEditor() => focused()
    if (!focusing)
    {
        focusing = true;
        TOutline::focused(i);
        if (auto *node = (Node *) getNode(i))
            if (auto *w = node->getEditor())
                w->focus();
        update();
        focusing = false;
    }
}

void DocumentTreeView::addEditor(EditorWindow *w) noexcept
{
    Node *parent;
    TNode **list;
    if (w->filePath().empty()) {
        parent = nullptr;
        list = &root;
    } else {
        parent = getDirNode(TPath::dirname(w->filePath()));
        list = &parent->childList;
    }
    putNode(list, new Node(parent, w));
    update();
    drawView();
}

void DocumentTreeView::focusEditor(EditorWindow *w) noexcept
{
    // Avoid the reentrant case (see focused()).
    if (focusing)
        return;
    int i;
    if (findByEditor(w, &i))
        focused(i);
    drawView();
}

void DocumentTreeView::removeEditor(EditorWindow *w) noexcept
{
    Node *f = findByEditor(w);
    if (f) {
        f->dispose();
        update();
        drawView();
    }
}

void DocumentTreeView::focusNext() noexcept
{
    firstThat([this] (auto *node, auto pos) {
        if (((Node *) node)->hasEditor() && pos > foc) {
            focused(pos);
            drawView();
            return true;
        }
        return false;
    });
}

void DocumentTreeView::focusPrev() noexcept
{
    int prevPos = -1;
    firstThat([this, &prevPos] (auto *node, auto pos) {
        if (((Node *) node)->hasEditor()) {
            if (pos < foc)
                prevPos = pos;
            else if (prevPos >= 0) {
                focused(prevPos);
                drawView();
                return true;
            }
        }
        return false;
    });
}

Node* DocumentTreeView::getDirNode(std::string_view dirPath) noexcept
{
    // The list where the dir will be inserted.
    TNode **list {nullptr};
    Node *parent {nullptr};
    {
        auto parentPath = TPath::dirname(dirPath);
        if ((parent = findByPath(parentPath)))
            list = &parent->childList;
    }
    if (!list)
        list = &root;
    // The directory we are searching for.
    auto *dir = (Node *) findInList(list, [dirPath] (Node *node) {
        auto *ppath = std::get_if<std::string>(&node->data);
        return ppath && *ppath == dirPath;
    });
    if (!dir) {
        dir = new Node(parent, dirPath);
        // Place already existing subdirectories under this dir.
        findInList(&root, [dir, dirPath] (Node *node) {
            auto *ppath = std::get_if<std::string>(&node->data);
            if (ppath && TPath::dirname(*ppath) == TStringView(dirPath))
                setParent(node, dir);
            return false;
        });
        putNode(list, dir);
    }
    return dir;
}

Node* DocumentTreeView::findByEditor(const EditorWindow *w, int *pos) noexcept
{
    return firstThat(
    [w, pos] (Node *node, int position)
    {
        auto *w_ = node->getEditor();
        if (w_ && w_ == w) {
            if (pos)
                *pos = position;
            return true;
        }
        return false;
    });
}


Node* DocumentTreeView::findByPath(std::string_view path) noexcept
{
    return firstThat(
    [path] (Node *node, int)
    {
        auto *ppath = std::get_if<std::string>(&node->data);
        return ppath && *ppath == path;
    });
}

DocumentTreeWindow::DocumentTreeWindow(const TRect &bounds, DocumentTreeWindow **ptr) noexcept :
    TWindowInit(&DocumentTreeWindow::initFrame),
    TWindow(bounds, "Open Editors", wnNoNumber),
    ptr(ptr)
{
    auto *hsb = standardScrollBar(sbHorizontal),
         *vsb = standardScrollBar(sbVertical);
    tree = new DocumentTreeView(getExtent().grow(-1, -1), hsb, vsb, nullptr);
    tree->growMode = gfGrowHiX | gfGrowHiY;
    insert(tree);
}

DocumentTreeWindow::~DocumentTreeWindow()
{
    if (ptr)
        *ptr = nullptr;
}

void DocumentTreeWindow::close()
{
    message(TurboApp::app, evCommand, cmToggleTree, 0);
}
