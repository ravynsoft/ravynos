// TermColor: AviSynth filter that demonstrates the terminal color
// quantizations used by Turbo Vision.
//
// Tested with AVS+ 3.7, Linux x86_64.
//
// TermColor(clip c, string "mode")
//
// * clip c: input clip must be RGB24 or RGB32.
// * string mode: one of: indexed8, indexed16, indexed256, direct.

#include <avisynth.h>

#define Uses_TPoint
#define Uses_TColorAttr
#include <tvision/tv.h>

enum TermColors
{
    Indexed8,
    Indexed16,
    Indexed256,
    Direct,
    TermColorCount,
};

enum PixelTypes
{
    RGB24,
    RGB32,
    PixelTypeCount,
};

using FrameProcessor = void (const PVideoFrame &, PVideoFrame &, TPoint);

struct TermColor : public GenericVideoFilter
{

    FrameProcessor *processor;

    TermColor(PClip, FrameProcessor *);

    int SetCacheHints(int cachehints, int frame_range) override;
    PVideoFrame GetFrame(int n, IScriptEnvironment* env) override;

    static AVSValue Create(AVSValue args, void *, IScriptEnvironment *env);

};

const AVS_Linkage *AVS_linkage = 0;

extern "C" const char* AvisynthPluginInit3(IScriptEnvironment* env, AVS_Linkage* vectors)
{
    AVS_linkage = vectors;
    env->AddFunction("TermColor", "c[mode]s", TermColor::Create, 0);
    return 0;
}

template <uint32_t transform(uint32_t, TPoint), bool alpha>
static void process_rgb(const PVideoFrame &src, PVideoFrame &dst, TPoint size)
{
    const uint8_t *srcp = src->GetReadPtr();
    uint8_t *dstp = dst->GetWritePtr();
    auto src_pitch = src->GetPitch(),
         dst_pitch = dst->GetPitch();
    constexpr size_t pixel_size = 3 + alpha;
    for (int y = 0; y < size.y; ++y)
    {
        size_t stride = 0;
        for (int x = 0; x < size.x; ++x, stride += pixel_size)
        {
            uint32_t in = 0;
            memcpy(&in, &srcp[stride], pixel_size);
            uint32_t out = transform(in, {x, y});
            if (alpha) out |= in & 0xFF000000;
            memcpy(&dstp[stride], &out, pixel_size);
        }
        srcp += src_pitch;
        dstp += dst_pitch;
    }
}

static inline uint32_t quantize_indexed8(uint32_t, TPoint);
static inline uint32_t quantize_indexed16(uint32_t, TPoint);
static inline uint32_t quantize_indexed256(uint32_t, TPoint);

static constexpr FrameProcessor *frameProcessors[PixelTypeCount][TermColorCount] =
{
    { // RGB24
        &process_rgb<quantize_indexed8, false>,
        &process_rgb<quantize_indexed16, false>,
        &process_rgb<quantize_indexed256, false>,
        nullptr,
    },
    { // RGB32
        &process_rgb<quantize_indexed8, true>,
        &process_rgb<quantize_indexed16, true>,
        &process_rgb<quantize_indexed256, true>,
        nullptr,
    },
};

static constexpr struct { const char *name; TermColors value; } quantizeModes[] =
{
    {"indexed8", Indexed8},
    {"indexed16", Indexed16},
    {"indexed256", Indexed256},
    {"direct", Direct},
};

AVSValue TermColor::Create(AVSValue args, void *, IScriptEnvironment *env)
{
    auto child = args[0].AsClip();

    PixelTypes pixel = PixelTypeCount;
    auto vi = child->GetVideoInfo();
    if (vi.IsRGB24())
        pixel = RGB24;
    else if (vi.IsRGB32())
        pixel = RGB32;
    if (pixel == PixelTypeCount)
        env->ThrowError("TermColor: input clip must be RGB24 or RGB32.");

    TermColors mode = TermColorCount;
    TStringView modeName = args[1].AsString();
    for (auto m : quantizeModes)
        if (m.name == modeName)
        {
            mode = m.value;
            break;
        }
    if (mode == TermColorCount)
        env->ThrowError("TermColor: 'mode' must be one of: indexed8, indexed16, indexed256, direct.");

    return new TermColor(child, frameProcessors[pixel][mode]);
}

inline TermColor::TermColor(PClip aChild, FrameProcessor *aProcessor) :
    GenericVideoFilter(aChild),
    processor(aProcessor)
{
}

int TermColor::SetCacheHints(int cachehints, int)
{
    return cachehints == CACHE_GET_MTMODE ? MT_NICE_FILTER : 0;
}

PVideoFrame TermColor::GetFrame(int n, IScriptEnvironment* env)
{
    PVideoFrame frame = child->GetFrame(n, env);
    if (processor)
    {
        env->MakeWritable(&frame);
        (*processor)(frame, frame, {vi.width, vi.height});
    }
    return frame;
}

static constexpr uint32_t xterm16_to_rgb[16] =
{
    0x000000,
    0x800000,
    0x008000,
    0x808000,
    0x000080,
    0x800080,
    0x008080,
    0xC0C0C0,
    0x808080,
    0xFF0000,
    0x00FF00,
    0xFFFF00,
    0x0000FF,
    0xFF00FF,
    0x00FFFF,
    0xFFFFFF,
};

static inline uint32_t quantize_indexed8(uint32_t rgb, TPoint pos)
{
    uint8_t idx = RGBtoXTerm16(rgb);
    if (idx >= 8 && (pos.y % 2 != 0))
        idx -= 8;
    return xterm16_to_rgb[idx];
}

static inline uint32_t quantize_indexed16(uint32_t rgb, TPoint)
{
    uint8_t idx = RGBtoXTerm16(rgb);
    return xterm16_to_rgb[idx];
}

static inline uint32_t quantize_indexed256(uint32_t rgb, TPoint)
{
    uint8_t idx = RGBtoXTerm256(rgb);
    return XTerm256toRGB(idx);
}
