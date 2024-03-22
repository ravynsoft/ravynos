#ifndef _glx_lib_glvnd_h_
#define _glx_lib_glvnd_h_

typedef struct __GLXapiExportsRec __GLXapiExports;

extern const __GLXapiExports *__glXGLVNDAPIExports;

extern const int DI_FUNCTION_COUNT;

extern const void * const __glXDispatchFunctions[];
extern int __glXDispatchTableIndices[];
extern const char * const __glXDispatchTableStrings[];

#endif
