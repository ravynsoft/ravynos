/*
   NSDPSContextOps - Translate method calls to PS ops.

   Copyright (C) 1999 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: Apr 1999

   This file is part of the GNU Objective C User Interface Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <math.h>

#ifdef HAVE_WRASTER_H
#include "wraster.h"
#else
#include "x11/wraster.h"
#endif

#define BOOL XWINDOWSBOOL
#include <DPS/dpsXclient.h>
#include <DPS/dpsXshare.h>
#undef BOOL
#include <DPS/dpsclient.h>
#include <DPS/dpsops.h>
#include <DPS/psops.h>

#include "x11/XGServerWindow.h"
#include "xdps/NSDPSContext.h"
#include "AppKit/NSFont.h"
#include "extensions.h"
#include "fonts.h"
#include "general.h"

#define XDPY (((RContext *)context)->dpy)
#define XSCR (((RContext *)context)->screen_number)

@interface NSDPSContext(PrivateOps)
- (void)DPSundefineuserobject: (int)index;
@end

@implementation NSDPSContext (Ops)

/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
- (void)DPScurrentblackgeneration
{
  DPScurrentblackgeneration(dps_context);
}

- (void)DPScurrentcmykcolor: (float *)c : (float *)m : (float *)y : (float *)k 
{
  DPScurrentcmykcolor(dps_context, c, m, y, k);
}

- (void)DPScurrentcolorscreen
{
  DPScurrentcolorscreen(dps_context);
}

- (void)DPScurrentcolortransfer
{
  DPScurrentcolortransfer(dps_context);
}

- (void)DPScurrentundercolorremoval
{
  DPScurrentundercolorremoval(dps_context);
}

- (void)DPSsetblackgeneration
{
  DPSsetblackgeneration(dps_context);
}

- (void)DPSsetcmykcolor: (float)c : (float)m : (float)y : (float)k 
{
  DPSsetcmykcolor(dps_context, c, m, y, k);
}

- (void)DPSsetcolorscreen
{
  DPSsetcolorscreen(dps_context);
}

- (void)DPSsetcolortransfer
{
  DPSsetcolortransfer(dps_context);
}

- (void)DPSsetundercolorremoval
{
  DPSsetundercolorremoval(dps_context);
}

/* ----------------------------------------------------------------------- */
/* Data operations */
/* ----------------------------------------------------------------------- */
- (void)DPSclear
{
  DPSclear(dps_context);
}

- (void)DPScleartomark
{
  DPScleartomark(dps_context);
}

- (void)DPScopy: (int)n
{
  DPScopy(dps_context, n);
}

- (void)DPScount: (int *)n
{
  DPScount(dps_context, n);
}

- (void)DPScounttomark: (int *)n
{
  DPScounttomark(dps_context, n);
}

- (void)DPSdup
{
  DPSdup(dps_context);
}

- (void)DPSexch
{
  DPSexch(dps_context);
}

- (void)DPSexecstack
{
  DPSexecstack(dps_context);
}

- (void)DPSget
{
  DPSget(dps_context);
}

- (void)DPSindex: (int)i
{
  DPSindex(dps_context, i);
}

- (void)DPSmark
{
  DPSmark(dps_context);
}

- (void)DPSmatrix
{
  DPSmatrix(dps_context);
}

- (void)DPSnull
{
  DPSnull(dps_context);
}

- (void)DPSpop
{
  DPSpop(dps_context);
}

- (void)DPSput
{
  DPSput(dps_context);
}

- (void)DPSroll: (int)n : (int)j
{
  DPSroll(dps_context, n, j);
}

/* ----------------------------------------------------------------------- */
/* Font operations */
/* ----------------------------------------------------------------------- */
- (void)DPSFontDirectory
{
  DPSFontDirectory(dps_context);
}

- (void)DPSISOLatin1Encoding
{
  DPSISOLatin1Encoding(dps_context);
}

- (void)DPSSharedFontDirectory
{
  DPSSharedFontDirectory(dps_context);
}

- (void)DPSStandardEncoding
{
  DPSStandardEncoding(dps_context);
}

- (void)DPScachestatus: (int *)bsize : (int *)bmax : (int *)msize 
{
  //PScachestatus(dps_context);
}

- (void)DPScurrentcacheparams
{
  DPScurrentcacheparams(dps_context);
}

- (void)DPScurrentfont
{
  DPScurrentfont(dps_context);
}

- (void)DPSdefinefont
{
  DPSdefinefont(dps_context);
}

- (void)DPSfindfont: (const char *)name 
{
  DPSfindfont(dps_context, name);
}

- (void)DPSmakefont
{
  DPSmakefont(dps_context);
}

- (void)DPSscalefont: (float)size 
{
  DPSscalefont(dps_context, size);
}

- (void)DPSselectfont: (const char *)name : (float)scale 
{
  DPSselectfont(dps_context, name, scale);
}

- (void)DPSsetcachedevice: (float)wx : (float)wy : (float)llx : (float)lly : (float)urx : (float)ury 
{
  DPSsetcachedevice(dps_context, wx, wy, llx, lly, urx, ury);
}

- (void)DPSsetcachelimit: (float)n 
{
  DPSsetcachelimit(dps_context, n);
}

- (void)DPSsetcacheparams
{
  DPSsetcacheparams(dps_context);
}

- (void)DPSsetcharwidth: (float)wx : (float)wy 
{
  DPSsetcharwidth(dps_context, wx, wy);
}

- (void)DPSsetfont: (int)f 
{
  DPSsetfont(dps_context, f);
}

- (void)DPSundefinefont: (const char *)name 
{
  DPSundefinefont(dps_context, name);
}

- (void) GSSetFont: (NSFont*) font
{
  NSString *fontName = [font fontName];

  if ([[self focusView] isFlipped])
    {
      float invmatrix[6];
      
      memcpy(invmatrix, [font matrix], sizeof(invmatrix));
      invmatrix[3] = -invmatrix[3];
      PSWSetFont ([fontName cString], invmatrix);
    }
  else
    PSWSetFont ([fontName cString], [font matrix]);
}

/* ----------------------------------------------------------------------- */
/* System  operations */
/* ----------------------------------------------------------------------- */
- (void)DPSrestore
{
  DPSrestore(dps_context);
}

- (void)DPSsave
{
  DPSsave(dps_context);
}

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void)DPSconcat: (const float *)m
{
  DPSconcat(dps_context, m);
}

- (void)DPScurrentdash
{
  DPScurrentdash(dps_context);
}

- (void)DPScurrentflat: (float *)flatness 
{
  DPScurrentflat(dps_context, flatness);
}

- (void)DPScurrentgray: (float *)gray 
{
  DPScurrentgray(dps_context, gray);
}

- (void)DPScurrenthalftone
{
  DPScurrenthalftone(dps_context);
}

- (void)DPScurrenthalftonephase: (float *)x : (float *)y 
{
  DPScurrenthalftonephase(dps_context, x, y);
}

- (void)DPScurrenthsbcolor: (float *)h : (float *)s : (float *)b 
{
  DPScurrenthsbcolor(dps_context, h, s, b);
}

- (void)DPScurrentlinecap: (int *)linecap 
{
  DPScurrentlinecap(dps_context, linecap);
}

- (void)DPScurrentlinejoin: (int *)linejoin 
{
  DPScurrentlinejoin(dps_context, linejoin);
}

- (void)DPScurrentlinewidth: (float *)width 
{
  DPScurrentlinewidth(dps_context, width);
}

- (void)DPScurrentmatrix
{
  DPScurrentmatrix(dps_context);
}

- (void)DPScurrentmiterlimit: (float *)limit 
{
  DPScurrentmiterlimit(dps_context, limit);
}

- (void)DPScurrentpoint: (float *)x : (float *)y 
{
  DPScurrentpoint(dps_context, x, y);
}

- (void)DPScurrentrgbcolor: (float *)r : (float *)g : (float *)b 
{
  DPScurrentrgbcolor(dps_context, r, g, b);
}

- (void)DPScurrentscreen
{
  DPScurrentscreen(dps_context);
}

- (void)DPScurrentstrokeadjust: (int *)b 
{
  DPScurrentstrokeadjust(dps_context, b);
}

- (void)DPScurrenttransfer
{
  DPScurrenttransfer(dps_context);
}

- (void)DPSdefaultmatrix
{
  DPSdefaultmatrix(dps_context);
}

- (void)DPSgrestore
{
  DPSgrestore(dps_context);
}

- (void)DPSgrestoreall
{
  DPSgrestoreall(dps_context);
}

- (void)DPSgsave
{
  DPSgsave(dps_context);
}

- (void)DPSinitgraphics
{
  DPSinitgraphics(dps_context);
}

- (void)DPSinitmatrix
{
  DPSinitmatrix(dps_context);
  /* This works around problems with non-"identity" matrices */
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"DPSDefaultMatrix"] 
      == NO)
    {
      float nctm[6];
      NSDebugLog(@"Resetting default matrix\n");
      nctm[0]=1; nctm[1]=0; nctm[2]=0; nctm[3]=-1; nctm[4]=0; nctm[5]=0;
      PSWSetMatrix(nctm);
    }
}

- (void)DPSrotate: (float)angle 
{
  DPSrotate(dps_context, angle);
}

- (void)DPSscale: (float)x : (float)y 
{
  DPSscale(dps_context, x, y);
}

- (void)DPSsetdash: (const float *)pat : (int)size : (float)offset 
{
  DPSsetdash(dps_context, pat, size, offset);
}

- (void)DPSsetflat: (float)flatness 
{
  DPSsetflat(dps_context, flatness);
}

- (void)DPSsetgray: (float)gray 
{
  DPSsetgray(dps_context, gray);
}

- (void)DPSsetgstate: (int)gst 
{
  DPSsetgstate(dps_context, gst);
}

- (void)DPSsethalftone
{
  DPSsethalftone(dps_context);
}

- (void)DPSsethalftonephase: (float)x : (float)y 
{
  DPSsethalftonephase(dps_context, x, y);
}

- (void)DPSsethsbcolor: (float)h : (float)s : (float)b 
{
  DPSsethsbcolor(dps_context, h, s, b);
}

- (void)DPSsetlinecap: (int)linecap 
{
  DPSsetlinecap(dps_context, linecap);
}

- (void)DPSsetlinejoin: (int)linejoin 
{
  DPSsetlinejoin(dps_context, linejoin);
}

- (void)DPSsetlinewidth: (float)width 
{
  DPSsetlinewidth(dps_context, width);
}

- (void)DPSsetmatrix
{
  DPSsetmatrix(dps_context);
}

- (void)DPSsetmiterlimit: (float)limit 
{
  DPSsetmiterlimit(dps_context, limit);
}

- (void)DPSsetrgbcolor: (float)r : (float)g : (float)b 
{
  DPSsetrgbcolor(dps_context, r, g, b);
}

- (void)DPSsetscreen
{
  DPSsetscreen(dps_context);
}

- (void)DPSsetstrokeadjust: (int)b 
{
  DPSsetstrokeadjust(dps_context, b);
}

- (void)DPSsettransfer
{
  DPSsettransfer(dps_context);
}

- (void)DPStranslate: (float)x : (float)y 
{
  DPStranslate(dps_context, x, y);
}

/* Should work the same as 'unique_index exch defineuserobject' */
- (int) GSDefineGState
{
  return PSDefineAsUserObj();
}

- (void) GSUndefineGState: (int)gst
{
  [self DPSundefineuserobject: gst];
}

/* Should work the same as 'currentgstate pop' */
- (void) GSReplaceGState: (int)gst
{
  [self DPScurrentgstate: gst];
  [self DPSpop];
}

/* ----------------------------------------------------------------------- */
/* I/O operations */
/* ----------------------------------------------------------------------- */
- (void)DPSflush
{
  DPSflush(dps_context);
}

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
- (void)DPSconcatmatrix
{
  DPSconcatmatrix(dps_context);
}

- (void)DPSdtransform: (float)x1 : (float)y1 : (float *)x2 : (float *)y2 
{
  DPSdtransform(dps_context, x1, y1, x2, y2);
}

- (void)DPSidentmatrix
{
  DPSidentmatrix(dps_context);
}

- (void)DPSidtransform: (float)x1 : (float)y1 : (float *)x2 : (float *)y2 
{
  DPSidtransform(dps_context, x1, y1, x2, y2);
}

- (void)DPSinvertmatrix
{
  DPSinvertmatrix(dps_context);
}

- (void)DPSitransform: (float)x1 : (float)y1 : (float *)x2 : (float *)y2 
{
  DPSitransform(dps_context, x1, y1, x2, y2);
}

- (void)DPStransform: (float)x1 : (float)y1 : (float *)x2 : (float *)y2 
{
  DPStransform(dps_context, x1, y1, x2, y2);
}

/* ----------------------------------------------------------------------- */
/* Opstack operations */
/* ----------------------------------------------------------------------- */

- (void)DPSdefineuserobject
{
  DPSdefineuserobject(dps_context);
}

- (void)DPSexecuserobject: (int)index
{
  DPSexecuserobject(dps_context, index);
}

- (void)DPSundefineuserobject: (int)index
{
  DPSundefineuserobject(dps_context, index);
}

- (void)DPSgetboolean: (int *)it 
{
  DPSgetboolean(dps_context, it);
}

- (void)DPSgetchararray: (int)size : (char *)s 
{
  DPSgetchararray(dps_context, size, s);
}

- (void)DPSgetfloat: (float *)it 
{
  DPSgetfloat(dps_context, it);
}

- (void)DPSgetfloatarray: (int)size : (float *)a 
{
  DPSgetfloatarray(dps_context, size, a);
}

- (void)DPSgetint: (int *)it 
{
  DPSgetint(dps_context, it);
}

- (void)DPSgetintarray: (int)size : (int *)a 
{
  DPSgetintarray(dps_context, size, a);
}

- (void)DPSgetstring: (char *)s 
{
  DPSgetstring(dps_context, s);
}

- (void)DPSsendboolean: (int)it 
{
  DPSsendboolean(dps_context, it);
}

- (void)DPSsendchararray: (const char *)s : (int)size 
{
  DPSsendchararray(dps_context, s, size);
}

- (void)DPSsendfloat: (float)it 
{
  DPSsendfloat(dps_context, it);
}

- (void)DPSsendfloatarray: (const float *)a : (int)size 
{
  DPSsendfloatarray(dps_context, a, size);
}

- (void)DPSsendint: (int)it 
{
  DPSsendint(dps_context, it);
}

- (void)DPSsendintarray: (const int *)a : (int)size 
{
  DPSsendintarray(dps_context, a, size);
}

- (void)DPSsendstring: (const char *)s 
{
  DPSsendstring(dps_context, s);
}

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (void)DPSashow: (float)x : (float)y : (const char *)s 
{
  DPSashow(dps_context, x, y, s);
}

- (void)DPSawidthshow: (float)cx : (float)cy : (int)c : (float)ax : (float)ay : (const char *)s 
{
  DPSawidthshow(dps_context, cx, cy, c, ax, ay, s);
}

- (void)DPScopypage
{
  DPScopypage(dps_context);
}

- (void)DPSeofill
{
  DPSeofill(dps_context);
}

- (void)DPSerasepage
{
  DPSerasepage(dps_context);
}

- (void)DPSfill
{
  DPSfill(dps_context);
}

- (void)DPSimage
{
  DPSimage(dps_context);
}

- (void)DPSimagemask
{
  DPSimagemask(dps_context);
}

- (void)DPScolorimage
{
  DPScolorimage(dps_context);
}

- (void)DPSalphaimage
{
  if (ext_flags & ALPHAIMAGE_EXT)
    PSWalphaimage();
  else
    NSDebugLLog(@"NSDPSContext", @"DPS does not support alphaimage op\n");
}

- (void)DPSkshow: (const char *)s 
{
  DPSkshow(dps_context, s);
}

- (void)DPSrectfill: (float)x : (float)y : (float)w : (float)h 
{
  DPSrectfill(dps_context, x, y, w, h);
}

- (void)DPSrectstroke: (float)x : (float)y : (float)w : (float)h 
{
  DPSrectstroke(dps_context, x, y, w, h);
}

- (void)DPSshow: (const char *)s 
{
  DPSshow(dps_context, s);
}

- (void)DPSshowpage
{
  DPSshowpage(dps_context);
}

- (void)DPSstroke
{
  DPSstroke(dps_context);
}

- (void)DPSstrokepath
{
  DPSstrokepath(dps_context);
}

- (void)DPSueofill: (const char *)nums : (int)n : (const char *)ops : (int)l 
{
  DPSueofill(dps_context, nums, n, ops, l);
}

- (void)DPSufill: (const char *)nums : (int)n : (const char *)ops : (int)l 
{
  DPSufill(dps_context, nums, n, ops, l);
}

- (void)DPSustroke: (const char *)nums : (int)n : (const char *)ops : (int)l 
{
  DPSustroke(dps_context, nums, n, ops, l);
}

- (void)DPSustrokepath: (const char *)nums : (int)n : (const char *)ops : (int)l 
{
  DPSustrokepath(dps_context, nums, n, ops, l);
}

- (void)DPSwidthshow: (float)x : (float)y : (int)c : (const char *)s 
{
  DPSwidthshow(dps_context, x, y, c, s);
}

- (void)DPSxshow: (const char *)s : (const float *)numarray : (int)size 
{
  DPSxshow(dps_context, s, numarray, size);
}

- (void)DPSxyshow: (const char *)s : (const float *)numarray : (int)size 
{
  DPSxyshow(dps_context, s, numarray, size);
}

- (void)DPSyshow: (const char *)s : (const float *)numarray : (int)size 
{
  DPSyshow(dps_context, s, numarray, size);
}

/* ----------------------------------------------------------------------- */
/* Path operations */
/* ----------------------------------------------------------------------- */
- (void)DPSarc: (float)x : (float)y : (float)r : (float)angle1 : (float)angle2 
{
  DPSarc(dps_context, x, y, r, angle1, angle2);
}

- (void)DPSarcn: (float)x : (float)y : (float)r : (float)angle1 : (float)angle2 
{
  DPSarcn(dps_context, x, y, r, angle1, angle2);
}

- (void)DPSarct: (float)x1 : (float)y1 : (float)x2 : (float)y2 : (float)r 
{
  DPSarct(dps_context, x1, y1, x2, y2, r);
}

- (void)DPSarcto: (float)x1 : (float)y1 : (float)x2 : (float)y2 : (float)r : (float *)xt1 : (float *)yt1 : (float *)xt2 : (float *)yt2 
{
  DPSarcto(dps_context, x1, y1, x2, y2, r, xt1, yt1, xt2, yt2);
}

- (void)DPScharpath: (const char *)s : (int)b 
{
  DPScharpath(dps_context, s, b);
}

- (void)DPSclip
{
  DPSclip(dps_context);
}

- (void)DPSclippath
{
  DPSclippath(dps_context);
}

- (void)DPSclosepath
{
  DPSclosepath(dps_context);
}

- (void)DPScurveto: (float)x1 : (float)y1 : (float)x2 : (float)y2 : (float)x3 : (float)y3 
{
  DPScurveto(dps_context, x1, y1, x2, y2, x3, y3);
}

- (void)DPSeoclip
{
  DPSeoclip(dps_context);
}

- (void)DPSeoviewclip
{
  DPSeoviewclip(dps_context);
}

- (void)DPSflattenpath
{
  DPSflattenpath(dps_context);
}

- (void)DPSinitclip
{
  DPSinitclip(dps_context);
}

- (void)DPSinitviewclip
{
  DPSinitviewclip(dps_context);
}

- (void)DPSlineto: (float)x : (float)y 
{
  DPSlineto(dps_context, x, y);
}

- (void)DPSmoveto: (float)x : (float)y 
{
  DPSmoveto(dps_context, x, y);
}

- (void)DPSnewpath
{
  DPSnewpath(dps_context);
}

- (void)DPSpathbbox: (float *)llx : (float *)lly : (float *)urx : (float *)ury 
{
  DPSpathbbox(dps_context, llx, lly, urx, ury);
}

- (void)DPSpathforall
{
  DPSpathforall(dps_context);
}

- (void)DPSrcurveto: (float)x1 : (float)y1 : (float)x2 : (float)y2 : (float)x3 : (float)y3 
{
  DPSrcurveto(dps_context, x1, y1, x2, y2, x3, y3);
}

- (void)DPSrectclip: (float)x : (float)y : (float)w : (float)h 
{
  DPSrectclip(dps_context, x, y, w, h);
}

- (void)DPSrectviewclip: (float)x : (float)y : (float)w : (float)h 
{
  DPSrectviewclip(dps_context, x, y, w, h);
}

- (void)DPSreversepath
{
  DPSreversepath(dps_context);
}

- (void)DPSrlineto: (float)x : (float)y 
{
  DPSrlineto(dps_context, x, y);
}

- (void)DPSrmoveto: (float)x : (float)y 
{
  DPSrmoveto(dps_context, x, y);
}

- (void)DPSsetbbox: (float)llx : (float)lly : (float)urx : (float)ury 
{
  DPSsetbbox(dps_context, llx, lly, urx, ury);
}

- (void)DPSsetucacheparams
{
  DPSsetucacheparams(dps_context);
}

- (void)DPSuappend: (const char *)nums : (int)n : (char *)ops : (int)l 
{
  DPSuappend(dps_context, nums, n, ops, l);
}

- (void)DPSucache
{
  DPSucache(dps_context);
}

- (void)DPSucachestatus
{
  DPSucachestatus(dps_context);
}

- (void)DPSupath: (int)b 
{
  DPSupath(dps_context, b);
}

- (void)DPSviewclip
{
  DPSviewclip(dps_context);
}

- (void)DPSviewclippath
{
  DPSviewclippath(dps_context);
}

/* ----------------------------------------------------------------------- */
/* X operations */
/* ----------------------------------------------------------------------- */
- (void)DPScurrentdrawingfunction: (int *)function 
{
  DPScurrentXdrawingfunction(dps_context, function);
}

- (void)DPScurrentgcdrawable: (void **)gc : (void **)draw : (int *)x : (int *)y 
{
  /* FIXME: This really can't work since this returns an XGContext not a GC */
  DPScurrentXgcdrawable(dps_context, gc, (int *)draw, x, y);
}

- (void)DPScurrentgcdrawablecolor: (void **)gc : (void **)draw : (int *)x : (int *)y 
				      : (int *)colorInfo
{
  /* FIXME: This really can't work since this returns an XGContext not a GC */
  DPScurrentXgcdrawablecolor(dps_context, gc, (int *)draw, x, y, colorInfo);
}

- (void)DPScurrentoffset: (int *)x : (int *)y 
{
  DPScurrentXoffset(dps_context, x, y);
}

- (void)DPSsetdrawingfunction: (int)function 
{
  DPSsetXdrawingfunction(dps_context, function);
}

- (void)DPSsetoffset: (short int)x : (short int)y 
{
  DPSsetXoffset(dps_context, x, y);
}

- (void)DPSsetXrgbactual: (double)r : (double)g : (double)b : (int *)success 
{
  DPSsetXrgbactual(dps_context, r, g, b, success);
}

- (void)DPSsetgcdrawable: (void *)gc : (void *)draw : (int)x : (int)y 
{
  DPSsetXgcdrawable(dps_context, XGContextFromGC(gc), (int)draw, x, y);
}

- (void)DPSsetgcdrawablecolor: (void *)gc : (void *)draw : (int)x : (int)y 
				  : (const int *)colorInfo
{
  DPSsetXgcdrawablecolor(dps_context, XGContextFromGC(gc), (int)draw, x, y, colorInfo);
}

/*-------------------------------------------------------------------------*/
/* Graphics Extension Ops */
/*-------------------------------------------------------------------------*/
- (void) _copyBits: (int) srcGstate : (NSRect) srcRect : (NSPoint) destPoint
{
  XRectangle	dst;
  XRectangle    src;
  Drawable source, draw;
  NSWindow *window;
  gswindow_device_t *windev;

  window = [[self focusView] window];
  if ([window gState] == 0)
    return;
  windev = [XGServer _windowWithTag: [window windowNumber]];
  draw = (windev->buffer) ? windev->buffer : windev->ident;
  if (draw == 0)
    return;

      NSDebugLLog (@"CTM", @"Frame  %@\n", NSStringFromRect(windev->xframe));
  source = 0;
  if (srcGstate != 0)
    {
      int gc, x, y;
      PSgsave();
      PSsetgstate(srcGstate);
      PScurrentXgcdrawable(&gc, (int *)(&source), &x, &y);
      if (source == 0)
        return;
      if (source == draw)
	{
	  /* This probably shouldn't happen, It might come from a bug
	     in DGS 0.5.x
	  */
	}
      NSDebugLLog (@"Copy", @"Orig  %@\n", NSStringFromRect(srcRect));
      srcRect = [self XRectFromUserRect: srcRect];
      NSDebugLLog (@"Copy", @"XCoor %@\n", NSStringFromRect(srcRect));
      PSgrestore();
    }
  else
    {
      source = draw;
      srcRect = [self XRectFromUserRect: srcRect];
    }
  NSDebugLLog (@"Copy", @"ODest %@\n", NSStringFromPoint(destPoint));
  destPoint = [self XPointFromUserPoint: destPoint];
  /* FIXME: Why is this needed? */
  if (![[self focusView] isFlipped])
    destPoint.y -= NSHeight(srcRect);
  NSDebugLLog (@"Copy", @"XDest %@\n", NSStringFromPoint(destPoint));

  src.x = NSMinX(srcRect); src.y = NSMinY(srcRect);
  src.width = NSWidth(srcRect); src.height = NSHeight(srcRect);
  dst.x = destPoint.x; dst.y = destPoint.y;
  NSDebugLLog (@"NSWindow", @"Copying bitmap from (%d %d %d %d) to (%d %d)\n",
  	src.x, src.y, src.width, src.height, dst.x, dst.y);
  [self wait];
  XCopyArea([self xDisplay], source, draw, windev->gc,
                src.x, src.y, src.width, src.height, dst.x, dst.y);
}

- (void) DPScomposite: (float)x : (float)y : (float)w : (float)h : (int)gstateNum : (float)dx : (float)dy : (int)op
{
  if (ext_flags & COMPOSITE_EXT)
    PSWcomposite(x, y, w, h, gstateNum, dx, dy, op);
  else
    {
      NSRect s = NSMakeRect(x, y, w, h);
      NSPoint d = NSMakePoint(dx, dy);
      NSDebugLLog(@"NSDPSContext", @"DPS does not support composite op\n");
      [self wait];
      [self _copyBits:gstateNum : s : d];
    }
}

- (void) DPScompositerect: (float)x : (float)y : (float)w : (float)h : (int)op
{
  if (ext_flags & COMPOSITERECT_EXT)
    PSWcompositerect(x, y, w, h, op);
  else
    {
      /* Try to emulate this */
      float gray;
      XGCValues gcv;
      NSWindow *window;
      gswindow_device_t *windev;
      
      DPScurrentgray(dps_context, &gray);
      if (fabs(gray - 0.667) < .002)
	DPSsetgray(dps_context, 0.333);
      else    
	DPSsetrgbcolor(dps_context, 0.121, 0.121, 0);
      
      switch (op)
	{
	case   NSCompositeClear:
	  gcv.function = GXclear;
	  break;
	case   NSCompositeCopy:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeSourceOver:
	case   NSCompositeHighlight:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeSourceIn:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeSourceOut:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeSourceAtop:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeDestinationOver:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeDestinationIn:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeDestinationOut:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeDestinationAtop:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositeXOR:
	  gcv.function = GXcopy;
	  break;
	case   NSCompositePlusDarker:
	  gcv.function = GXcopy;
	  break;
	case   GSCompositeHighlight:
	  gcv.function = GXxor;
	  break;
	case   NSCompositePlusLighter:
	  gcv.function = GXcopy;
	  break;
	default:
	  gcv.function = GXcopy;
	  break;
	}
      
      window = [[self focusView] window];
      windev = [XGServer _windowWithTag: [window windowNumber]];
      [self wait];
      XChangeGC(XDPY, windev->gc, GCFunction, &gcv);
      DPSrectfill(dps_context, x, y, w, h);
      [window flushWindow];
      gcv.function = GXcopy;
      XChangeGC(XDPY, windev->gc, GCFunction, &gcv);
      DPSsetgray(dps_context, gray);
    }
}

- (void) DPSdissolve: (float)x : (float)y : (float)w : (float)h : (int)gstateNum
 : (float)dx : (float)dy : (float)delta
{
  if (ext_flags & DISSOLVE_EXT)
    PSWdissolve(x, y, w, h, gstateNum, dx, dy, delta);
  else
    NSDebugLLog(@"NSDPSContext", @"DPS does not support dissolve op\n");
}

- (void) DPSreadimage
{
  if (ext_flags & READIMAGE_EXT)
    PSWreadimage();
  else
    NSDebugLLog(@"NSDPSContext", @"DPS does not support readimage op\n");
}

- (void) DPSsetalpha: (float)a
{
  if (ext_flags & SETALPHA_EXT)
    PSWsetalpha(a);
  else
    NSDebugLLog(@"NSDPSContext", @"DPS does not support setalpha op\n");
}

- (void) DPScurrentalpha: (float *)alpha
{
  if (ext_flags & SETALPHA_EXT)
    PSWcurrentalpha(alpha);
  else
    NSDebugLLog(@"NSDPSContext", @"DPS does not support currentalpha op\n");
}

- (void) DPSflushpage
{
  if (ext_flags & FLUSHPAGE_EXT)
    PSWflushpage();
}

/* ----------------------------------------------------------------------- */
/* Client functions */
/* ----------------------------------------------------------------------- */
- (void) DPSPrintf: (char *)fmt : (va_list)args
{
  int count;
  char buf[1024];
  count = vsnprintf(buf, 1024, fmt, args);
  DPSWriteData(dps_context, buf, MIN(1024, count));
}

- (void) DPSWriteData: (char *)buf : (unsigned int)count
{
  DPSWriteData(dps_context, buf, count);
}

//
// Imaging Functions
//
- (void) NSDrawBitmap: (NSRect) rect : (int) pixelsWide : (int) pixelsHigh
		     : (int) bitsPerSample : (int) samplesPerPixel 
		     : (int) bitsPerPixel : (int) bytesPerRow : (BOOL) isPlanar
		     : (BOOL) hasAlpha : (NSString *) colorSpaceName
		     : (const unsigned char *const [5]) data
{
  int bytes;
  int working_alphaimage;

  // FIXME
#if 0
  working_alphaimage = [self operatorExtensions]
    & ALPHAIMAGE_EXT;
#else
  working_alphaimage = NO;
#endif

  /* Save scaling */
  PSmatrix(); PScurrentmatrix();
  PSmoveto(NSMinX(rect), NSMinY(rect));
  PSscale(NSWidth(rect), NSHeight(rect));

  if (bitsPerSample == 0)
    bitsPerSample = 8;
  bytes = 
    (bitsPerSample * pixelsWide * pixelsHigh + 7) / 8;
  if (bytes * samplesPerPixel != bytesPerRow * pixelsHigh) 
    {
      NSLog(@"Image Rendering Error: Dodgy bytesPerRow value %d", bytesPerRow);
      NSLog(@"   pixelsHigh=%d, bytes=%d, samplesPerPixel=%d",
	    bytesPerRow, pixelsHigh, bytes);
      return;
    }

  // send the PostScript code
  if (hasAlpha && working_alphaimage == YES) 
    {
      // FIXME
      NSLog(@"Alphaimage not implemented");
      return;
    } 
  else if (samplesPerPixel > 1) 
    {
      if (isPlanar || (hasAlpha && (working_alphaimage == NO))) 
	{
	  if (bitsPerSample != 8) 
	    {
	      NSLog(@"Image format conversion not supported for bps!=8");
	      return;
	    }
	}
      PSWColorImageHeader(pixelsWide, pixelsHigh, 
			  bitsPerSample,
			  hasAlpha?(samplesPerPixel-1):samplesPerPixel);
    } 
  else
    PSWImageHeader(pixelsWide, pixelsHigh, bitsPerSample);
  
  // The context is now waiting for data on its standard input
  if (isPlanar || (hasAlpha && (working_alphaimage == NO))) 
    {
      // We need to do a format conversion.
      // We do this on the fly, sending data to the context as soon as
      // it is computed.
      int i, j, spp, isAlpha, alpha;
      unsigned char val;
      isAlpha = hasAlpha && (working_alphaimage == NO);
      if (isAlpha)
	spp = samplesPerPixel - 1;
      else
	spp = samplesPerPixel;
      
      for (j=0; j<bytes; j++) 
	{
	  if (isAlpha) 
	    {
	      if (isPlanar)
		alpha = data[spp][j];
	      else
		alpha = data[0][spp+j*samplesPerPixel];
	    }
	  for (i = 0; i < spp; i++) 
	    {
	      if (isPlanar)
		val = data[i][j];
	      else
		val = data[0][i+j*samplesPerPixel];
	      if (isAlpha)
		val = 255 - ((255-val)*(long)alpha)/255;
	      DPSWriteData(dps_context, &val, 1);
	    }
	}
    } 
  else 
    {
      // The data is already in the format the context expects it in
      DPSWriteData(dps_context, (char*)data[0], bytes*samplesPerPixel);
    }

  /* Restore original scaling */
  PSsetmatrix();
}

@end
