/*
   Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

   This file is part of GNUstep.

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

#ifndef ARTGState_h
#define ARTGState_h

#include "art/ARTContext.h"
#include "gsc/GSGState.h"

#include "config.h"

#if (BUILD_SERVER==SERVER_rds)
#define RDS
#endif

#ifndef RDS
#include "x11/XGServer.h"
#include "x11/XGServerWindow.h"
#else
#include "rds/RDSServer.h"
#endif


#include <libart_lgpl/art_vpath_dash.h>


@class XWindowBuffer;


@interface ARTGState : GSGState
{
	unsigned char fill_color[4],stroke_color[4];

	float line_width;
	int linecapstyle,linejoinstyle;
	float miter_limit;
	BOOL strokeadjust;

	struct _ArtVpathDash dash;
	int do_dash;


	XWindowBuffer *wi;

	int clip_x0,clip_y0,clip_x1,clip_y1;
	BOOL all_clipped;
#define CLIP_DATA (wi->data+clip_x0*wi->bytes_per_pixel+clip_y0*wi->bytes_per_line)
	int clip_sx,clip_sy;

	/*
	Clipping spans are stored this way. clip_index has an index to the
	spans (in clip_span) for each line. clip_span has the x-starting
	coordinate for each span. A line starts 'off', each coordinate flips
	the state. The spans are stored in increasing y order, so
	clip_index[y+1]-1 is the index of the last span coordinate. Thus, if
	clip_index[y]==clip_index[y+1], the entire line is clipped.
	clip_index actually has clip_sy+1 entries, so clip_index[y+1] is
	valid for _all_ lines.

	All coordinates are in device space and counted inside the clipping
	rectangle.

	To make things easier, each line also ends in the off state (so the
	last entry in clip_span might be a dummy entry at the end of the
	line).
	*/
	unsigned int *clip_span;
	unsigned int *clip_index;
	int clip_num_span;
}

@end


@interface ARTGState (internal_stuff)
-(void) GSSetDevice: (gswindow_device_t *)win : (int)x : (int)y;
-(void) GSCurrentDevice: (void **)device : (int *)x : (int *)y;
@end

#define UPDATE_UNBUFFERED \
  if (wi->window->type==NSBackingStoreNonretained) \
    { \
      [wi _exposeRect: NSMakeRect(clip_x0,clip_y0,clip_sx,clip_sy)]; \
    }

extern struct draw_info_s ART_DI;
#define DI ART_DI


#endif

