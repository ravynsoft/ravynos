#include <turbo/scintilla/internals.h>

namespace Scintilla {

class ListBoxTV : public ListBox
{
    void SetFont(Font &) override
    {
    }

    void Create(Window &, int, Point, int, bool, int) override
    {
    }

    void SetAverageCharWidth(int) override
    {
    }

    void SetVisibleRows(int) override
    {
    }

    int GetVisibleRows() const override
    {
        return 0;
    }

    PRectangle GetDesiredRect() override
    {
        return PRectangle();
    }

    int CaretFromEdge() override
    {
        return 0;
    }

    void Clear() override
    {
    }

    void Append(char *, int) override
    {
    }

    int Length() override
    {
        return 0;
    }

    void Select(int n) override
    {
    }

    int GetSelection() override
    {
        return 0;
    }

    int Find(const char *) override
    {
        return 0;
    }

    void GetValue(int, char *, int ) override
    {
    }

    void RegisterImage(int, const char *) override
    {
    }

    void RegisterRGBAImage(int , int , int , const unsigned char *) override
    {
    }

    void ClearRegisteredImages() override
    {
    }

    void SetDelegate(IListBoxDelegate *) override
    {
    }

    void SetList(const char*, char, char) override
    {
    }
};

ListBox::ListBox() noexcept
{
}

ListBox::~ListBox()
{
}

ListBox* ListBox::Allocate()
{
    return new ListBoxTV;
}

} // namespace Scintilla
