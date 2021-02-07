/* context.c - X context management
 * 
 *  Raster graphics library
 * 
 *  Copyright (c) 1997-2002 Alfredo K. Kojima
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *  
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <math.h>

#include "wrasterP.h"

#ifdef HAVE_HERMES
#include <Hermes/Hermes.h>
#endif

#include "x11/StdCmap.h"

#include "x11/wraster.h"

#ifdef XRENDER
#include <X11/extensions/Xrender.h>
#endif

extern void _wraster_change_filter(int type);


static Bool bestContext(Display *dpy, int screen_number, RContext *context);

static RContextAttributes DEFAULT_CONTEXT_ATTRIBS = {
    RC_UseSharedMemory|RC_RenderMode|RC_ColorsPerChannel, /* flags */
	RDitheredRendering, 		       /* render_mode */
	4,			       /* colors_per_channel */
	0, 
	0,
	0,
	0,
	True,				   /* use_shared_memory */
	RMitchellFilter,
	RUseStdColormap
};

char**
RSupportedFileFormats(void)
{
  static char *tmp[2];
  tmp[0] = NULL;
  return tmp;
}

/*
 * 
 * Colormap allocation for PseudoColor visuals:
 * 
 * 
 * switch standardColormap:
 * 	none:
 * 		allocate colors according to colors_per_channel
 * 
 * 	best/default:
 * 		if there's a std colormap defined then use it
 * 
 * 		else
 * 			create a std colormap and set it
 */




/*
 *----------------------------------------------------------------------
 * allocateStandardPseudoColor
 * 	Creates the internal colormap for PseudoColor, setting the
 * color values according to the supplied standard colormap.
 * 
 * Returns: -
 *
 * Side effects: -
 *
 * Notes: -
 *----------------------------------------------------------------------
 */
static Bool
allocateStandardPseudoColor(RContext *ctx, XStandardColormap *stdcmap)
{
    int i;

    ctx->ncolors = stdcmap->red_max * stdcmap->red_mult
	+ stdcmap->green_max * stdcmap->green_mult
	+ stdcmap->blue_max * stdcmap->blue_mult + 1;

    if (ctx->ncolors <= 1) {
	RErrorCode = RERR_INTERNAL;
	puts("wraster: bad standard colormap");

	return False;
    }
    
    ctx->colors = malloc(sizeof(XColor)*ctx->ncolors);
    if (!ctx->colors) {
	RErrorCode = RERR_NOMEMORY;

	return False;
    }
    
    ctx->pixels = malloc(sizeof(unsigned long)*ctx->ncolors);
    if (!ctx->pixels) {
	
	free(ctx->colors);
	ctx->colors = NULL;
	
	RErrorCode = RERR_NOMEMORY;

	return False;
    }

    
#define calc(max,mult) (((i / stdcmap->mult) % \
                         (stdcmap->max + 1)) * 65535) / stdcmap->max

    for (i = 0; i < ctx->ncolors; i++) {
	ctx->colors[i].pixel = i + stdcmap->base_pixel;
	ctx->colors[i].red = calc(red_max, red_mult);
	ctx->colors[i].green = calc(green_max, green_mult);
	ctx->colors[i].blue = calc(blue_max, blue_mult);
	
	ctx->pixels[i] = ctx->colors[i].pixel;
    }

#undef calc

    return True;
}


static Bool
setupStandardColormap(RContext *ctx, Atom property)
{
    if (!XmuLookupStandardColormap(ctx->dpy, ctx->screen_number,
				   ctx->visual->visualid,
				   ctx->depth, property,
				   True, True)) {
	RErrorCode = RERR_STDCMAPFAIL;
	
	return False;
    }
    return True;
}









static Bool
allocatePseudoColor(RContext *ctx)
{
    XColor *colors;
    XColor avcolors[256];
    int avncolors;
    int i, ncolors, r, g, b;
    int retries;
    int cpc = ctx->attribs->colors_per_channel;
    
    ncolors = cpc * cpc * cpc;
    
    if (ncolors > (1<<ctx->depth)) {
	/* reduce colormap size */
	cpc = ctx->attribs->colors_per_channel = 1<<((int)ctx->depth/3);
	ncolors = cpc * cpc * cpc;
    }

    assert(cpc >= 2 && ncolors <= (1<<ctx->depth));

    colors = malloc(sizeof(XColor)*ncolors);
    if (!colors) {
	RErrorCode = RERR_NOMEMORY;
	return False;
    }

    ctx->pixels = malloc(sizeof(unsigned long)*ncolors);
    if (!ctx->pixels) {
	free(colors);
	RErrorCode = RERR_NOMEMORY;
	return False;
    }
    
    i=0;

    if ((ctx->attribs->flags & RC_GammaCorrection) && ctx->attribs->rgamma > 0
	&& ctx->attribs->ggamma > 0 && ctx->attribs->bgamma > 0) {
	double rg, gg, bg;
	double tmp;

	/* do gamma correction */
	rg = 1.0/ctx->attribs->rgamma;
	gg = 1.0/ctx->attribs->ggamma;
	bg = 1.0/ctx->attribs->bgamma;
	for (r=0; r<cpc; r++) {
	    for (g=0; g<cpc; g++) {
		for (b=0; b<cpc; b++) {
		    colors[i].red=(r*0xffff) / (cpc-1);
		    colors[i].green=(g*0xffff) / (cpc-1);
		    colors[i].blue=(b*0xffff) / (cpc-1);
		    colors[i].flags = DoRed|DoGreen|DoBlue;

		    tmp = (double)colors[i].red / 65536.0;
		    colors[i].red = (unsigned short)(65536.0*pow(tmp, rg));

		    tmp = (double)colors[i].green / 65536.0;
		    colors[i].green = (unsigned short)(65536.0*pow(tmp, gg));

		    tmp = (double)colors[i].blue / 65536.0;
		    colors[i].blue = (unsigned short)(65536.0*pow(tmp, bg));

		    i++;
		}
	    }
	}

    } else {
	for (r=0; r<cpc; r++) {
	    for (g=0; g<cpc; g++) {
		for (b=0; b<cpc; b++) {
		    colors[i].red=(r*0xffff) / (cpc-1);
		    colors[i].green=(g*0xffff) / (cpc-1);
		    colors[i].blue=(b*0xffff) / (cpc-1);
		    colors[i].flags = DoRed|DoGreen|DoBlue;
		    i++;
		}
	    }
	}
    }
    /* try to allocate the colors */
    for (i=0; i<ncolors; i++) {
	if (!XAllocColor(ctx->dpy, ctx->cmap, &(colors[i]))) {
	    colors[i].flags = 0; /* failed */
	} else {
	    colors[i].flags = DoRed|DoGreen|DoBlue;	    
	}
    }
    /* try to allocate close values for the colors that couldn't 
     * be allocated before */
    avncolors = (1<<ctx->depth>256 ? 256 : 1<<ctx->depth);
    for (i=0; i<avncolors; i++) avcolors[i].pixel = i;

    XQueryColors(ctx->dpy, ctx->cmap, avcolors, avncolors);

    for (i=0; i<ncolors; i++) {
	if (colors[i].flags==0) {
	    int j;
	    unsigned long cdiff=0xffffffff, diff;
	    unsigned long closest=0;
	    
	    retries = 2;
	    
	    while (retries--) {
		/* find closest color */
		for (j=0; j<avncolors; j++) {
		    r = (colors[i].red - avcolors[i].red)>>8;
		    g = (colors[i].green - avcolors[i].green)>>8;
		    b = (colors[i].blue - avcolors[i].blue)>>8;
		    diff = r*r + g*g + b*b;
		    if (diff<cdiff) {
			cdiff = diff;
			closest = j;
		    }
		}
		/* allocate closest color found */
		colors[i].red = avcolors[closest].red;
		colors[i].green = avcolors[closest].green;
		colors[i].blue = avcolors[closest].blue;
		if (XAllocColor(ctx->dpy, ctx->cmap, &colors[i])) {
		    colors[i].flags = DoRed|DoGreen|DoBlue;
		    break; /* succeeded, don't need to retry */
		}
#ifdef DEBUG
		printf("close color allocation failed. Retrying...\n");
#endif
	    }
	}
    }
    
    ctx->colors = colors;
    ctx->ncolors = ncolors;
    
    /* fill the pixels shortcut array */
    for (i = 0; i < ncolors; i++) {
	ctx->pixels[i] = ctx->colors[i].pixel;
    }
    
    return True;
}


static XColor*
allocateGrayScale(RContext *ctx)
{
    XColor *colors;
    XColor avcolors[256];
    int avncolors;
    int i, ncolors, r, g, b;
    int retries;
    int cpc = ctx->attribs->colors_per_channel;

    ncolors = cpc * cpc * cpc;
    
    if (ctx->vclass == StaticGray) {
      /* we might as well use all grays */
      ncolors = 1<<ctx->depth;
    } else {
	if ( ncolors > (1<<ctx->depth) ) {
	    /* reduce colormap size */
	    cpc = ctx->attribs->colors_per_channel = 1<<((int)ctx->depth/3);
	    ncolors = cpc * cpc * cpc;
	}
      
	assert(cpc >= 2 && ncolors <= (1<<ctx->depth));
    }

    if (ncolors>=256 && ctx->vclass==StaticGray) {
        /* don't need dithering for 256 levels of gray in StaticGray visual */
        ctx->attribs->render_mode = RBestMatchRendering;
    }

    colors = malloc(sizeof(XColor)*ncolors);
    if (!colors) {
	RErrorCode = RERR_NOMEMORY;
	return False;
    }
    for (i=0; i<ncolors; i++) {
	colors[i].red=(i*0xffff) / (ncolors-1);
	colors[i].green=(i*0xffff) / (ncolors-1);
	colors[i].blue=(i*0xffff) / (ncolors-1);
	colors[i].flags = DoRed|DoGreen|DoBlue;
    }
    /* try to allocate the colors */
    for (i=0; i<ncolors; i++) {
#ifdef DEBUG
        printf("trying:%x,%x,%x\n",colors[i].red,colors[i].green,colors[i].blue);
#endif
	if (!XAllocColor(ctx->dpy, ctx->cmap, &(colors[i]))) {
	    colors[i].flags = 0; /* failed */
#ifdef DEBUG
	    printf("failed:%x,%x,%x\n",colors[i].red,colors[i].green,colors[i].blue);
#endif
	} else {
	    colors[i].flags = DoRed|DoGreen|DoBlue;	    
#ifdef DEBUG
	    printf("success:%x,%x,%x\n",colors[i].red,colors[i].green,colors[i].blue);
#endif
	}
    }
    /* try to allocate close values for the colors that couldn't 
     * be allocated before */
    avncolors = (1<<ctx->depth>256 ? 256 : 1<<ctx->depth);
    for (i=0; i<avncolors; i++) avcolors[i].pixel = i;

    XQueryColors(ctx->dpy, ctx->cmap, avcolors, avncolors);

    for (i=0; i<ncolors; i++) {
	if (colors[i].flags==0) {
	    int j;
	    unsigned long cdiff=0xffffffff, diff;
	    unsigned long closest=0;
	    
	    retries = 2;
	    
	    while (retries--) {
		/* find closest color */
		for (j=0; j<avncolors; j++) {
		    r = (colors[i].red - avcolors[i].red)>>8;
		    g = (colors[i].green - avcolors[i].green)>>8;
		    b = (colors[i].blue - avcolors[i].blue)>>8;
		    diff = r*r + g*g + b*b;
		    if (diff<cdiff) {
			cdiff = diff;
			closest = j;
		    }
		}
		/* allocate closest color found */
#ifdef DEBUG
		printf("best match:%x,%x,%x => %x,%x,%x\n",colors[i].red,colors[i].green,colors[i].blue,avcolors[closest].red,avcolors[closest].green,avcolors[closest].blue);
#endif
		colors[i].red = avcolors[closest].red;
		colors[i].green = avcolors[closest].green;
		colors[i].blue = avcolors[closest].blue;
		if (XAllocColor(ctx->dpy, ctx->cmap, &colors[i])) {
		    colors[i].flags = DoRed|DoGreen|DoBlue;
		    break; /* succeeded, don't need to retry */
		}
#ifdef DEBUG
		printf("close color allocation failed. Retrying...\n");
#endif
	    }
	}
    }
    return colors;
}


static Bool
setupPseudoColorColormap(RContext *context)
{
    Atom property = 0;
    
    if (context->attribs->standard_colormap_mode == RCreateStdColormap) {
	property = XInternAtom(context->dpy, "RGB_DEFAULT_MAP", False);
	
	if (!setupStandardColormap(context, property)) {
	    return False;
	}
    }

    if (context->attribs->standard_colormap_mode != RIgnoreStdColormap) {
	XStandardColormap *maps;
	int count, i;

	if (!property) {
	    property = XInternAtom(context->dpy, "RGB_BEST_MAP", False);
	    if (!XGetRGBColormaps(context->dpy, 
				  DefaultRootWindow(context->dpy),
				  &maps, &count, property)) {
		maps = NULL;
	    }
	    
	    if (!maps) {
		property = XInternAtom(context->dpy, "RGB_DEFAULT_MAP", False);
		if (!XGetRGBColormaps(context->dpy,
				      DefaultRootWindow(context->dpy),
				      &maps, &count, property)) {
		    maps = NULL;
		}
	    }
	} else {
	    if (!XGetRGBColormaps(context->dpy, 
				  DefaultRootWindow(context->dpy),
				  &maps, &count, property)) {
		maps = NULL;
	    }
	}
	
	if (maps) {	    
	    int theMap = -1;
	    
	    for (i = 0; i < count; i++) {
		if (maps[i].visualid == context->visual->visualid) {
		    theMap = i;
		    break;
		}
	    }

	    if (theMap < 0) {
		puts("wrlib: no std cmap found");
	    }

	    if (theMap >= 0
		&& allocateStandardPseudoColor(context, &maps[theMap])) {

		context->std_rgb_map = XAllocStandardColormap();
		
		*context->std_rgb_map = maps[theMap];
		
		context->cmap = context->std_rgb_map->colormap;

		XFree(maps);

		return True;
	    }
	    
	    XFree(maps);
	}
    }
    
    context->attribs->standard_colormap_mode = RIgnoreStdColormap;

    /* RIgnoreStdColormap and fallback */
    return allocatePseudoColor(context);
}




static char*
mygetenv(char *var, int scr)
{
    char *p;
    char varname[64];

    sprintf(varname, "%s%i", var, scr);
    p = getenv(varname);
    if (!p) {
	p = getenv(var);
    }
    return p;
}


static void 
gatherconfig(RContext *context, int screen_n)
{
    char *ptr;

    ptr = mygetenv("WRASTER_GAMMA", screen_n);
    if (ptr) {
	float g1,g2,g3;
	if (sscanf(ptr, "%f/%f/%f", &g1, &g2, &g3)!=3 
	    || g1<=0.0 || g2<=0.0 || g3<=0.0) {
	    printf("wrlib: invalid value(s) for gamma correction \"%s\"\n", 
		   ptr);
	} else {
	    context->attribs->flags |= RC_GammaCorrection;
	    context->attribs->rgamma = g1;
	    context->attribs->ggamma = g2;
	    context->attribs->bgamma = g3;
	}
    }
    ptr = mygetenv("WRASTER_COLOR_RESOLUTION", screen_n);
    if (ptr) {
	int i;
	if (sscanf(ptr, "%d", &i)!=1 || i<2 || i>6) {
	    printf("wrlib: invalid value for color resolution \"%s\"\n",ptr);
	} else {
	    context->attribs->flags |= RC_ColorsPerChannel;
	    context->attribs->colors_per_channel = i;
	}
    }
    
    ptr = mygetenv("WRASTER_OPTIMIZE_FOR_SPEED", screen_n);
    if (ptr) {
	context->flags.optimize_for_speed = 1;
    } else {
	context->flags.optimize_for_speed = 0;
    }
    
}


static void
getColormap(RContext *context, int screen_number)
{
    Colormap cmap = None;
    XStandardColormap *cmaps;
    int ncmaps, i;

    if (XGetRGBColormaps(context->dpy, 
			 RootWindow(context->dpy, screen_number), 
			 &cmaps, &ncmaps, XA_RGB_DEFAULT_MAP)) { 
	for (i=0; i<ncmaps; ++i) {
	    if (cmaps[i].visualid == context->visual->visualid) {
		cmap = cmaps[i].colormap;
		break;
	    }
	}
	XFree(cmaps);
    }
    if (cmap == None) {
	XColor color;
	
	cmap = XCreateColormap(context->dpy, 
			       RootWindow(context->dpy, screen_number),
			       context->visual, AllocNone);
	
	color.red = color.green = color.blue = 0;
	XAllocColor(context->dpy, cmap, &color);
	context->black = color.pixel;

	color.red = color.green = color.blue = 0xffff;
	XAllocColor(context->dpy, cmap, &color);
	context->white = color.pixel;
	
    }
    context->cmap = cmap;
}


static int
count_offset(unsigned long mask) 
{
    int i;
    
    i=0;
    while ((mask & 1)==0) {
	i++;
	mask = mask >> 1;
    }
    return i;
}


RContext*
RCreateContext(Display *dpy, int screen_number, RContextAttributes *attribs)
{
    RContext *context;
    XGCValues gcv;

    
    context = malloc(sizeof(RContext));
    if (!context) {
	RErrorCode = RERR_NOMEMORY;
	return NULL;
    }
    memset(context, 0, sizeof(RContext));

    context->dpy = dpy;
    
    context->screen_number = screen_number;
    
    context->attribs = malloc(sizeof(RContextAttributes));
    if (!context->attribs) {
	free(context);
	RErrorCode = RERR_NOMEMORY;
	return NULL;
    }
    if (!attribs)
	*context->attribs = DEFAULT_CONTEXT_ATTRIBS;
    else
	*context->attribs = *attribs;

    if (!(context->attribs->flags & RC_StandardColormap)) {
	context->attribs->standard_colormap_mode = RUseStdColormap;
    }

    if (!(context->attribs->flags & RC_ScalingFilter)) {
	context->attribs->flags |= RC_ScalingFilter;
	context->attribs->scaling_filter = RMitchellFilter;
    }

    /* get configuration from environment variables */
    gatherconfig(context, screen_number);
#ifndef BENCH
    _wraster_change_filter(context->attribs->scaling_filter);
#endif
    if ((context->attribs->flags & RC_VisualID)) {
	XVisualInfo *vinfo, templ;
	int nret;
	    
	templ.screen = screen_number;
	templ.visualid = context->attribs->visualid;
	vinfo = XGetVisualInfo(context->dpy, VisualIDMask|VisualScreenMask,
			       &templ, &nret);
	if (!vinfo || nret==0) {
	    free(context);
	    RErrorCode = RERR_BADVISUALID;
	    return NULL;
	}

	if (vinfo[0].visual == DefaultVisual(dpy, screen_number)) {
	    context->attribs->flags |= RC_DefaultVisual;
	} else {
	    XSetWindowAttributes attr;
	    unsigned long mask;
		
	    context->visual = vinfo[0].visual;
	    context->depth = vinfo[0].depth;
	    context->vclass = vinfo[0].class;
	    getColormap(context, screen_number);
	    attr.colormap = context->cmap;
	    attr.override_redirect = True;
	    attr.border_pixel = 0;
	    attr.background_pixel = 0;
	    mask = CWBorderPixel|CWColormap|CWOverrideRedirect|CWBackPixel;
	    context->drawable =
		XCreateWindow(dpy, RootWindow(dpy, screen_number), 1, 1, 
			      1, 1, 0, context->depth, CopyFromParent,
			      context->visual, mask, &attr);
	    /*		XSetWindowColormap(dpy, context->drawable, attr.colormap);*/
	}
	XFree(vinfo);
    }

    /* use default */
    if (!context->visual) {
	if ((context->attribs->flags & RC_DefaultVisual)
	    || !bestContext(dpy, screen_number, context)) {
	    context->visual = DefaultVisual(dpy, screen_number);
	    context->depth = DefaultDepth(dpy, screen_number);
	    context->cmap = DefaultColormap(dpy, screen_number);
	    context->drawable = RootWindow(dpy, screen_number);
	    context->black = BlackPixel(dpy, screen_number);
	    context->white = WhitePixel(dpy, screen_number);
	    context->vclass = context->visual->class;
	}
    }
    
    gcv.function = GXcopy;
    gcv.graphics_exposures = False;
    context->copy_gc = XCreateGC(dpy, context->drawable, GCFunction
				 |GCGraphicsExposures, &gcv);

#ifdef HAVE_HERMES
    context->hermes_data = malloc(sizeof(RHermesData));
    if (!context->hermes_data) {
	RErrorCode = RERR_NOMEMORY;
	free(context);
	return NULL;
    }

    Hermes_Init();

    context->hermes_data->palette = Hermes_PaletteInstance();
    {
	unsigned long flags = 0;
	
	if (context->attribs->render_mode == RDitheredRendering) {
	    flags |= HERMES_CONVERT_DITHER;
	}
	context->hermes_data->converter = Hermes_ConverterInstance(flags);
    }
#endif
    
    if (context->vclass == PseudoColor || context->vclass == StaticColor) {
	if (!setupPseudoColorColormap(context)) {
	    free(context);
	    return NULL;
	}
#ifdef HAVE_HERMES
	{
	    int32 palette[256];
	    int i;

	    for (i = 0; i < context->ncolors; i++) {
		palette[i] = ((context->colors[i].red >> 8) << 16) ||
		    ((context->colors[i].green >> 8) << 8) ||
		    ((context->colors[i].blue >> 8));
	    }
	    
	    Hermes_PaletteSet(context->hermes_data->palette, palette);
	}			  
#endif
    } else if (context->vclass == GrayScale || context->vclass == StaticGray) {
	context->colors = allocateGrayScale(context);
	if (!context->colors) {
	    free(context);
	    return NULL;
	}
#ifdef HAVE_HERMES
	{
	    int32 palette[256];
	    int i;

	    for (i = 0; i < context->ncolors; i++) {
		palette[i] = ((context->colors[i].red >> 8) << 16) ||
		    ((context->colors[i].green >> 8) << 8) ||
		    ((context->colors[i].blue >> 8));
	    }
	    
	    Hermes_PaletteSet(context->hermes_data->palette, palette);
	}			  
#endif	
    } else if (context->vclass == TrueColor) {
    	/* calc offsets to create a TrueColor pixel */
	context->red_offset = count_offset(context->visual->red_mask);
	context->green_offset = count_offset(context->visual->green_mask);
	context->blue_offset = count_offset(context->visual->blue_mask);
	/* disable dithering on 24 bits visuals */
	if (context->depth >= 24)
	    context->attribs->render_mode = RBestMatchRendering;
    }

#ifdef HAVE_HERMES
    
#endif
    
    /* check avaiability of MIT-SHM */
#ifdef XSHM
    if (!(context->attribs->flags & RC_UseSharedMemory)) {
	context->attribs->flags |= RC_UseSharedMemory;
	context->attribs->use_shared_memory = True;
    }

    if (context->attribs->use_shared_memory) {
	int major, minor;
	Bool sharedPixmaps;

	context->flags.use_shared_pixmap = 0;

	if (!XShmQueryVersion(context->dpy, &major, &minor, &sharedPixmaps)) {
	    context->attribs->use_shared_memory = False;
	} else {
	    if (XShmPixmapFormat(context->dpy)==ZPixmap)
	    	context->flags.use_shared_pixmap = sharedPixmaps;
	}
    } 
#endif

    return context;
}


static Bool
bestContext(Display *dpy, int screen_number, RContext *context)
{
    XVisualInfo *vinfo=NULL, rvinfo;
    int best = -1, numvis, i;
    long flags;
    XSetWindowAttributes attr;
    
    rvinfo.class  = TrueColor;
    rvinfo.screen = screen_number;

    #ifdef XRENDER

    rvinfo.depth = 32;
    flags = VisualClassMask | VisualScreenMask | VisualDepthMask;

    vinfo = XGetVisualInfo(dpy, flags, &rvinfo, &numvis);
    if (vinfo)
      {
        for (i=numvis-1, best = -1; i>=0; i--)
          {
	    XRenderPictFormat* pictFormat = 
		XRenderFindVisualFormat (dpy, vinfo[i].visual);
	    if (NULL != pictFormat
		&& (pictFormat->type == PictTypeDirect)
		&& (pictFormat->direct.alphaMask))
	      {
		best = i;	
	      }
          }
      }

    #endif

    if (best == -1)
      { 
        flags = VisualClassMask | VisualScreenMask;
        vinfo = XGetVisualInfo(dpy, flags, &rvinfo, &numvis);
        if (vinfo) 
          {     /* look for a TrueColor, 24-bit or more (pref 24) */
            for (i=numvis-1, best = -1; i>=0; i--) 
              {
	        if (vinfo[i].depth == 24) best = i;
	        else if (vinfo[i].depth>24 && best<0) best = i;
              }
          }
      }

#if 0
    if (best == -1) {   /* look for a DirectColor, 24-bit or more (pref 24) */
	rvinfo.class = DirectColor;
	if (vinfo) XFree((char *) vinfo);
	vinfo = XGetVisualInfo(dpy, flags, &rvinfo, &numvis);
	if (vinfo) {
	    for (i=0, best = -1; i<numvis; i++) {
		if (vinfo[i].depth == 24) best = i;
		else if (vinfo[i].depth>24 && best<0) best = i;
	    }
	}
    }
#endif
    if (best > -1) {
	context->visual = vinfo[best].visual;
	context->depth = vinfo[best].depth;
	context->vclass = vinfo[best].class;
	getColormap(context, screen_number);
	attr.colormap = context->cmap;
	attr.override_redirect = True;
	attr.border_pixel = 0;
	context->drawable =
	    XCreateWindow(dpy, RootWindow(dpy, screen_number),
			  1, 1, 1, 1, 0, context->depth,
			  CopyFromParent, context->visual,
			  CWBorderPixel|CWColormap|CWOverrideRedirect, &attr);
/*	XSetWindowColormap(dpy, context->drawable, context->cmap);*/
    }
    if (vinfo) XFree((char *) vinfo);

    if (best < 0)
	return False;
    else
	return True;
}
