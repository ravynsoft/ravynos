
#ifdef __clang__
#define CGL_DLLEXPORT
#define CGL_DLLIMPORT
#else
#define CGL_DLLEXPORT __declspec(dllexport)
#define CGL_DLLIMPORT __declspec(dllimport)
#endif

#ifdef __cplusplus

#if defined(__WIN32__)
#if defined(CGL_INSIDE_BUILD)
#define CGL_EXPORT extern "C" CGL_DLLEXPORT
#else
#define CGL_EXPORT extern "C" CGL_DLLIMPORT
#endif
#else
#define CGL_EXPORT extern "C"
#endif

#else

#if defined(__WIN32__)
#if defined(CGL_INSIDE_BUILD)
#define CGL_EXPORT CGL_DLLEXPORT extern
#else
#define CGL_EXPORT CGL_DLLIMPORT extern
#endif
#else
#define CGL_EXPORT extern
#endif

#endif // __cplusplus

#import <stdint.h>

typedef enum {
    kCGLNoError = 0,
    kCGLBadAttribute = 10000,
    kCGLBadProperty = 10001,
    kCGLBadPixelFormat = 10002,
    kCGLBadRendererInfo = 10003,
    kCGLBadContext = 10004,
    kCGLBadDrawable = 10005,
    kCGLBadDisplay = 10006,
    kCGLBadState = 10007,
    kCGLBadValue = 10008,
    kCGLBadMatch = 10009,
    kCGLBadEnumeration = 10010,
    kCGLBadOffScreen = 10011,
    kCGLBadFullScreen = 10012,
    kCGLBadWindow = 10013,
    kCGLBadAddress = 10014,
    kCGLBadCodeModule = 10015,
    kCGLBadAlloc = 10016,
    kCGLBadConnection = 10017,
} CGLError;

enum {
    kCGLPFAAllRenderers = 1,
    kCGLPFADoubleBuffer = 5,
    kCGLPFAStereo = 6,
    kCGLPFAAuxBuffers = 7,
    kCGLPFAColorSize = 8,
    kCGLPFAAlphaSize = 11,
    kCGLPFADepthSize = 12,
    kCGLPFAStencilSize = 13,
    kCGLPFAAccumSize = 14,
    kCGLPFAMinimumPolicy = 51,
    kCGLPFAMaximumPolicy = 52,
    kCGLPFAOffScreen = 53,
    kCGLPFAFullScreen = 54,
    kCGLPFASampleBuffers = 55,
    kCGLPFASamples = 56,
    kCGLPFAAuxDepthStencil = 57,
    kCGLPFAColorFloat = 58,
    kCGLPFAMultisample = 59,
    kCGLPFASupersample = 60,
    kCGLPFASampleAlpha = 61,
    kCGLPFARendererID = 70,
    kCGLPFASingleRenderer = 71,
    kCGLPFANoRecovery = 72,
    kCGLPFAAccelerated = 73,
    kCGLPFAClosestPolicy = 74,
    kCGLPFARobust = 75,
    kCGLPFABackingStore = 76,
    kCGLPFAMPSafe = 78,
    kCGLPFAWindow = 80,
    kCGLPFAMultiScreen = 81,
    kCGLPFACompliant = 83,
    kCGLPFADisplayMask = 84,
    kCGLPFAPBuffer = 90,
    kCGLPFARemotePBuffer = 91,
    kCGLPFAAllowOfflineRenderers = 96,
    kCGLPFAAcceleratedCompute = 97,
    kCGLPFAVirtualScreenCount = 128,
};

typedef uint32_t CGLPixelFormatAttribute;

typedef enum {
    kCGLCPSwapInterval = 222,
    kCGLCPSurfaceOpacity = 236,
    kCGLCPSurfaceBackingSize = 304,

    // internal, do not use
    kCGLCPSurfaceFrame = 499,
    kCGLCPSurfaceIsChildWindow = 500,
    kCGLCPWindowNumber = 501,
    kCGLCPOverlayPointer = 502,

    kCGLCPSurfaceBackingOrigin = 503,
    kCGLCPSurfaceWindowNumber = 504,
    kCGLCPSurfaceHidden = 505,

} CGLContextParameter;

typedef struct _CGLContextObj *CGLContextObj;
typedef struct _CGLPixelFormatObj *CGLPixelFormatObj;
typedef struct _CGLPBufferObj *CGLPBufferObj;
