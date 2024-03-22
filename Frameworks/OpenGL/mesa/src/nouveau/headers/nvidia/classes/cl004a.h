/*******************************************************************************
    Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#ifndef _cl004a_h_
#define _cl004a_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV04_GDI_RECTANGLE_TEXT                                   (0x0000004A)
/* NvNotification[] elements */
#define NV04A_NOTIFIERS_NOTIFY                                     (0)
#define NV04A_NOTIFIERS_MAXCOUNT								   (1)
/* NvNotification[] fields and values */
#define NV04A_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV04A_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV04A_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV04A_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV04A_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV04A_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* memory data structures */
typedef struct {                /* start of data structure          0000-    */
   NvV32 size;                  /* height_width U16_U16 in pixels      0-   3*/
   NvV32 monochrome[1];         /* 32 monochrome pixels per write      4-   7*/
} Nv04aCharacter8[];            /* end of data structure                -0007*/
typedef struct {                /* start of data structure          0000-    */
  NvV32 size;                   /* height_width U16_U16 in pixels      0-   3*/
  NvV32 monochrome[3];          /* 32 monochrome pixels per write      4-   f*/
} Nv04aCharacter16[];           /* end of data structure                -000f*/
typedef struct {                /* start of data structure          0000-    */
  NvV32 size;                   /* height_width U16_U16 in pixels     00-  03*/
  NvV32 monochrome[7];          /* 32 monochrome pixels per write     04-  1f*/
} Nv04aCharacter32[];           /* end of data structure                -001f*/
typedef struct {                /* start of data structure          0000-    */
  NvV32 size;                   /* height_width U16_U16 in pixels     00-  03*/
  NvV32 monochrome[15];         /* 32 monochrome pixels per write     04-  3f*/
} Nv04aCharacter64[];           /* end of data structure                -003f*/
typedef struct {                /* start of data structure          0000-    */
  NvV32 size;                   /* height_width U16_U16 in pixels     00-  03*/
  NvV32 monochrome[31];         /* 32 monochrome pixels per write     04-  7f*/
} Nv04aCharacter128[];          /* end of data structure                -007f*/
typedef struct {                /* start of data structure          0000-    */
  NvV32 size;                   /* height_width U16_U16 in pixels     00-  03*/
  NvV32 monochrome[63];         /* 32 monochrome pixels per write     04-  ff*/
} Nv04aCharacter256[];          /* end of data structure                -00ff*/
typedef struct {                /* start of data structure          0000-    */
  NvV32 size;                   /* height_width U16_U16 in pixels    000- 003*/
  NvV32 monochrome[127];        /* 32 monochrome pixels per write    004- 1ff*/
} Nv04aCharacter512[];          /* end of data structure                -01ff*/
/* pio method data structure */
typedef volatile struct _cl004a_tag0 {
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV04A_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextDmaFonts;      /* NV01_CONTEXT_DMA                 0184-0187*/
 NvV32 SetContextPattern;       /* NV04_CONTEXT_PATTERN             0188-018b*/
 NvV32 SetContextRop;           /* NV03_CONTEXT_ROP                 018c-018f*/
 NvV32 SetContextBeta1;         /* NV01_CONTEXT_BETA                0190-0193*/
 NvV32 SetContextBeta4;         /* NV04_CONTEXT_BETA                0194-0197*/
 NvV32 SetContextSurface;       /* NV04_CONTEXT_SURFACES_2D         0198-019b*/
 NvV32 Reserved01[0x058];
 NvV32 SetOperation;            /* NV04A_SET_OPERATION_*            02fc-02ff*/
 NvV32 SetColorFormat;          /* NV04A_SET_COLOR_FORMAT_*         0300-0303*/
 NvV32 SetMonochromeFormat;     /* NV04A_SET_MONOCHROME_FORMAT_*    0304-0307*/
 NvV32 Reserved02[0x03D];
 NvV32 Color1A;                 /* rectangle color                  03fc-03ff*/
 struct {                       /* start aliased methods in array   0400-    */
  NvV32 point;                  /* x_y S16_S16 in pixels               0-   3*/
  NvV32 size;                   /* width_height U16_U16 in pixels      4-   7*/
 } UnclippedRectangle[32];      /* end of aliased methods in array      -04ff*/
 NvV32 Reserved03[0x03D];
 NvV32 ClipPoint0B;             /* top_left S16_S16 in pixels       05f4-05f7*/
 NvV32 ClipPoint1B;             /* bottom_right S16_S16 in pixels   05f8-05fb*/
 NvV32 Color1B;                 /* rectangle color                  05fc-05ff*/
 struct {                       /* start aliased methods in array   0600-    */
  NvV32 point0;                 /* top_left S16_S16 in pixels          0-   3*/
  NvV32 point1;                 /* bottom_right S16_S16 in pixels      4-   7*/
 } ClippedRectangle[32];        /* end of aliased methods in array      -06ff*/
 NvV32 Reserved04[0x03B];
 NvV32 ClipPoint0C;             /* top_left S16_S16 in pixels       07ec-07ef*/
 NvV32 ClipPoint1C;             /* bottom_right S16_S16 in pixe     07f0-07f3*/
 NvV32 Color1C;                 /* color of 1 pixels                07f4-07f7*/
 NvV32 SizeC;                   /* height_width U16_U16 in pixels   07f8-07fb*/
 NvV32 PointC;                  /* y_x S16_S16 in pixels            07fc-07ff*/
 NvV32 MonochromeColor1C[128];  /* 32 monochrome pixels per write   0800-09ff*/
 NvV32 Reserved05[0x079];
 NvV32 ClipPoint0E;             /* top_left S16_S16 in pixels       0be4-0be7*/
 NvV32 ClipPoint1E;             /* bottom_right S16_S16 in pixels   0be8-0beb*/
 NvV32 Color0E;                 /* color of 0 pixels                0bec-0bef*/
 NvV32 Color1E;                 /* color of 1 pixels                0bf0-0bf3*/
 NvV32 SizeInE;                 /* height_width U16_U16 in pixels   0bf4-0bf7*/
 NvV32 SizeOutE;                /* height_width U16_U16 in pixels   0bf8-0bfb*/
 NvV32 PointE;                  /* y_x S16_S16 in pixels            0bfc-0bff*/
 NvV32 MonochromeColor01E[128]; /* 32 monochrome pixels per write   0c00-0dff*/
 NvV32 Reserved06[0x07C];
 NvV32 FontF;                   /* pitch_offset V4_U28              0ff0-0ff3*/
 NvV32 ClipPoint0F;             /* top_left S16_S16 in pixels       0ff4-0ff7*/
 NvV32 ClipPoint1F;             /* bottom_right S16_S16 in pixels   0ff8-0ffb*/
 NvV32 Color1F;                 /* color of 1 pixels                0ffc-0fff*/
 NvV32 CharacterColor1F[256];   /* y_x_index S12_S12_U8             1000-13ff*/
 NvV32 Reserved07[0x0FC];
 NvV32 FontG;                   /* pitch_offset V4_U28              17f0-17f3*/
 NvV32 ClipPoint0G;             /* top_left S16_S16 in pixels       17f4-17f7*/
 NvV32 ClipPoint1G;             /* bottom_right S16_S16 in pixels   17f8-17fb*/
 NvV32 Color1G;                 /* color of 1 pixels                17fc-17ff*/
 struct {                       /* start aliased methods in array   1800-    */
  NvV32 point;                  /* y_x S16_S16 in pixels               0-   3*/
  NvU32 index;                  /* 0<=index<=65525                     4-   7*/
 } CharacterColor1G[256];       /* end of aliased methods in array      -1fff*/
} Nv04aTypedef, Nv04GdiRectangleText;
#define NV04A_TYPEDEF                                      Nv04GdiRectangleText
/* dma method offsets, fields, and values */
#define NV04A_SET_OBJECT                                           (0x00000000)
#define NV04A_NO_OPERATION                                         (0x00000100)
#define NV04A_NOTIFY                                               (0x00000104)
#define NV04A_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV04A_NOTIFY_WRITE_THEN_AWAKEN                             (0x00000001)
#define NV04A_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV04A_SET_CONTEXT_DMA_FONTS                                (0x00000184)
#define NV04A_SET_CONTEXT_PATTERN                                  (0x00000188)
#define NV04A_SET_CONTEXT_ROP                                      (0x0000018C)
#define NV04A_SET_CONTEXT_BETA1                                    (0x00000190)
#define NV04A_SET_CONTEXT_BETA4                                    (0x00000194)
#define NV04A_SET_CONTEXT_SURFACE                                  (0x00000198)
#define NV04A_SET_OPERATION                                        (0x000002FC)
#define NV04A_SET_OPERATION_SRCCOPY_AND                            (0x00000000)
#define NV04A_SET_OPERATION_ROP_AND                                (0x00000001)
#define NV04A_SET_OPERATION_BLEND_AND                              (0x00000002)
#define NV04A_SET_OPERATION_SRCCOPY                                (0x00000003)
#define NV04A_SET_OPERATION_SRCCOPY_PREMULT                        (0x00000004)
#define NV04A_SET_OPERATION_BLEND_PREMULT                          (0x00000005)
#define NV04A_SET_COLOR_FORMAT                                     (0x00000300)
#define NV04A_SET_COLOR_FORMAT_LE_X16R5G6B5                        (0x00000001)
#define NV04A_SET_COLOR_FORMAT_LE_X17R5G5B5                        (0x00000002)
#define NV04A_SET_COLOR_FORMAT_LE_X8R8G8B8                         (0x00000003)
#define NV04A_SET_MONOCHROME_FORMAT                                (0x00000304)
#define NV04A_SET_MONOCHROME_FORMAT_CGA6_M1                        (0x00000001)
#define NV04A_SET_MONOCHROME_FORMAT_LE_M1                          (0x00000002)
#define NV04A_COLOR1_A                                             (0x000003FC)
#define NV04A_UNCLIPPED_RECTANGLE(a)                               (0x00000400\
                                                                   +(a)*0x0008)
#define NV04A_UNCLIPPED_RECTANGLE_POINT(a)                         (0x00000400\
                                                                   +(a)*0x0008)
#define NV04A_UNCLIPPED_RECTANGLE_POINT_Y                          15:0
#define NV04A_UNCLIPPED_RECTANGLE_POINT_X                          31:16
#define NV04A_UNCLIPPED_RECTANGLE_SIZE(a)                          (0x00000404\
                                                                   +(a)*0x0008)
#define NV04A_UNCLIPPED_RECTANGLE_SIZE_HEIGHT                      15:0
#define NV04A_UNCLIPPED_RECTANGLE_SIZE_WIDTH                       31:16
#define NV04A_CLIP_POINT0_B                                        (0x000005F4)
#define NV04A_CLIP_POINT0_B_LEFT                                   15:0
#define NV04A_CLIP_POINT0_B_TOP                                    31:16
#define NV04A_CLIP_POINT1_B                                        (0x000005F8)
#define NV04A_CLIP_POINT1_B_RIGHT                                  15:0
#define NV04A_CLIP_POINT1_B_BOTTOM                                 31:16
#define NV04A_COLOR1_B                                             (0x000005FC)
#define NV04A_CLIPPED_RECTANGLE(a)                                 (0x00000600\
                                                                   +(a)*0x0008)
#define NV04A_CLIPPED_RECTANGLE_POINT_0(a)                         (0x00000600\
                                                                   +(a)*0x0008)
#define NV04A_CLIPPED_RECTANGLE_POINT_0_LEFT                       15:0
#define NV04A_CLIPPED_RECTANGLE_POINT_0_TOP                        31:16
#define NV04A_CLIPPED_RECTANGLE_POINT_1(a)                         (0x00000604\
                                                                   +(a)*0x0008)
#define NV04A_CLIPPED_RECTANGLE_POINT_1_RIGHT                      15:0
#define NV04A_CLIPPED_RECTANGLE_POINT_1_BOTTOM                     31:16
#define NV04A_CLIP_POINT0_C                                        (0x000007EC)
#define NV04A_CLIP_POINT0_C_LEFT                                   15:0
#define NV04A_CLIP_POINT0_C_TOP                                    31:16
#define NV04A_CLIP_POINT1_C                                        (0x000007F0)
#define NV04A_CLIP_POINT1_C_RIGHT                                  15:0
#define NV04A_CLIP_POINT1_C_BOTTOM                                 31:16
#define NV04A_COLOR1_C                                             (0x000007F4)
#define NV04A_SIZE_C                                               (0x000007F8)
#define NV04A_SIZE_C_WIDTH                                         15:0
#define NV04A_SIZE_C_HEIGHT                                        31:16
#define NV04A_POINT_C                                              (0x000007FC)
#define NV04A_POINT_C_X                                            15:0
#define NV04A_POINT_C_Y                                            31:16
#define NV04A_MONOCHROME_COLOR1_C(a)                               (0x00000800\
                                                                   +(a)*0x0004)
#define NV04A_CLIP_POINT0_E                                        (0x00000BE4)
#define NV04A_CLIP_POINT0_E_LEFT                                   15:0
#define NV04A_CLIP_POINT0_E_TOP                                    31:16
#define NV04A_CLIP_POINT1_E                                        (0x00000BE8)
#define NV04A_CLIP_POINT1_E_RIGHT                                  15:0
#define NV04A_CLIP_POINT1_E_BOTTOM                                 31:16
#define NV04A_COLOR0_E                                             (0x00000BEC)
#define NV04A_COLOR1_E                                             (0x00000BF0)
#define NV04A_SIZE_IN_E                                            (0x00000BF4)
#define NV04A_SIZE_IN_E_WIDTH                                      15:0
#define NV04A_SIZE_IN_E_HEIGHT                                     31:16
#define NV04A_SIZE_OUT_E                                           (0x00000BF8)
#define NV04A_SIZE_OUT_E_WIDTH                                     15:0
#define NV04A_SIZE_OUT_E_HEIGHT                                    31:16
#define NV04A_POINT_E                                              (0x00000BFC)
#define NV04A_POINT_E_X                                            15:0
#define NV04A_POINT_E_Y                                            31:16
#define NV04A_MONOCHROME_COLOR01_E(a)                              (0x00000C00\
                                                                   +(a)*0x0004)
#define NV04A_FONT_F                                               (0x00000FF0)
#define NV04A_FONT_F_OFFSET                                        27:0
#define NV04A_FONT_F_PITCH                                         31:28
#define NV04A_FONT_F_PITCH_8                                       (0x00000003)
#define NV04A_FONT_F_PITCH_16                                      (0x00000004)
#define NV04A_FONT_F_PITCH_32                                      (0x00000005)
#define NV04A_FONT_F_PITCH_64                                      (0x00000006)
#define NV04A_FONT_F_PITCH_128                                     (0x00000007)
#define NV04A_FONT_F_PITCH_256                                     (0x00000008)
#define NV04A_FONT_F_PITCH_512                                     (0x00000009)
#define NV04A_CLIP_POINT0_F                                        (0x00000FF4)
#define NV04A_CLIP_POINT0_F_LEFT                                   15:0
#define NV04A_CLIP_POINT0_F_TOP                                    31:16
#define NV04A_CLIP_POINT1_F                                        (0x00000FF8)
#define NV04A_CLIP_POINT1_F_RIGHT                                  15:0
#define NV04A_CLIP_POINT1_F_BOTTOM                                 31:16
#define NV04A_COLOR1_F                                             (0x00000FFC)
#define NV04A_CHARACTER_COLOR1_F(a)                                (0x00001000\
                                                                   +(a)*0x0004)
#define NV04A_CHARACTER_COLOR1_F_INDEX                             7:0
#define NV04A_CHARACTER_COLOR1_F_X                                 19:8
#define NV04A_CHARACTER_COLOR1_F_Y                                 31:20
#define NV04A_FONT_G                                               (0x000017F0)
#define NV04A_FONT_G_OFFSET                                        27:0
#define NV04A_FONT_G_PITCH                                         31:28
#define NV04A_FONT_G_PITCH_8                                       (0x00000003)
#define NV04A_FONT_G_PITCH_16                                      (0x00000004)
#define NV04A_FONT_G_PITCH_32                                      (0x00000005)
#define NV04A_FONT_G_PITCH_64                                      (0x00000006)
#define NV04A_FONT_G_PITCH_128                                     (0x00000007)
#define NV04A_FONT_G_PITCH_256                                     (0x00000008)
#define NV04A_FONT_G_PITCH_512                                     (0x00000009)
#define NV04A_CLIP_POINT0_G                                        (0x000017F4)
#define NV04A_CLIP_POINT0_G_LEFT                                   15:0
#define NV04A_CLIP_POINT0_G_TOP                                    31:16
#define NV04A_CLIP_POINT1_G                                        (0x000017F8)
#define NV04A_CLIP_POINT1_G_RIGHT                                  15:0
#define NV04A_CLIP_POINT1_G_BOTTOM                                 31:16
#define NV04A_COLOR1_G                                             (0x000017FC)
#define NV04A_CHARACTER_COLOR1_G(a)                                (0x00001800\
                                                                   +(a)*0x0008)
#define NV04A_CHARACTER_COLOR1_G_POINT(a)                          (0x00001800\
                                                                   +(a)*0x0008)
#define NV04A_CHARACTER_COLOR1_G_POINT_X                           15:0
#define NV04A_CHARACTER_COLOR1_G_POINT_Y                           31:16
#define NV04A_CHARACTER_COLOR1_G_INDEX(a)                          (0x00001804\
                                                                   +(a)*0x0008)
/* obsolete stuff */
#define NV4_GDI_RECTANGLE_TEXT                                     (0x0000004A)
#define Nv4GdiRectangleText                                Nv04GdiRectangleText
#define nv4GdiRectangleText                                Nv04GdiRectangleText
#define nv04GdiRectangleText                               Nv04GdiRectangleText

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl004a_h_ */
