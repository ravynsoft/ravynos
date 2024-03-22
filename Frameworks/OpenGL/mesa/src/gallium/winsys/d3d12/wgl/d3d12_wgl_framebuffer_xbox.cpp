/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_wgl_public.h"

#include <new>

#include <windows.h>
#include <wrl.h>

#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "frontend/api.h"
#include "frontend/winsys_handle.h"

#include "stw_device.h"
#include "stw_pixelformat.h"
#include "stw_winsys.h"

#include "d3d12/d3d12_format.h"
#include "d3d12/d3d12_resource.h"
#include "d3d12/d3d12_screen.h"

constexpr uint32_t num_buffers = 2;
static int current_backbuffer_index = 0;
static bool has_signaled_first_time = false;
static int cached_interval = 1;

struct d3d12_wgl_framebuffer {
   struct stw_winsys_framebuffer base;

   struct d3d12_screen *screen;
   enum pipe_format pformat;
   ID3D12Resource *images[num_buffers];
   D3D12_CPU_DESCRIPTOR_HANDLE rtvs[num_buffers];
   ID3D12DescriptorHeap *rtvHeap;
   pipe_resource *buffers[num_buffers];
};

static struct d3d12_wgl_framebuffer*
d3d12_wgl_framebuffer(struct stw_winsys_framebuffer *fb)
{
   return (struct d3d12_wgl_framebuffer *) fb;
}

static void
d3d12_wgl_framebuffer_destroy(struct stw_winsys_framebuffer *fb,
                              pipe_context *ctx)
{
   struct d3d12_wgl_framebuffer *framebuffer = d3d12_wgl_framebuffer(fb);
   struct pipe_fence_handle *fence = NULL;

   if (ctx) {
      /* Ensure all resources are flushed */
      ctx->flush(ctx, &fence, PIPE_FLUSH_HINT_FINISH);
      if (fence) {
         ctx->screen->fence_finish(ctx->screen, ctx, fence, OS_TIMEOUT_INFINITE);
         ctx->screen->fence_reference(ctx->screen, &fence, NULL);
      }
   }

   framebuffer->rtvHeap->Release();
   for (int i = 0; i < num_buffers; ++i) {
      if (framebuffer->buffers[i]) {
         d3d12_resource_release(d3d12_resource(framebuffer->buffers[i]));
         pipe_resource_reference(&framebuffer->buffers[i], NULL);
      }
   }

   delete framebuffer;
}

static void
d3d12_wgl_framebuffer_resize(stw_winsys_framebuffer *fb,
                             pipe_context *ctx,
                             pipe_resource *templ)
{
   struct d3d12_wgl_framebuffer *framebuffer = d3d12_wgl_framebuffer(fb);

   if (framebuffer->rtvHeap == NULL) {
      D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
      descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      descHeapDesc.NumDescriptors = num_buffers;
      framebuffer->screen->dev->CreateDescriptorHeap(&descHeapDesc,
                                                     IID_PPV_ARGS(&framebuffer->rtvHeap));
   }

   // Release the old images
   for (int i = 0; i < num_buffers; i++) {
      if (framebuffer->buffers[i]) {
         d3d12_resource_release(d3d12_resource(framebuffer->buffers[i]));
         pipe_resource_reference(&framebuffer->buffers[i], NULL);
      }
   }

   D3D12_HEAP_PROPERTIES heapProps = {};
   heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

   D3D12_RESOURCE_DESC resDesc;
   resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
   resDesc.Width = templ->width0;
   resDesc.Height = templ->height0;
   resDesc.Alignment = 0;
   resDesc.DepthOrArraySize = 1;
   resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
   resDesc.Format = d3d12_get_format(templ->format);
   resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   resDesc.MipLevels = 1;
   resDesc.SampleDesc.Count = 1;
   resDesc.SampleDesc.Quality = 0;

   D3D12_CLEAR_VALUE optimizedClearValue = {};
   optimizedClearValue.Format = resDesc.Format;

   D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
   rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
   rtvDesc.Format = resDesc.Format;
   rtvDesc.Texture2D.MipSlice = 0;
   rtvDesc.Texture2D.PlaneSlice = 0;

   for (int i = 0; i < num_buffers; i++) {
      if (FAILED(framebuffer->screen->dev->CreateCommittedResource(
         &heapProps,
         D3D12_HEAP_FLAG_ALLOW_DISPLAY,
         &resDesc,
         D3D12_RESOURCE_STATE_PRESENT,
         &optimizedClearValue,
         IID_PPV_ARGS(&framebuffer->images[i])
      ))) {
         assert(0);
      }

      framebuffer->rtvs[i].ptr =
         framebuffer->rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
         ((int64_t)i * framebuffer->screen->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

      framebuffer->screen->dev->CreateRenderTargetView(
         framebuffer->images[i],
         &rtvDesc,
         framebuffer->rtvs[i]
      );
   }

   framebuffer->pformat = templ->format;
}

static bool
d3d12_wgl_framebuffer_present(stw_winsys_framebuffer *fb, int interval)
{
   auto framebuffer = d3d12_wgl_framebuffer(fb);
   D3D12XBOX_PRESENT_PLANE_PARAMETERS planeParams = {};
   planeParams.Token = framebuffer->screen->frame_token;
   planeParams.ResourceCount = 1;
   planeParams.ppResources = &framebuffer->images[current_backbuffer_index];

   D3D12XBOX_PRESENT_PARAMETERS presentParams = {};
   presentParams.Flags = (interval == 0) ?
      D3D12XBOX_PRESENT_FLAG_IMMEDIATE :
      D3D12XBOX_PRESENT_FLAG_NONE;

   int clamped_interval = CLAMP(interval, 1, 4); // SetFrameIntervalX only supports values [1,4]
   if (cached_interval != clamped_interval) {
      framebuffer->screen->dev->SetFrameIntervalX(
         nullptr,
         D3D12XBOX_FRAME_INTERVAL_60_HZ,
         clamped_interval,
         D3D12XBOX_FRAME_INTERVAL_FLAG_NONE
      );
      framebuffer->screen->dev->ScheduleFrameEventX(
         D3D12XBOX_FRAME_EVENT_ORIGIN,
         0,
         nullptr,
         D3D12XBOX_SCHEDULE_FRAME_EVENT_FLAG_NONE
      );
      cached_interval = clamped_interval;
   }

   framebuffer->screen->cmdqueue->PresentX(1, &planeParams, &presentParams);

   current_backbuffer_index = !current_backbuffer_index;

   framebuffer->screen->frame_token = D3D12XBOX_FRAME_PIPELINE_TOKEN_NULL;
   framebuffer->screen->dev->WaitFrameEventX(D3D12XBOX_FRAME_EVENT_ORIGIN, INFINITE,
                                             nullptr, D3D12XBOX_WAIT_FRAME_EVENT_FLAG_NONE,
                                             &framebuffer->screen->frame_token);

   return true;
}

static struct pipe_resource*
d3d12_wgl_framebuffer_get_resource(struct stw_winsys_framebuffer *pframebuffer,
                                   st_attachment_type statt)
{
   auto framebuffer = d3d12_wgl_framebuffer(pframebuffer);
   auto pscreen = &framebuffer->screen->base;

   UINT index = current_backbuffer_index;
   if (statt == ST_ATTACHMENT_FRONT_LEFT)
      index = !index;

   if (framebuffer->buffers[index]) {
      pipe_reference(NULL, &framebuffer->buffers[index]->reference);
      return framebuffer->buffers[index];
   }

   ID3D12Resource *res = framebuffer->images[index];

   struct winsys_handle handle;
   memset(&handle, 0, sizeof(handle));
   handle.type = WINSYS_HANDLE_TYPE_D3D12_RES;
   handle.format = framebuffer->pformat;
   handle.com_obj = res;

   D3D12_RESOURCE_DESC res_desc = GetDesc(res);

   struct pipe_resource templ;
   memset(&templ, 0, sizeof(templ));
   templ.target = PIPE_TEXTURE_2D;
   templ.format = framebuffer->pformat;
   templ.width0 = res_desc.Width;
   templ.height0 = res_desc.Height;
   templ.depth0 = 1;
   templ.array_size = res_desc.DepthOrArraySize;
   templ.nr_samples = res_desc.SampleDesc.Count;
   templ.last_level = res_desc.MipLevels - 1;
   templ.bind = PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_RENDER_TARGET;
   templ.usage = PIPE_USAGE_DEFAULT;
   templ.flags = 0;

   pipe_resource_reference(&framebuffer->buffers[index],
      pscreen->resource_from_handle(pscreen, &templ, &handle,
         PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE));
   return framebuffer->buffers[index];
}

struct stw_winsys_framebuffer*
d3d12_wgl_create_framebuffer(struct pipe_screen *screen,
                             HWND hWnd,
                             int iPixelFormat)
{
   const struct stw_pixelformat_info *pfi =
      stw_pixelformat_get_info(iPixelFormat);
   if (!(pfi->pfd.dwFlags & PFD_DOUBLEBUFFER) ||
      (pfi->pfd.dwFlags & PFD_SUPPORT_GDI))
      return NULL;

   struct d3d12_wgl_framebuffer *fb = CALLOC_STRUCT(d3d12_wgl_framebuffer);
   if (!fb)
      return NULL;

   new (fb) struct d3d12_wgl_framebuffer();

   fb->screen = d3d12_screen(screen);
   fb->images[0] = NULL;
   fb->images[1] = NULL;
   fb->rtvHeap = NULL;
   fb->base.destroy = d3d12_wgl_framebuffer_destroy;
   fb->base.resize = d3d12_wgl_framebuffer_resize;
   fb->base.present = d3d12_wgl_framebuffer_present;
   fb->base.get_resource = d3d12_wgl_framebuffer_get_resource;

   // Xbox applications must manually handle Suspend/Resume events on the Command Queue.
   // To allow the application to access the queue, we store a pointer in the HWND's user data.
   SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) fb->screen->cmdqueue);

   // Schedule the frame interval and origin frame event
   fb->screen->dev->SetFrameIntervalX(
      nullptr,
      D3D12XBOX_FRAME_INTERVAL_60_HZ,
      cached_interval,
      D3D12XBOX_FRAME_INTERVAL_FLAG_NONE
   );
   fb->screen->dev->ScheduleFrameEventX(
      D3D12XBOX_FRAME_EVENT_ORIGIN,
      0,
      nullptr,
      D3D12XBOX_SCHEDULE_FRAME_EVENT_FLAG_NONE
   );

   return &fb->base;
}
