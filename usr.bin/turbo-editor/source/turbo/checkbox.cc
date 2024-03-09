#define Uses_TGroup
#include <tvision/tv.h>

#include "checkbox.h"

void CheckBox::press(int)
{
    TCheckBoxes::press(0);
    message(owner, evBroadcast, cmStateChanged, nullptr);
}

bool CheckBox::isChecked()
{
    return mark(0);
}

void CheckBox::setChecked(bool checked)
{
    if (checked != isChecked())
        TCheckBoxes::press(0);
}
