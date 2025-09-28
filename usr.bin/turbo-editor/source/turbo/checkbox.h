#ifndef TURBO_CHECKBOX_H
#define TURBO_CHECKBOX_H

#define Uses_TSItem
#define Uses_TCheckBoxes
#include <tvision/tv.h>

#include "cmds.h"

class CheckBox : public TCheckBoxes
{
    void press(int) override;

public:

    CheckBox(const TRect &bounds, TStringView text) noexcept :
        TCheckBoxes(bounds, new TSItem(text, nullptr))
    {
    }

    bool isChecked();
    void setChecked(bool checked);
};

#endif // TURBO_CHECKBOX_H
