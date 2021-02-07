/*
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

/*
 * Environment variables:
 * 
 * WRASTER_GAMMA <rgamma>/<ggamma>/<bgamma>
 * gamma correction value. Must be  greater than 0
 * Only for PseudoColor visuals.
 * 
 * Default:
 * WRASTER_GAMMA 1/1/1
 * 
 * 
 * If you want a specific value for a screen, append the screen number
 * preceded by a hash to the variable name as in
 * WRASTER_GAMMA#1
 * for screen number 1
 */

#ifndef RLRASTER_H_
#define RLRASTER_H_


/* version of the header for the library: 0.21 */
#define WRASTER_HEADER_VERSION	21


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XSHM
#include <X11/extensions/XShm.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* RBestMatchRendering or RDitheredRendering */
#define RC_RenderMode 		(1<<0)

/* number of colors per channel for colormap in PseudoColor mode */
#define RC_ColorsPerChannel	(1<<1)

/* do gamma correction */
#define RC_GammaCorrection	(1<<2)

/* visual id to use */
#define RC_VisualID		(1<<3)

/* shared memory usage */
#define RC_UseSharedMemory	(1<<4)

/* use default instead of best visual */
#define RC_DefaultVisual	(1<<5)

/* filter type for smoothed scaling */
#define RC_ScalingFilter	(1<<6)

/* standard colormap usage */
#define RC_StandardColormap	(1<<7)

    
    

    
    
/* std colormap usage/creation modes */
enum {
    RUseStdColormap,		       /* default. fallbacks to RIgnore.. if 
					there is none defined */
	RCreateStdColormap,
	RIgnoreStdColormap
};
    

    
typedef struct RContextAttributes {
    int flags;
    int render_mode;
    int colors_per_channel;	       /* for PseudoColor */
    float rgamma;		       /* gamma correction for red, */
    float ggamma;		       /* green, */
    float bgamma;		       /* and blue */
    VisualID visualid;		       /* visual ID to use */
    int use_shared_memory;	       /* True of False */
    int scaling_filter;
    int standard_colormap_mode;	       /* what to do with std cma */
} RContextAttributes;


/*
 * describes a screen in terms of depth, visual, number of colors
 * we can use, if we should do dithering, and what colors to use for
 * dithering.
 */
typedef struct RContext {
    Display *dpy;
    int screen_number;
    Colormap cmap;
    
    RContextAttributes *attribs;

    GC copy_gc;

    Visual *visual;
    int depth;
    Window drawable;		       /* window to pass for XCreatePixmap().*/
				       /* generally = root */
    int vclass;
    
    unsigned long black;
    unsigned long white;

    int red_offset;		       /* only used in 24bpp */
    int green_offset;
    int blue_offset;

    /* only used for pseudocolor and grayscale */

    XStandardColormap *std_rgb_map;    /* standard RGB colormap */
    XStandardColormap *std_gray_map;   /* standard grayscale colormap */
    
    int ncolors;		       /* total number of colors we can use */
    XColor *colors;		       /* internal colormap */
    unsigned long *pixels;	       /* RContext->colors[].pixel */

    struct {
	unsigned int use_shared_pixmap:1;
	unsigned int optimize_for_speed:1;
    } flags;
    
    struct RHermesData *hermes_data;   /* handle for Hermes stuff */
} RContext;


typedef struct RColor {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
} RColor;


typedef struct RHSVColor {
    unsigned short hue;		       /* 0-359 */
    unsigned char saturation;	       /* 0-255 */
    unsigned char value;	       /* 0-255 */
} RHSVColor;



typedef struct RPoint {
    int x, y;
} RPoint;


typedef struct RSegment {
    int x1, y1, x2, y2;
} RSegment;

    

/* image formats */
enum RImageFormat {
    RRGBFormat,
    RRGBAFormat
};


/*
 * internal 24bit+alpha image representation
 */
typedef struct RImage {
    unsigned char *data;       /* image data RGBA or RGB */
    int width, height;	       /* size of the image */
    enum RImageFormat format;
    RColor background;	       /* background color */
    int refCount;
} RImage;


/*
 * internal wrapper for XImage. Used for shm abstraction
 */
typedef struct RXImage {
    XImage *image;

    /* Private data. Do not access */
#ifdef XSHM
    XShmSegmentInfo info;
    char is_shared;
#endif
} RXImage;


/* image display modes */
enum {
    RDitheredRendering = 0,
    RBestMatchRendering = 1
};


/* smoothed scaling filter types */
enum {
    RBoxFilter,
	RTriangleFilter,
	RBellFilter,
	RBSplineFilter,
	RLanczos3Filter,
	RMitchellFilter
};


/* note that not all operations are supported in all functions */
enum {
    RClearOperation,		       /* clear with 0 */
	RCopyOperation,
	RNormalOperation,	       /* same as combine */
	RAddOperation,
	RSubtractOperation
};


enum {
    RAbsoluteCoordinates = 0,
    RRelativeCoordinates = 1
};


enum {
    RSunkenBevel	= -1,
    RNoBevel		= 0,
    RRaisedBevel	= 1    
};
/* bw compat */
#define RBEV_SUNKEN	RSunkenBevel
/* 1 pixel wide */
#define RBEV_RAISED	RRaisedBevel
/* 1 pixel wide on top/left 2 on bottom/right */
#define RBEV_RAISED2	2
/* 2 pixel width */
#define RBEV_RAISED3	3

enum {
    RHorizontalGradient = 2,
	RVerticalGradient = 3,
	RDiagonalGradient = 4
};
/* for backwards compatibility */
#define RGRD_HORIZONTAL  RHorizontalGradient
#define RGRD_VERTICAL 	RVerticalGradient
#define RGRD_DIAGONAL	RDiagonalGradient



/* error codes */
#define RERR_NONE		0
#define RERR_OPEN	 	1      /* cant open file */
#define RERR_READ		2      /* error reading from file */
#define RERR_WRITE		3      /* error writing to file */
#define RERR_NOMEMORY		4      /* out of memory */
#define RERR_NOCOLOR		5      /* out of color cells */
#define RERR_BADIMAGEFILE	6      /* image file is corrupted or invalid */
#define RERR_BADFORMAT		7      /* image file format is unknown */
#define RERR_BADINDEX		8      /* no such image index in file */

#define RERR_BADVISUALID	16     /* invalid visual ID requested for context */
#define RERR_STDCMAPFAIL	17     /* failed to created std colormap */
    
#define RERR_XERROR		127    /* internal X error */
#define RERR_INTERNAL		128    /* should not happen */


/*
 * Returns a NULL terminated array of strings containing the
 * supported formats, such as: TIFF, XPM, PNG, JPEG, PPM, GIF
 * Do not free the returned data.
 */
char **RSupportedFileFormats(void);


char *RGetImageFileFormat(char *file);

/*
 * Xlib contexts
 */
RContext *RCreateContext(Display *dpy, int screen_number,
			 RContextAttributes *attribs);

void RDestroyContext(RContext *context);

Bool RGetClosestXColor(RContext *context, RColor *color, XColor *retColor);

/*
 * RImage creation
 */
RImage *RCreateImage(unsigned width, unsigned height, int alpha);

RImage *RCreateImageFromXImage(RContext *context, XImage *image, XImage *mask);

RImage *RCreateImageFromDrawable(RContext *context, Drawable drawable,
				 Pixmap mask);

RImage *RLoadImage(RContext *context, char *file, int index);

RImage* RRetainImage(RImage *image);

void RReleaseImage(RImage *image);

/* Obsoleted function. Use RReleaseImage() instead. This was kept only to
 * allow a smoother transition and to avoid breaking existing programs, but
 * it will be removed in a future release. Right now is just an alias to
 * RReleaseImage(). Do _NOT_ use RDestroyImage() anymore in your programs.
 * Being an alias to RReleaseImage() this function no longer actually
 * destroys the image, unless the image is no longer retained in some other
 * place.
 */
void RDestroyImage(RImage *image);

RImage *RGetImageFromXPMData(RContext *context, char **xpmData);

/*
 * RImage storing
 */
Bool RSaveImage(RImage *image, char *filename, char *format);

/*
 * Area manipulation
 */
RImage *RCloneImage(RImage *image);

RImage *RGetSubImage(RImage *image, int x, int y, unsigned width, 
		     unsigned height);

void RCombineImageWithColor(RImage *image, RColor *color);

void RCombineImages(RImage *image, RImage *src);

void RCombineArea(RImage *image, RImage *src, int sx, int sy, unsigned width,
		 unsigned height, int dx, int dy);

void RCombineImagesWithOpaqueness(RImage *image, RImage *src, int opaqueness);

void RCombineAreaWithOpaqueness(RImage *image, RImage *src, int sx, int sy, 
			       unsigned width, unsigned height, int dx, int dy,
			       int opaqueness);

RImage *RScaleImage(RImage *image, unsigned new_width, unsigned new_height);

RImage *RSmoothScaleImage(RImage *src, unsigned new_width, 
			  unsigned new_height);

RImage *RRotateImage(RImage *image, float angle);
    

RImage *RMakeTiledImage(RImage *tile, unsigned width, unsigned height);

RImage* RMakeCenteredImage(RImage *image, unsigned width, unsigned height,
                           RColor *color);

/*
 * Drawing
 */
Bool RGetPixel(RImage *image, int x, int y, RColor *color);

void RPutPixel(RImage *image, int x, int y, RColor *color);

void ROperatePixel(RImage *image, int operation, int x, int y, RColor *color);

void RPutPixels(RImage *image, RPoint *points, int npoints, int mode, 
		RColor *color);

void ROperatePixels(RImage *image, int operation, RPoint *points, 
		    int npoints, int mode, RColor *color);

int RDrawLine(RImage *image, int x0, int y0, int x1, int y1, RColor *color);

int ROperateLine(RImage *image, int operation, int x0, int y0, int x1, int y1,
		 RColor *color);

void RDrawLines(RImage *image, RPoint *points, int npoints, int mode, 
		RColor *color);

void ROperateLines(RImage *image, int operation, RPoint *points, int npoints,
		   int mode, RColor *color);

void RDrawSegments(RImage *image, RSegment *segs, int nsegs, RColor *color);

void ROperateSegments(RImage *image, int operation, RSegment *segs, int nsegs,
		      RColor *color);

/*
 * Color convertion
 */
void RRGBtoHSV(RColor *color, RHSVColor *hsv);
void RHSVtoRGB(RHSVColor *hsv, RColor *rgb);

/*
 * Painting
 */
void RClearImage(RImage *image, RColor *color);

void RFillImage(RImage *image, RColor *color);
    
void RBevelImage(RImage *image, int bevel_type);

RImage *RRenderGradient(unsigned width, unsigned height, RColor *from, 
			RColor *to, int style);


RImage *RRenderMultiGradient(unsigned width, unsigned height, RColor **colors, 
			     int style);


RImage *RRenderInterwovenGradient(unsigned width, unsigned height,
				  RColor colors1[2], int thickness1,
				  RColor colors2[2], int thickness2);


/*
 * Convertion into X Pixmaps
 */
int RConvertImage(RContext *context, RImage *image, Pixmap *pixmap);

int RConvertImageMask(RContext *context, RImage *image, Pixmap *pixmap,
		      Pixmap *mask, int threshold);


/*
 * misc. utilities
 */
RXImage *RCreateXImage(RContext *context, int depth,
		       unsigned width, unsigned height);

RXImage *RGetXImage(RContext *context, Drawable d, int x, int y,
		    unsigned width, unsigned height);

void RDestroyXImage(RContext *context, RXImage *ximage);

void RPutXImage(RContext *context, Drawable d, GC gc, RXImage *ximage, 
		int src_x, int src_y, int dest_x, int dest_y,
		unsigned width, unsigned height);
    
/* do not free the returned string! */
const char *RMessageForError(int errorCode);

int RBlurImage(RImage *image);

/****** Global Variables *******/

extern int RErrorCode;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
