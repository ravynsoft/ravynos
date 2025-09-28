#define Uses_TColorAttr
#define Uses_TEvent
#define Uses_TWindow
#include <tvision/tv.h>

#include <internal/ansidisp.h>

#include <test.h>

namespace tvision
{

TEST(Endianess, AliasingInKeyDownEventShouldWorkCorrectly)
{
    KeyDownEvent keyDown {};
    keyDown.keyCode = 0x1234;
    EXPECT_EQ(keyDown.charScan.charCode, 0x34);
    EXPECT_EQ(keyDown.charScan.scanCode, 0x12);
}

TEST(Endianess, AliasingInMessageEventShouldWorkCorrectly)
{
    MessageEvent message {};
    message.infoPtr = (void *) 0x12345678;

    EXPECT_EQ(message.infoByte, 0x78);
    EXPECT_EQ(message.infoChar, 0x78);
    EXPECT_EQ(message.infoWord, 0x5678);
    EXPECT_EQ(message.infoInt, 0x5678);
    EXPECT_EQ(message.infoLong, 0x12345678);
}

TEST(Endianess, TWindowShouldHandleSelectCommand)
{
    short number = 1;
    TWindow window(TRect(0, 0, 0, 0), nullptr, number);

    void *response = message(&window, evBroadcast, cmSelectWindowNum, (void *)(size_t) number);
    EXPECT_EQ(response, &window);

    window.shutDown();
}

TEST(Endianess, ColorsWithBitFieldsShouldBehaveAsExpected)
{
    TColorRGB rgb = 0x112233;
    EXPECT_EQ(rgb.r, 0x11);
    EXPECT_EQ(rgb.g, 0x22);
    EXPECT_EQ(rgb.b, 0x33);

    TColorBIOS bios = 0x3;
    EXPECT_EQ(bios.b, 1);
    EXPECT_EQ(bios.g, 1);
    EXPECT_EQ(bios.r, 0);
    EXPECT_EQ(bios.bright, 0);

    bios = 0xC;
    EXPECT_EQ(bios.b, 0);
    EXPECT_EQ(bios.g, 0);
    EXPECT_EQ(bios.r, 1);
    EXPECT_EQ(bios.bright, 1);
}

TEST(Endianess, TermColorShouldBehaveAsExpected)
{
    TColorRGB rgb = 0x123456;
    TermColor termRgb {rgb, TermColor::RGB};
    EXPECT_EQ(termRgb.type, TermColor::RGB);
    EXPECT_EQ(termRgb.bgr[0], 0x56);
    EXPECT_EQ(termRgb.bgr[1], 0x34);
    EXPECT_EQ(termRgb.bgr[2], 0x12);

    uint8_t idx = 15;
    TermColor termIdx {idx, TermColor::Indexed};
    EXPECT_EQ(termIdx.type, TermColor::Indexed);
    EXPECT_EQ(termIdx.idx, idx);

    TermColor termNoColor {TermColor::NoColor};
    EXPECT_EQ(termNoColor.type, TermColor::NoColor);
}

} // namespace tvision
