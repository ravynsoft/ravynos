#ifndef TURBO_COMBOBOX_H
#define TURBO_COMBOBOX_H

#define Uses_TView
#include <tvision/tv.h>

#include "listviews.h"

/* ---------------------------------------------------------------------- */
/*      class ComboBox                                                    */
/*                                                                        */
/*      Palette layout (TDialog)                                          */
/*        1 = Text passive                                                */
/*        2 = Text active                                                 */
/*        3 = Text arrow                                                  */
/*        4 = Icon arrow                                                  */
/*        5 = Icon sides                                                  */
/* ---------------------------------------------------------------------- */

class ComboBox : public TView
{
    const ListModel &model;
    short currentIndex {0};

    static const TStringView icon;
    static const TStringView rightArrow;

    int calcBodySize() noexcept;
    void showPopup(int) noexcept;

public:

    // The lifetime of 'aModel' must exceed that of 'this'.
    ComboBox(const TRect &bounds, const ListModel &aModel) noexcept;

    void draw() override;
    TPalette &getPalette() const override;
    void handleEvent(TEvent &) override;

    void *getCurrent() const noexcept;
    void setCurrentIndex(short i) noexcept;
};

#endif // TURBO_COMBOBOX_H
