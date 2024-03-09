#ifndef TURBO_EDITWINDOW_H
#define TURBO_EDITWINDOW_H

#define Uses_TWindow
#define Uses_TPalette
#include <tvision/tv.h>

#include <turbo/fileeditor.h>
#include <turbo/basicwindow.h>
#include "apputils.h"
#include "editor.h"
#include "search.h"

struct FileNumberState
{
    active_counter *counter;
    uint number;

    FileNumberState(active_counter &aCounter) noexcept;
    ~FileNumberState();
    FileNumberState &operator=(FileNumberState &&other) noexcept;
};

struct TitleState
{
    active_counter *counter;
    uint number;
    bool inSavePoint;

    bool operator!=(const TitleState &other) const noexcept
    {
        return !( counter == other.counter && number == other.number &&
                  inSavePoint == other.inSavePoint );
    }
};

struct EditorWindow;

struct EditorWindowParent
{
    virtual void handleFocus(EditorWindow &w) noexcept = 0;
    virtual void handleTitleChange(EditorWindow &w) noexcept = 0;
    virtual void removeEditor(EditorWindow &w) noexcept = 0;
    virtual const char *getFileDialogDir() noexcept = 0;
};

struct EditorWindow : public turbo::BasicEditorWindow
{
    using super = turbo::BasicEditorWindow;

    list_head<EditorWindow> listHead;
    FileNumberState fileNumber;
    EditorWindowParent &parent;
    TitleState lastTitleState {};
    std::string title;
    TCommandSet enabledCmds, disabledCmds;

    TView *bottomView {nullptr};
    SearchState searchState;

    EditorWindow( const TRect &bounds, TurboEditor &aEditor, active_counter &fileCounter,
                  turbo::SearchSettings &searchSettings, EditorWindowParent &aParent ) noexcept;

    void shutDown() override;
    void handleEvent(TEvent &ev) override;
    void setState(ushort aState, Boolean enable) override;
    Boolean valid(ushort command) override;
    const char *getTitle(short = 0) override;
    void sizeLimits(TPoint &min, TPoint &max) override;
    void updateCommands() noexcept;
    void handleNotification(const SCNotification &scn, turbo::Editor &) override;

    void closeBottomView();
    void setBottomView(TView *view);
    template <class T, class ...Args>
    void openBottomView(Args&& ...args);

    enum TitleFormatFlags
    {
        tfNoSavePoint = 0x0001,
    };

    const char *formatTitle(ushort flags = 0) noexcept;

    auto &getEditor() { return (TurboEditor &) super::editor; }
    auto &filePath() { return getEditor().filePath; }

};

inline FileNumberState::FileNumberState(active_counter &aCounter) noexcept :
    counter(&aCounter),
    number(++aCounter)
{
}

inline FileNumberState::~FileNumberState()
{
    --*counter;
}

inline FileNumberState &FileNumberState::operator=(FileNumberState &&other) noexcept
{
    std::swap(counter, other.counter);
    std::swap(number, other.number);
    return *this;
}

#endif
