#define Uses_TKeys
#define Uses_TEvent
#define Uses_TProgram
#include <tvision/tv.h>

#include "combobox.h"

#define cpComboBox "\x13\x13\x15\x16\x17"

const TStringView ComboBox::icon = "▐~↓~▌";
const TStringView ComboBox::rightArrow = "►";

ComboBox::ComboBox(const TRect &bounds, const ListModel &aModel) noexcept :
    TView(bounds),
    model(aModel)
{
    options |= ofSelectable | ofFirstClick;
}

int ComboBox::calcBodySize() noexcept
{
    int textAreaSize = maxWidth(model) + 2;
    int iconSize = cstrlen(icon);
    return min(size.x, textAreaSize + iconSize);
}

void ComboBox::draw()
{
    TDrawBuffer b;

    int bodySize = calcBodySize();
    int iconSize = cstrlen(icon);

    TAttrPair iconColors = getColor(0x0405);

    if (bodySize >= iconSize)
        b.moveCStr(bodySize - iconSize, icon, iconColors);
    else
        b.moveCStr(0, icon, iconColors, bodySize, iconSize - bodySize);
    if (bodySize > iconSize)
    {
        TStringView text = model.getText(model.at(currentIndex));
        int textWidth = strwidth(text);

        TColorAttr textColor = getColor((state & sfFocused) ? 2 : 1);
        TColorAttr arrowColor = getColor(3);

        b.moveChar(0, ' ', textColor, bodySize - iconSize);
        b.moveStr(1, text, textColor, bodySize - iconSize - 1);
        if (bodySize - iconSize - 1 < 1 + textWidth)
            b.moveStr(bodySize - iconSize - 1, rightArrow, arrowColor, 1);
    }
    if (bodySize < size.x)
        b.moveChar(bodySize, ' ', iconColors[0], size.x - bodySize);
    writeLine( 0, 0, size.x, size.y, b );
}

TPalette &ComboBox::getPalette() const
{
    static TPalette palette( cpComboBox, sizeof( cpComboBox )-1 );
    return palette;
}

void *ComboBox::getCurrent() const noexcept
{
    return model.at(currentIndex);
}

void ComboBox::setCurrentIndex(short i) noexcept
{
    currentIndex = i;
}

void ComboBox::handleEvent(TEvent &ev)
{
    TView::handleEvent(ev);

    if (ev.what == evMouseDown)
    {
        int bodySize = calcBodySize();
        TPoint where = makeLocal(ev.mouse.where);
        if (0 <= where.x && where.x < bodySize && where.y == 0)
        {
            showPopup(0);
            clearEvent(ev);
        }
    }
    else if ( ev.what == evKeyDown &&
              (ev.keyDown.keyCode == kbDown || ev.keyDown.keyCode == kbUp) )
    {
        showPopup(ev.keyDown.keyCode == kbDown ? 1 : -1);
        clearEvent(ev);
    }
}

void ComboBox::showPopup(int indexOffset) noexcept
{
    TGroup *app = TProgram::application;
    TPoint globalOrigin = owner->makeGlobal(origin);
    int popupWidth = maxWidth(model) + 4;
    int popupHeight = model.size() + 2;
    int spaceBelow = app->size.y - globalOrigin.y - 1;
    bool above = spaceBelow < popupHeight && spaceBelow < globalOrigin.y;
    TPoint popupOrigin {globalOrigin.x - 1, globalOrigin.y + (above ? -popupHeight : 1)};
    TRect r {popupOrigin.x, popupOrigin.y, popupOrigin.x + popupWidth, popupOrigin.y + popupHeight};

    auto *popup = new ListWindow(r, nullptr, model, ListViewCreator<ListView>(lvNoScrollBars | lvSelectSingleClick));
    popup->flags = 0;
    popup->state &= ~sfShadow;
    popup->growMode = (growMode & ~gfGrowHiX) | gfGrowLoY | gfGrowHiY;
    popup->setCurrentIndex(currentIndex + indexOffset);

    if (app->execView(popup) == cmOK)
    {
        int oldIndex = currentIndex;
        currentIndex = popup->getCurrentIndex();
        drawView();
        if (oldIndex != currentIndex)
            message(owner, evBroadcast, cmStateChanged, this);
    }

    destroy(popup);
}


