#ifndef TURBO_DOCTREE_H
#define TURBO_DOCTREE_H

#define Uses_TWindow
#define Uses_TOutline
#include <tvision/tv.h>

#include <variant>
#include <string_view>

struct EditorWindow;

struct DocumentTreeView : public TOutline {

    struct Node : public TNode {

        TNode **ptr;
        Node *parent;
        std::variant<std::string, EditorWindow *> data;

        Node(Node *parent, std::string_view path) noexcept;
        Node(Node *parent, EditorWindow *w) noexcept;
        bool hasEditor() const noexcept;
        EditorWindow* getEditor() noexcept;
        void remove() noexcept;
        void dispose() noexcept;

    };

    bool focusing {false};

    using TOutline::TOutline;

    void focused(int i) noexcept override;

    void addEditor(EditorWindow *w) noexcept;
    void focusEditor(EditorWindow *w) noexcept;
    void removeEditor(EditorWindow *w) noexcept;
    void focusNext() noexcept;
    void focusPrev() noexcept;
    Node *getDirNode(std::string_view dirPath) noexcept;
    Node *findByEditor(const EditorWindow *w, int *pos=nullptr) noexcept;
    Node *findByPath(std::string_view path) noexcept;
    template <class Func>
    Node *firstThat(Func &&func) noexcept;

};

template <class Func>
inline DocumentTreeView::Node *DocumentTreeView::firstThat(Func &&func) noexcept
{
    auto applyCallback =
    [] ( TOutlineViewer *, TNode *node, int, int position,
         long, ushort, void *arg )
    {
        return (*(Func *) arg)((Node *) node, position);
    };

    return (Node *) TOutlineViewer::firstThat(applyCallback, &func);
}

struct DocumentTreeWindow : public TWindow {

    DocumentTreeView *tree;
    DocumentTreeWindow **ptr;

    DocumentTreeWindow(const TRect &bounds, DocumentTreeWindow **ptr) noexcept;
    ~DocumentTreeWindow();

    void close() override;

};

template <class Func>
inline TNode* findInList(TNode **list, Func &&test)
{
    auto *node = *list;
    while (node) {
        auto *next = node->next;
        if (test((DocumentTreeView::Node *) node))
            return node;
        node = next;
    }
    return nullptr;
}

#endif
