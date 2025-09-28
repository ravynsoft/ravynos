threads = 4

function TerminalPreview(clip c, string "mode")
{
    c
    PointResize(width/8, height/8)
    TermColor(mode)
    PointResize(width*2, height*2)
}

LWLibavVideoSource("termcolor.png", format="RGB32")
StackHorizontal( \
    StackVertical(TerminalPreview("indexed256"), TerminalPreview("indexed16")), \
    StackVertical(TerminalPreview("direct"), TerminalPreview("indexed8")) \
)

Prefetch(threads)
