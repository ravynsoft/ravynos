#include <turbo/scintilla.h>
#include <turbo/scintilla/internals.h>

namespace Scintilla {

ColourDesired Platform::Chrome()
{
    return {0x0, 0x0, 0x0};
}

ColourDesired Platform::ChromeHighlight()
{
    return {0x0, 0x0, 0x0};
}

const char* Platform::DefaultFont()
{
    return "";
}

int Platform::DefaultFontSize()
{
    return 1;
}

unsigned int Platform::DoubleClickTime()
{
    return 500;
}

void Platform::DebugDisplay(const char *s)
{
}

void Platform::DebugPrintf(const char *format, ...)
{
}

bool Platform::ShowAssertionPopUps(bool assertionPopUps_)
{
    return false;
}

void Platform::Assert(const char *c, const char *file, int line)
{
}

} // namespace Scintilla
