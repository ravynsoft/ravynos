/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "nine_debug.h"

#include <ctype.h>
#include "c11/threads.h"

static const struct debug_named_value nine_debug_flags[] = {
    { "unknown", DBG_UNKNOWN,              "IUnknown implementation." },
    { "adapter", DBG_ADAPTER,              "ID3D9Adapter implementation." },
    { "overlay", DBG_OVERLAYEXTENSION,     "IDirect3D9ExOverlayExtension implementation." },
    { "auth",    DBG_AUTHENTICATEDCHANNEL, "IDirect3DAuthenticatedChannel9 implementation." },
    { "basetex", DBG_BASETEXTURE,          "IDirect3DBaseTexture9 implementation." },
    { "crypto",  DBG_CRYPTOSESSION,        "IDirect3DCryptoSession9 implementation." },
    { "cubetex", DBG_CUBETEXTURE,          "IDirect3DCubeTexture9 implementation." },
    { "device",  DBG_DEVICE,               "IDirect3DDevice9(Ex) implementation." },
    { "video",   DBG_DEVICEVIDEO,          "IDirect3DDeviceVideo9 implementation." },
    { "ibuf",    DBG_INDEXBUFFER,          "IDirect3DIndexBuffer9 implementation." },
    { "ps",      DBG_PIXELSHADER,          "IDirect3DPixelShader9 implementation." },
    { "query",   DBG_QUERY,                "IDirect3DQuery9 implementation." },
    { "res",     DBG_RESOURCE,             "IDirect3DResource9 implementation." },
    { "state",   DBG_STATEBLOCK,           "IDirect3DStateBlock9 implementation." },
    { "surf",    DBG_SURFACE,              "IDirect3DSurface9 implementation." },
    { "swap",    DBG_SWAPCHAIN,            "IDirect3DSwapChain9(Ex) implementation." },
    { "tex",     DBG_TEXTURE,              "IDirect3DTexture9 implementation." },
    { "vbuf",    DBG_VERTEXBUFFER,         "IDirect3DVertexBuffer9 implementation." },
    { "vdecl",   DBG_VERTEXDECLARATION,    "IDirect3DVertexDeclaration9 implementation." },
    { "vs",      DBG_VERTEXSHADER,         "IDirect3DVertexShader9 implementation." },
    { "3dsurf",  DBG_VOLUME,               "IDirect3DVolume9 implementation." },
    { "3dtex",   DBG_VOLUMETEXTURE,        "IDirect3DVolumeTexture9 implementation." },
    { "shader",  DBG_SHADER,               "Shader token stream translator." },
    { "ff",      DBG_FF,                   "Fixed function emulation." },
    { "user",    DBG_USER,                 "User errors, both fixable and unfixable." },
    { "error",   DBG_ERROR,                "Driver errors, always visible." },
    { "warn",    DBG_WARN,                 "Driver warnings, always visible in debug builds." },
    { "tid",     DBG_TID,                  "Display thread-ids." },
    DEBUG_NAMED_VALUE_END
};

void
_nine_debug_printf( unsigned long flag,
                    const char *func,
                    const char *fmt,
                    ... )
{
    static bool first = true;
    static unsigned long dbg_flags = DBG_ERROR | DBG_WARN;
    unsigned long tid = 0;

    if (first) {
        first = false;
        dbg_flags |= debug_get_flags_option("NINE_DEBUG", nine_debug_flags, 0);
    }

#if defined(HAVE_PTHREAD)
    if (dbg_flags & DBG_TID)
        tid = (unsigned long)pthread_self();
#endif

    if (dbg_flags & flag) {
        const char *f = func ? strrchr(func, '_') : NULL;
        va_list ap;
        /* inside a class this will print nine:tid:classinlowercase:func: while
         * outside a class (rarely used) it will just print nine:tid:func
         * the reason for lower case is simply to match the filenames, as it
         * will also strip off the "Nine" */
        if (f && strncmp(func, "Nine", 4) == 0) {
            char klass[96]; /* no class name is this long */
            char *ptr = klass;
            for (func += 4; func != f; ++func) { *ptr++ = tolower(*func); }
            *ptr = '\0';
            if (tid)
                _debug_printf("nine:0x%08lx:%s:%s: ", tid, klass, ++f);
            else
                _debug_printf("nine:%s:%s: ", klass, ++f);
        } else if (func) {
            if (tid)
                _debug_printf("nine:0x%08lx:%s ", tid, func);
            else
                _debug_printf("nine:%s ", func);
        }

        va_start(ap, fmt);
        _debug_vprintf(fmt, ap);
        va_end(ap);
    }
}

void
_nine_stub( const char *file,
            const char *func,
            unsigned line )
{
    const char *r = strrchr(file, '/');
    if (r == NULL) { r = strrchr(file, '\\'); }
    _debug_printf("nine:%s:%d: %s STUB!\n", r ? ++r : file, line, func);
}
