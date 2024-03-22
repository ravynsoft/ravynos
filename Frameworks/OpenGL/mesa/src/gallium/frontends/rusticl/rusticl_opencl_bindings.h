#include <CL/cl_icd.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#ifdef HAVE_X11_PLATFORM
#include <GL/glx.h>
#endif
#include "GL/mesa_glinterop.h"

#define DECL_CL_STRUCT(name) struct name { const cl_icd_dispatch *dispatch; }
DECL_CL_STRUCT(_cl_command_queue);
DECL_CL_STRUCT(_cl_context);
DECL_CL_STRUCT(_cl_device_id);
DECL_CL_STRUCT(_cl_event);
DECL_CL_STRUCT(_cl_kernel);
DECL_CL_STRUCT(_cl_mem);
DECL_CL_STRUCT(_cl_platform_id);
DECL_CL_STRUCT(_cl_program);
DECL_CL_STRUCT(_cl_sampler);
#undef DECL_CL_STRUCT
