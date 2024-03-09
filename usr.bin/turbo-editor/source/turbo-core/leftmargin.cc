#include <turbo/editor.h>

namespace turbo {

LeftMarginView::LeftMarginView(int aDistance) noexcept :
    TSurfaceView(TRect(0, 0, 0, 0)),
    distanceFromView(aDistance)
{
    growMode = gfGrowHiY | gfFixed;
}

} // namespace turbo
