//
// Copyright 2013 Francisco Jerez
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include "api/dispatch.hpp"

namespace clover {
   const cl_icd_dispatch _dispatch = {
      // OpenCL 1.0
      clGetPlatformIDs,
      GetPlatformInfo,
      clGetDeviceIDs,
      clGetDeviceInfo,
      clCreateContext,
      clCreateContextFromType,
      clRetainContext,
      clReleaseContext,
      clGetContextInfo,
      clCreateCommandQueue,
      clRetainCommandQueue,
      clReleaseCommandQueue,
      clGetCommandQueueInfo,
      NULL, // clSetCommandQueueProperty
      clCreateBuffer,
      clCreateImage2D,
      clCreateImage3D,
      clRetainMemObject,
      clReleaseMemObject,
      clGetSupportedImageFormats,
      clGetMemObjectInfo,
      clGetImageInfo,
      clCreateSampler,
      clRetainSampler,
      clReleaseSampler,
      clGetSamplerInfo,
      clCreateProgramWithSource,
      clCreateProgramWithBinary,
      clRetainProgram,
      clReleaseProgram,
      clBuildProgram,
      clUnloadCompiler,
      clGetProgramInfo,
      clGetProgramBuildInfo,
      clCreateKernel,
      clCreateKernelsInProgram,
      clRetainKernel,
      clReleaseKernel,
      clSetKernelArg,
      clGetKernelInfo,
      clGetKernelWorkGroupInfo,
      clWaitForEvents,
      clGetEventInfo,
      clRetainEvent,
      clReleaseEvent,
      clGetEventProfilingInfo,
      clFlush,
      clFinish,
      clEnqueueReadBuffer,
      clEnqueueWriteBuffer,
      clEnqueueCopyBuffer,
      clEnqueueReadImage,
      clEnqueueWriteImage,
      clEnqueueCopyImage,
      clEnqueueCopyImageToBuffer,
      clEnqueueCopyBufferToImage,
      clEnqueueMapBuffer,
      clEnqueueMapImage,
      clEnqueueUnmapMemObject,
      clEnqueueNDRangeKernel,
      clEnqueueTask,
      clEnqueueNativeKernel,
      clEnqueueMarker,
      clEnqueueWaitForEvents,
      clEnqueueBarrier,
      GetExtensionFunctionAddress,
      NULL, // clCreateFromGLBuffer
      NULL, // clCreateFromGLTexture2D
      NULL, // clCreateFromGLTexture3D
      NULL, // clCreateFromGLRenderbuffer
      NULL, // clGetGLObjectInfo
      NULL, // clGetGLTextureInfo
      NULL, // clEnqueueAcquireGLObjects
      NULL, // clEnqueueReleaseGLObjects

      // cl_khr_d3d10_sharing
      NULL, // clGetGLContextInfoKHR
      NULL, // clGetDeviceIDsFromD3D10KHR
      NULL, // clCreateFromD3D10BufferKHR
      NULL, // clCreateFromD3D10Texture2DKHR
      NULL, // clCreateFromD3D10Texture3DKHR
      NULL, // clEnqueueAcquireD3D10ObjectsKHR
      NULL, // clEnqueueReleaseD3D10ObjectsKHR

      // OpenCL 1.1
      clSetEventCallback,
      clCreateSubBuffer,
      clSetMemObjectDestructorCallback,
      clCreateUserEvent,
      clSetUserEventStatus,
      clEnqueueReadBufferRect,
      clEnqueueWriteBufferRect,
      clEnqueueCopyBufferRect,

      // cl_ext_device_fission
      NULL, // clCreateSubDevicesEXT
      NULL, // clRetainDeviceEXT
      NULL, // clReleaseDeviceEXT

      // cl_khr_gl_event
      NULL, // clCreateEventFromGLsyncKHR

      // OpenCL 1.2
      clCreateSubDevices,
      clRetainDevice,
      clReleaseDevice,
      clCreateImage,
      clCreateProgramWithBuiltInKernels,
      clCompileProgram,
      clLinkProgram,
      clUnloadPlatformCompiler,
      clGetKernelArgInfo,
      clEnqueueFillBuffer,
      clEnqueueFillImage,
      clEnqueueMigrateMemObjects,
      clEnqueueMarkerWithWaitList,
      clEnqueueBarrierWithWaitList,
      GetExtensionFunctionAddressForPlatform,
      NULL, // clCreateFromGLTexture

      // cl_khr_d3d11_sharing
      NULL, // clGetDeviceIDsFromD3D11KHR
      NULL, // clCreateFromD3D11BufferKHR
      NULL, // clCreateFromD3D11Texture2DKHR
      NULL, // clCreateFromD3D11Texture3DKHR
      NULL, // clCreateFromDX9MediaSurfaceKHR
      NULL, // clEnqueueAcquireD3D11ObjectsKHR
      NULL, // clEnqueueReleaseD3D11ObjectsKHR

      // cl_khr_dx9_media_sharing
      NULL, // clGetDeviceIDsFromDX9MediaAdapterKHR
      NULL, // clEnqueueAcquireDX9MediaSurfacesKHR
      NULL, // clEnqueueReleaseDX9MediaSurfacesKHR

      // cl_khr_egl_image
      NULL, // clCreateFromEGLImageKHR
      NULL, // clEnqueueAcquireEGLObjectsKHR
      NULL, // clEnqueueReleaseEGLObjectsKHR

      // cl_khr_egl_event
      NULL, // clCreateEventFromEGLSyncKHR

      // OpenCL 2.0
      clCreateCommandQueueWithProperties,
      clCreatePipe,
      clGetPipeInfo,
      clSVMAlloc,
      clSVMFree,
      clEnqueueSVMFree,
      clEnqueueSVMMemcpy,
      clEnqueueSVMMemFill,
      clEnqueueSVMMap,
      clEnqueueSVMUnmap,
      NULL, // clCreateSamplerWithProperties
      clSetKernelArgSVMPointer,
      clSetKernelExecInfo,

      // cl_khr_sub_groups
      NULL, // clGetKernelSubGroupInfoKHR

      // OpenCL 2.1
      NULL, // clCloneKernel
      clCreateProgramWithIL,
      clEnqueueSVMMigrateMem,
      clGetDeviceAndHostTimer,
      clGetHostTimer,
      clGetKernelSubGroupInfo,
      clSetDefaultDeviceCommandQueue,

      // OpenCL 2.2
      clSetProgramReleaseCallback,
      clSetProgramSpecializationConstant,
      clCreateBufferWithProperties,
      clCreateImageWithProperties,
      clSetContextDestructorCallback
   };
}
