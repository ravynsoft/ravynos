
#include "nine_debug.h"
#include "nine_pipe.h"

#include <stdio.h>
#include "c11/threads.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "nine_dump.h"

#if defined(DEBUG) || !defined(NDEBUG)

static char thread_local tls[128];

const char *nine_D3DDEVTYPE_to_str(D3DDEVTYPE type)
{
    switch (type) {
    case D3DDEVTYPE_HAL: return "HAL";
    case D3DDEVTYPE_NULLREF: return "NULLREF";
    case D3DDEVTYPE_REF: return "REF";
    case D3DDEVTYPE_SW: return "SW";
    default:
       return "(D3DDEVTYPE_?)";
    }
}

const char *nine_D3DPOOL_to_str(D3DPOOL pool)
{
    switch (pool) {
    case D3DPOOL_DEFAULT: return "DEFAULT";
    case D3DPOOL_MANAGED: return "MANAGED";
    case D3DPOOL_SYSTEMMEM: return "SYSTEMMEM";
    case D3DPOOL_SCRATCH: return "SCRATCH";
    default:
        return "(D3DPOOL_?)";
    }
}

const char *nine_D3DSAMP_to_str(DWORD samp)
{
    switch (samp) {
    case D3DSAMP_ADDRESSU: return "ADDRESSU";
    case D3DSAMP_ADDRESSV: return "ADDRESSV";
    case D3DSAMP_ADDRESSW: return "ADDRESSW";
    case D3DSAMP_BORDERCOLOR: return "BORDERCOLOR";
    case D3DSAMP_MAGFILTER: return "MAGFILTER";
    case D3DSAMP_MINFILTER: return "MINFILTER";
    case D3DSAMP_MIPFILTER: return "MIPFILTER";
    case D3DSAMP_MIPMAPLODBIAS: return "MIPMAPLODBIAS";
    case D3DSAMP_MAXMIPLEVEL: return "MAXMIPLEVEL";
    case D3DSAMP_MAXANISOTROPY: return "MAXANISOTROPY";
    case D3DSAMP_SRGBTEXTURE: return "SRGBTEXTURE";
    case D3DSAMP_ELEMENTINDEX: return "ELEMENTINDEX";
    case D3DSAMP_DMAPOFFSET: return "DMAPOFFSET";
    default:
        return "(D3DSAMP_?)";
    }
}

#define C2S(n,s) \
    do { \
        if (usage & D3DUSAGE_##n) p += snprintf(&tls[p], sizeof(tls) - p, s); \
    } while(0)
const char *nine_D3DUSAGE_to_str(DWORD usage)
{
    int p = 0;
    tls[0] = 0;
    C2S(AUTOGENMIPMAP, "MIPGEN");
    C2S(WRITEONLY, "WO");
    C2S(DYNAMIC, "DYNAMIC");
    C2S(DEPTHSTENCIL, "DS");
    C2S(RENDERTARGET, "RT");
    C2S(SOFTWAREPROCESSING, "SW");
    C2S(DONOTCLIP, "NOCLIP");
    C2S(POINTS, "POINTS");
    C2S(DMAP, "DMAP");
    C2S(NPATCHES, "NPATCHES");
    C2S(RTPATCHES, "RTPATCHES");
    C2S(TEXTAPI, "TEXTAPI");
    C2S(NONSECURE, "NONSECURE");
    C2S(RESTRICTED_CONTENT, "RESTRICTED_CONTENT");
    C2S(RESTRICT_SHARED_RESOURCE, "RESTRICT_SHARED_RESOURCE");
    C2S(RESTRICT_SHARED_RESOURCE_DRIVER, "RESTRICT_SHARED_RESOURCE_DRIVER");
    return tls;
}
#undef C2S

#define C2S(n) \
    do { \
        if (flags & D3DPRESENTFLAG_##n) \
            p += snprintf(&tls[p], sizeof(tls) - p, #n); \
    } while(0)
const char *nine_D3DPRESENTFLAG_to_str(DWORD flags)
{
    int p = 0;
    tls[0] = 0;
    C2S(DEVICECLIP);
    C2S(DISCARD_DEPTHSTENCIL);
    C2S(LOCKABLE_BACKBUFFER);
    C2S(NOAUTOROTATE);
    C2S(UNPRUNEDMODE);
    C2S(VIDEO);
    C2S(OVERLAY_LIMITEDRGB);
    C2S(OVERLAY_YCbCr_BT709);
    C2S(OVERLAY_YCbCr_xvYCC);
    C2S(RESTRICTED_CONTENT);
    C2S(RESTRICT_SHARED_RESOURCE_DRIVER);
    return tls;
}
#undef C2S

#define C2S(n) \
    do { \
        if (lock & D3DLOCK_##n) p += snprintf(&tls[p], sizeof(tls) - p, #n"|"); \
    } while(0)
const char *nine_D3DLOCK_to_str(DWORD lock)
{
    int p = 0;
    tls[0] = 0;
    C2S(DISCARD);
    C2S(DONOTWAIT);
    C2S(NO_DIRTY_UPDATE);
    C2S(NOOVERWRITE);
    C2S(NOSYSLOCK);
    C2S(READONLY);
    return tls;
}
#undef C2S

const char *nine_D3DRTYPE_to_str(D3DRESOURCETYPE type)
{
    switch (type) {
    case D3DRTYPE_SURFACE: return "SURFACE";
    case D3DRTYPE_VOLUME: return "VOLUME";
    case D3DRTYPE_TEXTURE: return "TEXTURE";
    case D3DRTYPE_VOLUMETEXTURE: return "VOLUMETEXTURE";
    case D3DRTYPE_CUBETEXTURE: return "CUBETEXTURE";
    case D3DRTYPE_VERTEXBUFFER: return "VERTEXBUFFER";
    case D3DRTYPE_INDEXBUFFER: return "INDEXBUFFER";
    default:
        return "(D3DRTYPE_?)";
    }
}

const char *nine_D3DQUERYTYPE_to_str(D3DQUERYTYPE type)
{
    switch (type) {
    case D3DQUERYTYPE_VCACHE: return "VCACHE";
    case D3DQUERYTYPE_RESOURCEMANAGER: return "RESOURCEMANAGER";
    case D3DQUERYTYPE_VERTEXSTATS: return "VERTEXSTATS";
    case D3DQUERYTYPE_EVENT: return "EVENT";
    case D3DQUERYTYPE_OCCLUSION: return "OCCLUSION";
    case D3DQUERYTYPE_TIMESTAMP: return "TIMESTAMP";
    case D3DQUERYTYPE_TIMESTAMPDISJOINT: return "TIMESTAMPDISJOINT";
    case D3DQUERYTYPE_TIMESTAMPFREQ: return "TIMESTAMPFREQ";
    case D3DQUERYTYPE_PIPELINETIMINGS: return "PIPELINETIMINGS";
    case D3DQUERYTYPE_INTERFACETIMINGS: return "INTERFACETIMINGS";
    case D3DQUERYTYPE_VERTEXTIMINGS: return "VERTEXTIMINGS";
    case D3DQUERYTYPE_PIXELTIMINGS: return "PIXELTIMINGS";
    case D3DQUERYTYPE_BANDWIDTHTIMINGS: return "BANDWIDTHTIMINGS";
    case D3DQUERYTYPE_CACHEUTILIZATION: return "CACHEUTILIZATION";
    default:
        return "(D3DQUERYTYPE_?)";
    }
}

const char *nine_D3DTSS_to_str(D3DTEXTURESTAGESTATETYPE type)
{
    switch (type) {
    case D3DTSS_COLOROP: return "COLOROP";
    case D3DTSS_ALPHAOP: return "ALPHAOP";
    case D3DTSS_COLORARG0: return "COLORARG0";
    case D3DTSS_COLORARG1: return "COLORARG1";
    case D3DTSS_COLORARG2: return "COLORARG2";
    case D3DTSS_ALPHAARG0: return "ALPHAARG0";
    case D3DTSS_ALPHAARG1: return "ALPHAARG1";
    case D3DTSS_ALPHAARG2: return "ALPHAARG2";
    case D3DTSS_RESULTARG: return "RESULTARG";
    case D3DTSS_BUMPENVMAT00: return "BUMPENVMAT00";
    case D3DTSS_BUMPENVMAT01: return "BUMPENVMAT01";
    case D3DTSS_BUMPENVMAT10: return "BUMPENVMAT10";
    case D3DTSS_BUMPENVMAT11: return "BUMPENVMAT11";
    case D3DTSS_BUMPENVLSCALE: return "BUMPENVLSCALE";
    case D3DTSS_BUMPENVLOFFSET: return "BUMPENVLOFFSET";
    case D3DTSS_TEXCOORDINDEX: return "TEXCOORDINDEX";
    case D3DTSS_TEXTURETRANSFORMFLAGS: return "TEXTURETRANSFORMFLAGS";
    case D3DTSS_CONSTANT: return "CONSTANT";
    default:
        return "(D3DTSS_?)";
    }
}

#define D3DTOP_TO_STR_CASE(n) case D3DTOP_##n: return #n
const char *nine_D3DTOP_to_str(D3DTEXTUREOP top)
{
    switch (top) {
    D3DTOP_TO_STR_CASE(DISABLE);
    D3DTOP_TO_STR_CASE(SELECTARG1);
    D3DTOP_TO_STR_CASE(SELECTARG2);
    D3DTOP_TO_STR_CASE(MODULATE);
    D3DTOP_TO_STR_CASE(MODULATE2X);
    D3DTOP_TO_STR_CASE(MODULATE4X);
    D3DTOP_TO_STR_CASE(ADD);
    D3DTOP_TO_STR_CASE(ADDSIGNED);
    D3DTOP_TO_STR_CASE(ADDSIGNED2X);
    D3DTOP_TO_STR_CASE(SUBTRACT);
    D3DTOP_TO_STR_CASE(ADDSMOOTH);
    D3DTOP_TO_STR_CASE(BLENDDIFFUSEALPHA);
    D3DTOP_TO_STR_CASE(BLENDTEXTUREALPHA);
    D3DTOP_TO_STR_CASE(BLENDFACTORALPHA);
    D3DTOP_TO_STR_CASE(BLENDTEXTUREALPHAPM);
    D3DTOP_TO_STR_CASE(BLENDCURRENTALPHA);
    D3DTOP_TO_STR_CASE(PREMODULATE);
    D3DTOP_TO_STR_CASE(MODULATEALPHA_ADDCOLOR);
    D3DTOP_TO_STR_CASE(MODULATECOLOR_ADDALPHA);
    D3DTOP_TO_STR_CASE(MODULATEINVALPHA_ADDCOLOR);
    D3DTOP_TO_STR_CASE(MODULATEINVCOLOR_ADDALPHA);
    D3DTOP_TO_STR_CASE(BUMPENVMAP);
    D3DTOP_TO_STR_CASE(BUMPENVMAPLUMINANCE);
    D3DTOP_TO_STR_CASE(DOTPRODUCT3);
    D3DTOP_TO_STR_CASE(MULTIPLYADD);
    D3DTOP_TO_STR_CASE(LERP);
    default:
        return "(D3DTOP_?)";
    }
}

static const char *
nine_D3DLIGHTTYPE_to_str(D3DLIGHTTYPE type)
{
    switch (type) {
    case D3DLIGHT_POINT: return "POINT";
    case D3DLIGHT_SPOT: return "SPOT";
    case D3DLIGHT_DIRECTIONAL: return "DIRECTIONAL";
    default:
        return "(D3DLIGHT_?)";
    }
}

static const char *
nine_D3DTA_to_str(DWORD value)
{
    switch (value & D3DTA_SELECTMASK) {
    case D3DTA_DIFFUSE: return "DIFFUSE";
    case D3DTA_CURRENT: return "CURRENT";
    case D3DTA_TEXTURE: return "TEXTURE";
    case D3DTA_TFACTOR: return "TFACTOR";
    case D3DTA_SPECULAR: return "SPECULAR";
    case D3DTA_TEMP: return "TEMP";
    case D3DTA_CONSTANT: return "CONSTANT";
    default:
        return "(D3DTA_?)";
    }
}

static const char *
nine_D3DTSS_TCI_to_str(DWORD value)
{
    switch (value & 0xf0000) {
    case D3DTSS_TCI_PASSTHRU: return "PASSTHRU";
    case D3DTSS_TCI_CAMERASPACENORMAL: return "CAMERASPACENORMAL";
    case D3DTSS_TCI_CAMERASPACEPOSITION: return "CAMERASPACEPOSITION";
    case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
        return "CAMERASPACEREFLECTIONVECTOR";
    case D3DTSS_TCI_SPHEREMAP: return "SPHEREMAP";
    default:
        return "(D3DTSS_TCI_?)";
    }
}

static const char *
nine_D3DTTFF_to_str(DWORD value)
{
    switch (value) {
    case D3DTTFF_DISABLE: return "DISABLE";
    case D3DTTFF_COUNT1: return "COUNT1";
    case D3DTTFF_COUNT2: return "COUNT2";
    case D3DTTFF_COUNT3: return "COUNT3";
    case D3DTTFF_COUNT4: return "COUNT4";
    case D3DTTFF_PROJECTED: return "PROJECTED";
    default:
        return "(D3DTTFF_?)";
    }
}

void
nine_dump_D3DLIGHT9(unsigned ch, const D3DLIGHT9 *lit)
{
    DBG_FLAG(ch, "D3DLIGHT9(%p):\n"
             "Type: %s\n"
             "Diffuse: (%f %f %f %f)\n"
             "Specular: (%f %f %f %f)\n"
             "Ambient: (%f %f %f %f)\n"
             "Position: (%f %f %f)\n"
             "Direction: (%f %f %f)\n"
             "Range: %f\n"
             "Falloff: %f\n"
             "Attenuation: %f + %f * d + %f * d^2\n"
             "Theta: %f deg\n"
             "Phi: %f deg\n", lit,
             nine_D3DLIGHTTYPE_to_str(lit->Type),
             lit->Diffuse.r,lit->Diffuse.r,lit->Diffuse.g,lit->Diffuse.a,
             lit->Specular.r,lit->Specular.r,lit->Specular.g,lit->Specular.a,
             lit->Ambient.r,lit->Ambient.r,lit->Ambient.g,lit->Ambient.a,
             lit->Position.x,lit->Position.y,lit->Position.z,
             lit->Direction.x,lit->Direction.y,lit->Direction.z,
             lit->Range,lit->Falloff,
             lit->Attenuation0,lit->Attenuation1,lit->Attenuation2,
             lit->Theta * 360.0f / M_PI,lit->Phi * 360.0f / M_PI);
}

void
nine_dump_D3DMATERIAL9(unsigned ch, const D3DMATERIAL9 *mat)
{
    DBG_FLAG(ch, "D3DMATERIAL9(%p):\n"
             "Diffuse: (%f %f %f %f)\n"
             "Specular: (%f %f %f %f)\n"
             "Ambient: (%f %f %f %f)\n"
             "Emissive: (%f %f %f %f)\n"
             "Power: %f\n", mat,
             mat->Diffuse.r,mat->Diffuse.r,mat->Diffuse.g,mat->Diffuse.a,
             mat->Specular.r,mat->Specular.r,mat->Specular.g,mat->Specular.a,
             mat->Ambient.r,mat->Ambient.r,mat->Ambient.g,mat->Ambient.a,
             mat->Emissive.r,mat->Emissive.r,mat->Emissive.g,mat->Emissive.a,
             mat->Power);
}

void
nine_dump_D3DTSS_value(unsigned ch, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
    float rgba[4];

    switch (type) {
    case D3DTSS_COLOROP:
    case D3DTSS_ALPHAOP:
        DBG_FLAG(ch, "D3DTSS_%s = %s\n",
                 nine_D3DTSS_to_str(type), nine_D3DTOP_to_str(value));
        break;
    case D3DTSS_COLORARG0:
    case D3DTSS_COLORARG1:
    case D3DTSS_COLORARG2:
    case D3DTSS_ALPHAARG0:
    case D3DTSS_ALPHAARG1:
    case D3DTSS_ALPHAARG2:
    case D3DTSS_RESULTARG:
        DBG_FLAG(ch, "D3DTSS_%s = %s%s%s\n",
                 nine_D3DTSS_to_str(type),
                 (value & D3DTA_COMPLEMENT) ? "COMPLEMENT " : "",
                 (value & D3DTA_ALPHAREPLICATE) ? "ALPHAREPLICATE " : "",
                 nine_D3DTA_to_str(value));
        break;
    case D3DTSS_BUMPENVMAT00:
    case D3DTSS_BUMPENVMAT01:
    case D3DTSS_BUMPENVMAT10:
    case D3DTSS_BUMPENVMAT11:
    case D3DTSS_BUMPENVLSCALE:
    case D3DTSS_BUMPENVLOFFSET:
        DBG_FLAG(ch, "D3DTSS_%s = %f\n",
                 nine_D3DTSS_to_str(type), asfloat(value));
        break;
    case D3DTSS_TEXCOORDINDEX:
        DBG_FLAG(ch, "D3DTSS_TEXCOORDINDEX = %s %u\n",
                 nine_D3DTSS_TCI_to_str(value),
                 value & 0xffff);
        break;
    case D3DTSS_TEXTURETRANSFORMFLAGS:
        DBG_FLAG(ch, "D3DTSS_TEXTURETRANSFORMFLAGS = %s\n",
                 nine_D3DTTFF_to_str(value));
        break;
    case D3DTSS_CONSTANT:
        d3dcolor_to_rgba(rgba, value);
        DBG_FLAG(ch, "D3DTSS_CONSTANT = %f %f %f %F\n",
                 rgba[0],rgba[1],rgba[2],rgba[3]);
        break;
    default:
        DBG_FLAG(ch, "D3DTSS_? = 0x%08x\n", value);
        break;
    }
}

void
nine_dump_D3DADAPTER_IDENTIFIER9(unsigned ch, const D3DADAPTER_IDENTIFIER9 *id)
{
    DBG_FLAG(ch, "D3DADAPTER_IDENTIFIER9(%p):\n"
             "Driver: %s\n"
             "Description: %s\n"
             "DeviceName: %s\n"
             "DriverVersion: %08x.%08x\n"
             "VendorId: %x\n"
             "DeviceId: %x\n"
             "SubSysId: %x\n"
             "Revision: %u\n"
             "GUID: %08x.%04x.%04x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n"
             "WHQLLevel: %u\n", id, id->Driver, id->Description,
             id->DeviceName,
             id->DriverVersionLowPart, id->DriverVersionHighPart,
             id->VendorId, id->DeviceId, id->SubSysId,
             id->Revision,
             id->DeviceIdentifier.Data1,
             id->DeviceIdentifier.Data2,
             id->DeviceIdentifier.Data3,
             id->DeviceIdentifier.Data4[0],
             id->DeviceIdentifier.Data4[1],
             id->DeviceIdentifier.Data4[2],
             id->DeviceIdentifier.Data4[3],
             id->DeviceIdentifier.Data4[4],
             id->DeviceIdentifier.Data4[5],
             id->DeviceIdentifier.Data4[6],
             id->DeviceIdentifier.Data4[7],
             id->WHQLLevel);
}

#define C2S(args...) p += snprintf(&s[p],c-p,args)

#define CAP_CASE(m,p,n) \
    do {                     \
        if (caps->m & p##_##n) \
            C2S(" "#n); \
        else \
            C2S(" ("#n")"); \
    } while(0)

void
nine_dump_D3DCAPS9(unsigned ch, const D3DCAPS9 *caps)
{
    const int c = 1 << 17;
    int p = 0;
    char *s = (char *)MALLOC(c);

    if (!s) {
        DBG_FLAG(ch, "D3DCAPS9(%p): (out of memory)\n", caps);
        return;
    }

    C2S("DeviceType: %s\n", nine_D3DDEVTYPE_to_str(caps->DeviceType));

    C2S("AdapterOrdinal: %u\nCaps:", caps->AdapterOrdinal);
    if (caps->Caps & 0x20000)
        C2S(" READ_SCANLINE");
    if (caps->Caps & ~0x20000)
        C2S(" %x", caps->Caps & ~0x20000);

    C2S("\nCaps2:");
    CAP_CASE(Caps2, D3DCAPS2, CANAUTOGENMIPMAP);
    CAP_CASE(Caps2, D3DCAPS2, CANCALIBRATEGAMMA);
    CAP_CASE(Caps2, D3DCAPS2, CANSHARERESOURCE);
    CAP_CASE(Caps2, D3DCAPS2, CANMANAGERESOURCE);
    CAP_CASE(Caps2, D3DCAPS2, DYNAMICTEXTURES);
    CAP_CASE(Caps2, D3DCAPS2, FULLSCREENGAMMA);

    C2S("\nCaps3:");
    CAP_CASE(Caps3, D3DCAPS3, ALPHA_FULLSCREEN_FLIP_OR_DISCARD);
    CAP_CASE(Caps3, D3DCAPS3, COPY_TO_VIDMEM);
    CAP_CASE(Caps3, D3DCAPS3, COPY_TO_SYSTEMMEM);
    CAP_CASE(Caps3, D3DCAPS3, DXVAHD);
    CAP_CASE(Caps3, D3DCAPS3, LINEAR_TO_SRGB_PRESENTATION);

    C2S("\nPresentationIntervals:");
    CAP_CASE(PresentationIntervals, D3DPRESENT_INTERVAL, ONE);
    CAP_CASE(PresentationIntervals, D3DPRESENT_INTERVAL, TWO);
    CAP_CASE(PresentationIntervals, D3DPRESENT_INTERVAL, THREE);
    CAP_CASE(PresentationIntervals, D3DPRESENT_INTERVAL, FOUR);
    CAP_CASE(PresentationIntervals, D3DPRESENT_INTERVAL, IMMEDIATE);

    C2S("\nCursorCaps:");
    CAP_CASE(CursorCaps, D3DCURSORCAPS, COLOR);
    CAP_CASE(CursorCaps, D3DCURSORCAPS, LOWRES);

    C2S("\nDevCaps:");
    CAP_CASE(DevCaps, D3DDEVCAPS, CANBLTSYSTONONLOCAL);
    CAP_CASE(DevCaps, D3DDEVCAPS, CANRENDERAFTERFLIP);
    CAP_CASE(DevCaps, D3DDEVCAPS, DRAWPRIMITIVES2);
    CAP_CASE(DevCaps, D3DDEVCAPS, DRAWPRIMITIVES2EX);
    CAP_CASE(DevCaps, D3DDEVCAPS, DRAWPRIMTLVERTEX);
    CAP_CASE(DevCaps, D3DDEVCAPS, EXECUTESYSTEMMEMORY);
    CAP_CASE(DevCaps, D3DDEVCAPS, EXECUTEVIDEOMEMORY);
    CAP_CASE(DevCaps, D3DDEVCAPS, HWRASTERIZATION);
    CAP_CASE(DevCaps, D3DDEVCAPS, HWTRANSFORMANDLIGHT);
    CAP_CASE(DevCaps, D3DDEVCAPS, NPATCHES);
    CAP_CASE(DevCaps, D3DDEVCAPS, PUREDEVICE);
    CAP_CASE(DevCaps, D3DDEVCAPS, QUINTICRTPATCHES);
    CAP_CASE(DevCaps, D3DDEVCAPS, RTPATCHES);
    CAP_CASE(DevCaps, D3DDEVCAPS, RTPATCHHANDLEZERO);
    CAP_CASE(DevCaps, D3DDEVCAPS, SEPARATETEXTUREMEMORIES);
    CAP_CASE(DevCaps, D3DDEVCAPS, TEXTURENONLOCALVIDMEM);
    CAP_CASE(DevCaps, D3DDEVCAPS, TEXTURESYSTEMMEMORY);
    CAP_CASE(DevCaps, D3DDEVCAPS, TEXTUREVIDEOMEMORY);
    CAP_CASE(DevCaps, D3DDEVCAPS, TLVERTEXSYSTEMMEMORY);
    CAP_CASE(DevCaps, D3DDEVCAPS, TLVERTEXVIDEOMEMORY);

    C2S("\nPrimitiveMiscCaps:");
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, MASKZ);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, CULLNONE);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, CULLCW);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, CULLCCW);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, COLORWRITEENABLE);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, CLIPPLANESCALEDPOINTS);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, CLIPTLVERTS);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, TSSARGTEMP);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, BLENDOP);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, NULLREFERENCE);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, INDEPENDENTWRITEMASKS);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, PERSTAGECONSTANT);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, POSTBLENDSRGBCONVERT);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, FOGANDSPECULARALPHA);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, SEPARATEALPHABLEND);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, MRTINDEPENDENTBITDEPTHS);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, MRTPOSTPIXELSHADERBLENDING);
    CAP_CASE(PrimitiveMiscCaps, D3DPMISCCAPS, FOGVERTEXCLAMPED);

    C2S("\nRasterCaps:");
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, ANISOTROPY);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, COLORPERSPECTIVE);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, DITHER);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, DEPTHBIAS);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, FOGRANGE);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, FOGTABLE);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, FOGVERTEX);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, MIPMAPLODBIAS);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, MULTISAMPLE_TOGGLE);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, SCISSORTEST);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, SLOPESCALEDEPTHBIAS);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, WBUFFER);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, WFOG);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, ZBUFFERLESSHSR);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, ZFOG);
    CAP_CASE(RasterCaps, D3DPRASTERCAPS, ZTEST);

    C2S("\nZCmpCaps:");
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, ALWAYS);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, EQUAL);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, GREATER);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, GREATEREQUAL);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, LESS);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, LESSEQUAL);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, NEVER);
    CAP_CASE(ZCmpCaps, D3DPCMPCAPS, NOTEQUAL);

    C2S("\nSrcBlendCaps");
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, BLENDFACTOR);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, BOTHINVSRCALPHA);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, BOTHSRCALPHA);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, DESTALPHA);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, DESTCOLOR);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, INVDESTALPHA);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, INVDESTCOLOR);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, INVSRCALPHA);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, INVSRCCOLOR);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, INVSRCCOLOR2);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, ONE);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, SRCALPHA);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, SRCALPHASAT);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, SRCCOLOR);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, SRCCOLOR2);
    CAP_CASE(SrcBlendCaps, D3DPBLENDCAPS, ZERO);

    C2S("\nDestBlendCaps");
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, BLENDFACTOR);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, BOTHINVSRCALPHA);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, BOTHSRCALPHA);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, DESTALPHA);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, DESTCOLOR);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, INVDESTALPHA);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, INVDESTCOLOR);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, INVSRCALPHA);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, INVSRCCOLOR);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, INVSRCCOLOR2);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, ONE);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, SRCALPHA);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, SRCALPHASAT);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, SRCCOLOR);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, SRCCOLOR2);
    CAP_CASE(DestBlendCaps, D3DPBLENDCAPS, ZERO);

    C2S("\nAlphaCmpCaps:");
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, ALWAYS);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, EQUAL);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, GREATER);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, GREATEREQUAL);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, LESS);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, LESSEQUAL);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, NEVER);
    CAP_CASE(AlphaCmpCaps, D3DPCMPCAPS, NOTEQUAL);

    C2S("\nShadeCaps:");
    CAP_CASE(ShadeCaps, D3DPSHADECAPS, ALPHAGOURAUDBLEND);
    CAP_CASE(ShadeCaps, D3DPSHADECAPS, COLORGOURAUDRGB);
    CAP_CASE(ShadeCaps, D3DPSHADECAPS, FOGGOURAUD);
    CAP_CASE(ShadeCaps, D3DPSHADECAPS, SPECULARGOURAUDRGB);

    C2S("\nTextureCaps:");
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, ALPHA);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, ALPHAPALETTE);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, CUBEMAP);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, CUBEMAP_POW2);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, MIPCUBEMAP);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, MIPMAP);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, MIPVOLUMEMAP);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, NONPOW2CONDITIONAL);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, NOPROJECTEDBUMPENV);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, PERSPECTIVE);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, POW2);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, PROJECTED);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, SQUAREONLY);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, TEXREPEATNOTSCALEDBYSIZE);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, VOLUMEMAP);
    CAP_CASE(TextureCaps, D3DPTEXTURECAPS, VOLUMEMAP_POW2);

    C2S("\nTextureFilterCaps:");
 /* CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, CONVOLUTIONMONO); */
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MAGFPOINT);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MAGFLINEAR);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MAGFANISOTROPIC);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MAGFPYRAMIDALQUAD);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MAGFGAUSSIANQUAD);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MINFPOINT);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MINFLINEAR);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MINFANISOTROPIC);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MINFPYRAMIDALQUAD);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MINFGAUSSIANQUAD);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MIPFPOINT);
    CAP_CASE(TextureFilterCaps, D3DPTFILTERCAPS, MIPFLINEAR);

    C2S("\nCubeTextureFilterCaps:");
 /* CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, CONVOLUTIONMONO); */
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MAGFPOINT);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MAGFLINEAR);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MAGFANISOTROPIC);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MAGFPYRAMIDALQUAD);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MAGFGAUSSIANQUAD);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MINFPOINT);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MINFLINEAR);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MINFANISOTROPIC);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MINFPYRAMIDALQUAD);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MINFGAUSSIANQUAD);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MIPFPOINT);
    CAP_CASE(CubeTextureFilterCaps, D3DPTFILTERCAPS, MIPFLINEAR);

    C2S("\nVolumeTextureFilterCaps:");
 /* CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, CONVOLUTIONMONO); */
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MAGFPOINT);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MAGFLINEAR);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MAGFANISOTROPIC);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MAGFPYRAMIDALQUAD);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MAGFGAUSSIANQUAD);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MINFPOINT);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MINFLINEAR);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MINFANISOTROPIC);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MINFPYRAMIDALQUAD);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MINFGAUSSIANQUAD);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MIPFPOINT);
    CAP_CASE(VolumeTextureFilterCaps, D3DPTFILTERCAPS, MIPFLINEAR);

    C2S("\nTextureAddressCaps:");
    CAP_CASE(TextureAddressCaps, D3DPTADDRESSCAPS, BORDER);
    CAP_CASE(TextureAddressCaps, D3DPTADDRESSCAPS, CLAMP);
    CAP_CASE(TextureAddressCaps, D3DPTADDRESSCAPS, INDEPENDENTUV);
    CAP_CASE(TextureAddressCaps, D3DPTADDRESSCAPS, MIRROR);
    CAP_CASE(TextureAddressCaps, D3DPTADDRESSCAPS, MIRRORONCE);
    CAP_CASE(TextureAddressCaps, D3DPTADDRESSCAPS, WRAP);

    C2S("\nVolumeTextureAddressCaps:");
    CAP_CASE(VolumeTextureAddressCaps, D3DPTADDRESSCAPS, BORDER);
    CAP_CASE(VolumeTextureAddressCaps, D3DPTADDRESSCAPS, CLAMP);
    CAP_CASE(VolumeTextureAddressCaps, D3DPTADDRESSCAPS, INDEPENDENTUV);
    CAP_CASE(VolumeTextureAddressCaps, D3DPTADDRESSCAPS, MIRROR);
    CAP_CASE(VolumeTextureAddressCaps, D3DPTADDRESSCAPS, MIRRORONCE);
    CAP_CASE(VolumeTextureAddressCaps, D3DPTADDRESSCAPS, WRAP);

    C2S("\nLineCaps:");
    CAP_CASE(LineCaps, D3DLINECAPS, ALPHACMP);
    CAP_CASE(LineCaps, D3DLINECAPS, ANTIALIAS);
    CAP_CASE(LineCaps, D3DLINECAPS, BLEND);
    CAP_CASE(LineCaps, D3DLINECAPS, FOG);
    CAP_CASE(LineCaps, D3DLINECAPS, TEXTURE);
    CAP_CASE(LineCaps, D3DLINECAPS, ZTEST);

    C2S("\nMaxTextureWidth: %u", caps->MaxTextureWidth);
    C2S("\nMaxTextureHeight: %u", caps->MaxTextureHeight);
    C2S("\nMaxVolumeExtent: %u", caps->MaxVolumeExtent);
    C2S("\nMaxTextureRepeat: %u", caps->MaxTextureRepeat);
    C2S("\nMaxTextureAspectRatio: %u", caps->MaxTextureAspectRatio);
    C2S("\nMaxAnisotropy: %u", caps->MaxAnisotropy);
    C2S("\nMaxVertexW: %f", caps->MaxVertexW);

    C2S("\nGuardBandLef,Top,Right,Bottom: %f %f %f %f",
        caps->GuardBandLeft, caps->GuardBandTop,
        caps->GuardBandRight, caps->GuardBandBottom);

    C2S("\nExtentsAdjust: %f", caps->ExtentsAdjust);

    C2S("\nStencilCaps:");
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, KEEP);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, ZERO);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, REPLACE);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, INCRSAT);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, DECRSAT);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, INVERT);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, INCR);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, DECR);
    CAP_CASE(StencilCaps, D3DSTENCILCAPS, TWOSIDED);

    C2S("\nFVFCaps:");
    CAP_CASE(FVFCaps, D3DFVFCAPS, DONOTSTRIPELEMENTS);
    CAP_CASE(FVFCaps, D3DFVFCAPS, PSIZE);
    CAP_CASE(FVFCaps, D3DFVFCAPS, TEXCOORDCOUNTMASK);

    C2S("\nTextureOpCaps:");
    CAP_CASE(TextureOpCaps, D3DTEXOPCAPS, ADD);
    CAP_CASE(TextureOpCaps, D3DTEXOPCAPS, ADDSIGNED);
    C2S(" ...");

    C2S("\nMaxTextureBlendStages: %u", caps->MaxTextureBlendStages);
    C2S("\nMaxSimultaneousTextures: %u", caps->MaxTextureBlendStages);

    C2S("\nVertexProcessingCaps:");
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, DIRECTIONALLIGHTS);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, LOCALVIEWER);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, MATERIALSOURCE7);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, NO_TEXGEN_NONLOCALVIEWER);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, POSITIONALLIGHTS);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, TEXGEN);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, TEXGEN_SPHEREMAP);
    CAP_CASE(VertexProcessingCaps, D3DVTXPCAPS, TWEENING);

    C2S("\nMaxActiveLights: %u", caps->MaxActiveLights);
    C2S("\nMaxUserClipPlanes: %u", caps->MaxUserClipPlanes);
    C2S("\nMaxVertexBlendMatrices: %u", caps->MaxVertexBlendMatrices);
    C2S("\nMaxVertexBlendMatrixIndex: %u", caps->MaxVertexBlendMatrixIndex);
    C2S("\nMaxPointSize: %f", caps->MaxPointSize);
    C2S("\nMaxPrimitiveCount: 0x%x", caps->MaxPrimitiveCount);
    C2S("\nMaxVertexIndex: 0x%x", caps->MaxVertexIndex);
    C2S("\nMaxStreams: %u", caps->MaxStreams);
    C2S("\nMaxStreamStride: 0x%x", caps->MaxStreamStride);

    C2S("\nVertexShaderVersion: %08x", caps->VertexShaderVersion);
    C2S("\nMaxVertexShaderConst: %u", caps->MaxVertexShaderConst);
    C2S("\nPixelShaderVersion: %08x", caps->PixelShaderVersion);
    C2S("\nPixelShader1xMaxValue: %f", caps->PixelShader1xMaxValue);

    DBG_FLAG(ch, "D3DCAPS9(%p) part 1:\n%s\n", caps, s);
    p = 0;

    C2S("DevCaps2:");
    CAP_CASE(DevCaps2, D3DDEVCAPS2, ADAPTIVETESSRTPATCH);
    CAP_CASE(DevCaps2, D3DDEVCAPS2, ADAPTIVETESSNPATCH);
    CAP_CASE(DevCaps2, D3DDEVCAPS2, CAN_STRETCHRECT_FROM_TEXTURES);
    CAP_CASE(DevCaps2, D3DDEVCAPS2, DMAPNPATCH);
    CAP_CASE(DevCaps2, D3DDEVCAPS2, PRESAMPLEDDMAPNPATCH);
    CAP_CASE(DevCaps2, D3DDEVCAPS2, STREAMOFFSET);
    CAP_CASE(DevCaps2, D3DDEVCAPS2, VERTEXELEMENTSCANSHARESTREAMOFFSET);

    C2S("\nMasterAdapterOrdinal: %u", caps->MasterAdapterOrdinal);
    C2S("\nAdapterOrdinalInGroup: %u", caps->AdapterOrdinalInGroup);
    C2S("\nNumberOfAdaptersInGroup: %u", caps->NumberOfAdaptersInGroup);

    C2S("\nDeclTypes:");
    CAP_CASE(DeclTypes, D3DDTCAPS, UBYTE4);
    CAP_CASE(DeclTypes, D3DDTCAPS, UBYTE4N);
    CAP_CASE(DeclTypes, D3DDTCAPS, SHORT2N);
    CAP_CASE(DeclTypes, D3DDTCAPS, SHORT4N);
    CAP_CASE(DeclTypes, D3DDTCAPS, USHORT2N);
    CAP_CASE(DeclTypes, D3DDTCAPS, USHORT4N);
    CAP_CASE(DeclTypes, D3DDTCAPS, UDEC3);
    CAP_CASE(DeclTypes, D3DDTCAPS, DEC3N);
    CAP_CASE(DeclTypes, D3DDTCAPS, FLOAT16_2);
    CAP_CASE(DeclTypes, D3DDTCAPS, FLOAT16_4);

    C2S("\nNumSimultaneousRTs: %u", caps->NumSimultaneousRTs);

    C2S("\nStretchRectFilterCaps:");
    CAP_CASE(StretchRectFilterCaps, D3DPTFILTERCAPS, MINFPOINT);
    CAP_CASE(StretchRectFilterCaps, D3DPTFILTERCAPS, MINFLINEAR);
    CAP_CASE(StretchRectFilterCaps, D3DPTFILTERCAPS, MAGFPOINT);
    CAP_CASE(StretchRectFilterCaps, D3DPTFILTERCAPS, MAGFLINEAR);

    C2S("\nVS20Caps.Caps: Predication=%s", caps->VS20Caps.Caps ? "yes" : "no");
    C2S("\nVS20Caps.DynamicFlowControlDepth: %u", caps->VS20Caps.DynamicFlowControlDepth);
    C2S("\nVS20Caps.NumTemps: %u", caps->VS20Caps.NumTemps);
    C2S("\nVS20Caps.StaticFlowControlDepth: %u", caps->VS20Caps.StaticFlowControlDepth);

    C2S("\nPS20Caps.Caps: Predication=%s", caps->VS20Caps.Caps ? "yes" : "no");
    C2S("\nPS20Caps.DynamicFlowControlDepth: %u", caps->PS20Caps.DynamicFlowControlDepth);
    C2S("\nPS20Caps.NumTemps: %u", caps->PS20Caps.NumTemps);
    C2S("\nPS20Caps.StaticFlowControlDepth: %u", caps->PS20Caps.StaticFlowControlDepth);
    C2S("\nPS20Caps.NumInstructionSlots: %u", caps->PS20Caps.NumInstructionSlots);

    C2S("\nVertexTextureFilterCaps");
 /* CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, CONVOLUTIONMONO); */
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MAGFPOINT);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MAGFLINEAR);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MAGFANISOTROPIC);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MAGFPYRAMIDALQUAD);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MAGFGAUSSIANQUAD);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MINFPOINT);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MINFLINEAR);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MINFANISOTROPIC);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MINFPYRAMIDALQUAD);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MINFGAUSSIANQUAD);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MIPFPOINT);
    CAP_CASE(VertexTextureFilterCaps, D3DPTFILTERCAPS, MIPFLINEAR);

    C2S("\nMaxVShaderInstructionsExecuted: %u", caps->MaxVShaderInstructionsExecuted);
    C2S("\nMaxPShaderInstructionsExecuted: %u", caps->MaxPShaderInstructionsExecuted);
    C2S("\nMaxVertexShader30InstructionSlots: %u >= 512", caps->MaxVertexShader30InstructionSlots);
    C2S("\nMaxPixelShader30InstructionSlots: %u >= 512", caps->MaxPixelShader30InstructionSlots);

    DBG_FLAG(ch, "D3DCAPS9(%p) part 2:\n%s\n", caps, s);

    FREE(s);
}

#endif /* DEBUG || !NDEBUG */
