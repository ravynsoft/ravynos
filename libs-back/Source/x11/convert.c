/* convert.c - convert RImage to Pixmap
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

/* Problems:
 *   1. Using Grayscale visual with Dithering crashes wmaker
 *   2. Ghost dock/appicon is wrong in Pseudocolor, Staticgray, Grayscale
 */

#include <config.h>


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>


#ifdef BENCH
#include "bench.h"
#endif

#include "wrasterP.h"

#ifdef XSHM
extern Pixmap R_CreateXImageMappedPixmap(RContext *context, RXImage *ximage);

#endif


#define HAS_ALPHA(I)	((I)->format == RRGBAFormat)


typedef struct RConversionTable {
    unsigned short table[256];
    unsigned short index;

    struct RConversionTable *next;
} RConversionTable;


typedef struct RStdConversionTable {
    unsigned int table[256];

    unsigned short mult;
    unsigned short max;

    struct RStdConversionTable *next;
} RStdConversionTable;



static RConversionTable *conversionTable = NULL;
static RStdConversionTable *stdConversionTable = NULL;


static unsigned short*
computeTable(unsigned short mask)
{
    RConversionTable *tmp = conversionTable;
    int i;

    while (tmp) {
        if (tmp->index == mask)
            break;
        tmp = tmp->next;
    }

    if (tmp)
        return tmp->table;

    tmp = (RConversionTable *)malloc(sizeof(RConversionTable));
    if (tmp == NULL)
        return NULL;

    for (i=0;i<256;i++)
        tmp->table[i] = (i*mask + 0x7f)/0xff;

    tmp->index = mask;
    tmp->next = conversionTable;
    conversionTable = tmp;
    return tmp->table;
}


static unsigned int*
computeStdTable(unsigned int mult, unsigned int max)
{
    RStdConversionTable *tmp = stdConversionTable;
    unsigned int i;

    while (tmp) {
        if (tmp->mult == mult && tmp->max == max)
            break;
        tmp = tmp->next;
    }

    if (tmp)
        return tmp->table;

    tmp = (RStdConversionTable *)malloc(sizeof(RStdConversionTable));
    if (tmp == NULL)
        return NULL;

    for (i=0; i<256; i++) {
        tmp->table[i] = (i*max)/0xff * mult;
    }
    tmp->mult = mult;
    tmp->max = max;

    tmp->next = stdConversionTable;
    stdConversionTable = tmp;

    return tmp->table;
}

/***************************************************************************/


static void
convertTrueColor_generic(RXImage *ximg, RImage *image,
			 signed char *err, signed char *nerr,
			 const unsigned short *rtable, 
			 const unsigned short *gtable,
			 const unsigned short *btable,
			 const int dr, const int dg, const int db,
			 const unsigned short roffs,
			 const unsigned short goffs,
			 const unsigned short boffs)
{
    signed char *terr;
    int x, y, r, g, b;
    int pixel;
    int rer, ger, ber;
    unsigned char *ptr = image->data;
    int channels = (image->format == RRGBAFormat ? 4 : 3);

    /* convert and dither the image to XImage */
    for (y=0; y<image->height; y++) {
	nerr[0] = 0;
	nerr[1] = 0;
	nerr[2] = 0;
	for (x=0; x<image->width; x++, ptr+=channels) {

	    /* reduce pixel */
	    pixel = *ptr + err[x];
	    if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	    r = rtable[pixel];
	    /* calc error */
	    rer = pixel - r*dr;

	    /* reduce pixel */
	    pixel = *(ptr+1) + err[x+1];
	    if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	    g = gtable[pixel];
	    /* calc error */
	    ger = pixel - g*dg;

	    /* reduce pixel */
	    pixel = *(ptr+2) + err[x+2];
	    if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	    b = btable[pixel];
	    /* calc error */
	    ber = pixel - b*db;


	    pixel = (r<<roffs) | (g<<goffs) | (b<<boffs);
	    XPutPixel(ximg->image, x, y, pixel);

	    /* distribute error */
	    r = (rer*3)/8;
	    g = (ger*3)/8;
	    b = (ber*3)/8;
	    /* x+1, y */
	    err[x+3*1]+=r;
	    err[x+1+3*1]+=g;
	    err[x+2+3*1]+=b;
	    /* x, y+1 */
	    nerr[x]+=r;
	    nerr[x+1]+=g;
	    nerr[x+2]+=b;
	    /* x+1, y+1 */
	    nerr[x+3*1]=rer-2*r;
	    nerr[x+1+3*1]=ger-2*g;
	    nerr[x+2+3*1]=ber-2*b;
	}
	/* skip to next line */
	terr = err;
	err = nerr;
	nerr = terr;
    }
    
    /* redither the 1st line to distribute error better */
    ptr=image->data;
    y=0;
    nerr[0] = 0;
    nerr[1] = 0;
    nerr[2] = 0;
    for (x=0; x<image->width; x++, ptr+=channels) {
	
	/* reduce pixel */
	pixel = *ptr + err[x];
	if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	r = rtable[pixel];
	/* calc error */
	rer = pixel - r*dr;

	/* reduce pixel */
	pixel = *(ptr+1) + err[x+1];
	if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	g = gtable[pixel];
	/* calc error */
	ger = pixel - g*dg;
	
	/* reduce pixel */
	pixel = *(ptr+2) + err[x+2];
	if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	b = btable[pixel];
	/* calc error */
	ber = pixel - b*db;
	
	
	pixel = (r<<roffs) | (g<<goffs) | (b<<boffs);
	XPutPixel(ximg->image, x, y, pixel);
	
	/* distribute error */
	r = (rer*3)/8;
	g = (ger*3)/8;
	b = (ber*3)/8;
	/* x+1, y */
	err[x+3*1]+=r;
	err[x+1+3*1]+=g;
	err[x+2+3*1]+=b;
	/* x, y+1 */
	nerr[x]+=r;
	nerr[x+1]+=g;
	nerr[x+2]+=b;
	/* x+1, y+1 */
	nerr[x+3*1]=rer-2*r;
	nerr[x+1+3*1]=ger-2*g;
	nerr[x+2+3*1]=ber-2*b;
    }    
}




static RXImage*
image2TrueColor(RContext *ctx, RImage *image)
{
    RXImage *ximg;
    unsigned short rmask, gmask, bmask;
    unsigned short roffs, goffs, boffs;
    unsigned short *rtable, *gtable, *btable;
    int channels = (image->format == RRGBAFormat ? 4 : 3);

    ximg = RCreateXImage(ctx, ctx->depth, image->width, image->height);
    if (!ximg) {
	return NULL;
    }

    roffs = ctx->red_offset;
    goffs = ctx->green_offset;
    boffs = ctx->blue_offset;
    
    rmask = ctx->visual->red_mask >> roffs;
    gmask = ctx->visual->green_mask >> goffs;
    bmask = ctx->visual->blue_mask >> boffs;

    rtable = computeTable(rmask);
    gtable = computeTable(gmask);
    btable = computeTable(bmask);

    if (rtable==NULL || gtable==NULL || btable==NULL) {
	RErrorCode = RERR_NOMEMORY;
        RDestroyXImage(ctx, ximg);
        return NULL;
    }


#ifdef BENCH
    cycle_bench(1);
#endif

    if (ctx->attribs->render_mode==RBestMatchRendering) {
	int ofs, r, g, b;
	int x, y;
	unsigned long pixel;
	unsigned char *ptr = image->data;
	
        /* fake match */
#ifdef WR_DEBUG
        puts("true color match");
#endif
        if (rmask==0xff && gmask==0xff && bmask==0xff) {
            for (y=0; y < image->height; y++) {
                for (x=0; x < image->width; x++, ptr+=channels) {
                    /* reduce pixel */
                    pixel = (*(ptr)<<roffs) | (*(ptr+1)<<goffs) | (*(ptr+2)<<boffs);
                    XPutPixel(ximg->image, x, y, pixel);
                }
            }
        } else {
            for (y=0, ofs=0; y < image->height; y++) {
                for (x=0; x < image->width; x++, ofs+=channels-3) {
                    /* reduce pixel */
                    r = rtable[ptr[ofs++]];
                    g = gtable[ptr[ofs++]];
                    b = btable[ptr[ofs++]];
                    pixel = (r<<roffs) | (g<<goffs) | (b<<boffs);
                    XPutPixel(ximg->image, x, y, pixel);
                }
            }
        }
    } else {
        /* dither */
	const int dr=0xff/rmask;
	const int dg=0xff/gmask;
        const int db=0xff/bmask;

#ifdef WR_DEBUG
        puts("true color dither");
#endif	
	{
	    signed char *err;
	    signed char *nerr;
	    int ch = (image->format == RRGBAFormat ? 4 : 3);

	    err = malloc(ch*(image->width+2));
	    nerr = malloc(ch*(image->width+2));
	    if (!err || !nerr) {
		if (nerr)
		    free(nerr);
		if (err)
		    free(err);
		RErrorCode = RERR_NOMEMORY;
		RDestroyXImage(ctx, ximg);
		return NULL;
	    }
	    
	    memset(err, 0, ch*(image->width+2));
	    memset(nerr, 0, ch*(image->width+2));

	    convertTrueColor_generic(ximg, image, err, nerr, 
				     rtable, gtable, btable,
				     dr, dg, db, roffs, goffs, boffs);
	    free(err);
	    free(nerr);
	}

    }

#ifdef BENCH
    cycle_bench(0);
#endif

    return ximg;
}


/***************************************************************************/

static void
convertPseudoColor_to_8(RXImage *ximg, RImage *image,
			   signed char *err, signed char *nerr,
			   const unsigned short *rtable, 
			   const unsigned short *gtable,
			   const unsigned short *btable,
			   const int dr, const int dg, const int db,
			   unsigned long *pixels,
			   int cpc)
{
    signed char *terr;
    int x, y, r, g, b;
    int pixel;
    int rer, ger, ber;
    unsigned char *ptr = image->data;
    unsigned char *optr = (unsigned char *)ximg->image->data;
    int channels = (image->format == RRGBAFormat ? 4 : 3);
    int cpcpc = cpc*cpc;

    /* convert and dither the image to XImage */
    for (y=0; y<image->height; y++) {
	nerr[0] = 0;
	nerr[1] = 0;
	nerr[2] = 0;
	for (x=0; x<image->width*3; x+=3, ptr+=channels) {

	    /* reduce pixel */
	    pixel = *ptr + err[x];
	    if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	    r = rtable[pixel];
	    /* calc error */
	    rer = pixel - r*dr;

	    /* reduce pixel */
	    pixel = *(ptr+1) + err[x+1];
	    if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	    g = gtable[pixel];
	    /* calc error */
	    ger = pixel - g*dg;

	    /* reduce pixel */
	    pixel = *(ptr+2) + err[x+2];
	    if (pixel<0) pixel=0; else if (pixel>0xff) pixel=0xff;
	    b = btable[pixel];
	    /* calc error */
	    ber = pixel - b*db;

	    *optr++ = pixels[r*cpcpc + g*cpc + b];

	    /* distribute error */
	    r = (rer*3)/8;
	    g = (ger*3)/8;
	    b = (ber*3)/8;

	    /* x+1, y */
	    err[x+3*1]+=r;
	    err[x+1+3*1]+=g;
	    err[x+2+3*1]+=b;
	    /* x, y+1 */
	    nerr[x]+=r;
	    nerr[x+1]+=g;
	    nerr[x+2]+=b;
	    /* x+1, y+1 */
	    nerr[x+3*1]=rer-2*r;
	    nerr[x+1+3*1]=ger-2*g;
	    nerr[x+2+3*1]=ber-2*b;
	}
	/* skip to next line */
	terr = err;
	err = nerr;
	nerr = terr;
	
	optr += ximg->image->bytes_per_line - image->width;
    }
}



static RXImage*
image2PseudoColor(RContext *ctx, RImage *image)
{
    RXImage *ximg;
    register int x, y, r, g, b;
    unsigned char *ptr;
    unsigned long pixel;
    const int cpc=ctx->attribs->colors_per_channel;
    const unsigned short rmask = cpc-1; /* different sizes could be used */
    const unsigned short gmask = rmask; /* for r,g,b */
    const unsigned short bmask = rmask;
    unsigned short *rtable, *gtable, *btable;
    const int cpccpc = cpc*cpc;
    int channels = (image->format == RRGBAFormat ? 4 : 3);

    ximg = RCreateXImage(ctx, ctx->depth, image->width, image->height);
    if (!ximg) {
	return NULL;
    }

    ptr = image->data;

    /* Tables are same at the moment because rmask==gmask==bmask. */
    rtable = computeTable(rmask);
    gtable = computeTable(gmask);
    btable = computeTable(bmask);

    if (rtable==NULL || gtable==NULL || btable==NULL) {
	RErrorCode = RERR_NOMEMORY;
        RDestroyXImage(ctx, ximg);
        return NULL;
    }

    if (ctx->attribs->render_mode == RBestMatchRendering) {
        /* fake match */
#ifdef WR_DEBUG
        printf("pseudo color match with %d colors per channel\n", cpc);
#endif
	for (y=0; y<image->height; y++) {
	    for (x=0; x<image->width; x++, ptr+=channels-3) {
		/* reduce pixel */
                r = rtable[*ptr++];
                g = gtable[*ptr++];
                b = btable[*ptr++];
		pixel = r*cpccpc + g*cpc + b;
                /*data[ofs] = ctx->colors[pixel].pixel;*/
                XPutPixel(ximg->image, x, y, ctx->colors[pixel].pixel);
	    }
	}
    } else {
	/* dither */
	signed char *err;
	signed char *nerr;
	const int dr=0xff/rmask;
	const int dg=0xff/gmask;
	const int db=0xff/bmask;


#ifdef WR_DEBUG
        printf("pseudo color dithering with %d colors per channel\n", cpc);
#endif
	err = malloc(4*(image->width+3));
	nerr = malloc(4*(image->width+3));
	if (!err || !nerr) {
	    if (nerr)
		free(nerr);
	    if (err)
		free(err);
	    RErrorCode = RERR_NOMEMORY;
	    RDestroyXImage(ctx, ximg);
	    return NULL;
	}
	memset(err, 0, 4*(image->width+3));
	memset(nerr, 0, 4*(image->width+3));

	convertPseudoColor_to_8(ximg, image, err+4, nerr+4,
				rtable,	gtable,	btable,
				dr, dg, db, ctx->pixels, cpc);

	free(err);
	free(nerr);
    }
    
    return ximg;
}


/*
 * For standard colormap
 */
static RXImage*
image2StandardPseudoColor(RContext *ctx, RImage *image)
{
    RXImage *ximg;
    register int x, y, r, g, b;
    unsigned char *ptr;
    unsigned long pixel;
    unsigned char *data;
    unsigned int *rtable, *gtable, *btable;
    unsigned int base_pixel = ctx->std_rgb_map->base_pixel;
    int channels = (image->format == RRGBAFormat ? 4 : 3);


    ximg = RCreateXImage(ctx, ctx->depth, image->width, image->height);
    if (!ximg) {
	return NULL;
    }

    ptr = image->data;
    
    data = (unsigned char *)ximg->image->data;


    rtable = computeStdTable(ctx->std_rgb_map->red_mult,
			     ctx->std_rgb_map->red_max);

    gtable = computeStdTable(ctx->std_rgb_map->green_mult,
			     ctx->std_rgb_map->green_max);

    btable = computeStdTable(ctx->std_rgb_map->blue_mult,
			     ctx->std_rgb_map->blue_max);

    if (rtable==NULL || gtable==NULL || btable==NULL) {
	RErrorCode = RERR_NOMEMORY;
        RDestroyXImage(ctx, ximg);
        return NULL;
    }


    if (ctx->attribs->render_mode == RBestMatchRendering) {
	for (y=0; y<image->height; y++) {
	    for (x=0; x<image->width; x++, ptr+=channels) {
		/* reduce pixel */
		pixel = (rtable[*ptr] + gtable[*(ptr+1)]
			 + btable[*(ptr+2)] + base_pixel) & 0xffffffff;

                XPutPixel(ximg->image, x, y, pixel);
	    }
	}
    } else {
	/* dither */
	signed short *err, *nerr;
	signed short *terr;
	int rer, ger, ber;
	int x1, ofs;

#ifdef WR_DEBUG
        printf("pseudo color dithering with %d colors per channel\n", channels);
#endif
	err = (short*)malloc(3*(image->width+2)*sizeof(short));
	nerr = (short*)malloc(3*(image->width+2)*sizeof(short));
	if (!err || !nerr) {
	    if (err)
		free(err);
	    if (nerr)
		free(nerr);
	    RErrorCode = RERR_NOMEMORY;
	    RDestroyXImage(ctx, ximg);
	    return NULL;
	}
	for (x=0, x1=0; x<image->width*3; x1+=channels-3) {
	    err[x++] = ptr[x1++];
	    err[x++] = ptr[x1++];
	    err[x++] = ptr[x1++];
	}
        err[x] = err[x+1] = err[x+2] = 0;
	/* convert and dither the image to XImage */
	for (y=0, ofs=0; y<image->height; y++) {
	    if (y<image->height-1) {
		int x1;
		for (x=0, x1=(y+1)*image->width*channels;
		     x<image->width*3;
		     x1+=channels-3) {
		    nerr[x++] = ptr[x1++];
		    nerr[x++] = ptr[x1++];
		    nerr[x++] = ptr[x1++];
		}
		/* last column */
		x1-=channels;
		nerr[x++] = ptr[x1++];
		nerr[x++] = ptr[x1++];
		nerr[x] = ptr[x1];
	    }
	    for (x=0; x<image->width*3; x+=3, ofs++) {
		/* reduce pixel */
                if (err[x]>0xff) err[x]=0xff; else if (err[x]<0) err[x]=0;
                if (err[x+1]>0xff) err[x+1]=0xff; else if (err[x+1]<0) err[x+1]=0;
                if (err[x+2]>0xff) err[x+2]=0xff; else if (err[x+2]<0) err[x+2]=0;

                r = rtable[err[x]];
                g = gtable[err[x+1]];
                b = btable[err[x+2]];

		pixel = r + g + b;

                data[ofs] = base_pixel + pixel;

		/* calc error */
		rer = err[x] - (ctx->colors[pixel].red>>8);
		ger = err[x+1] - (ctx->colors[pixel].green>>8);
		ber = err[x+2] - (ctx->colors[pixel].blue>>8);

		/* distribute error */
		err[x+3*1]+=(rer*7)/16;
		err[x+1+3*1]+=(ger*7)/16;
		err[x+2+3*1]+=(ber*7)/16;

		nerr[x]+=(rer*5)/16;
		nerr[x+1]+=(ger*5)/16;
		nerr[x+2]+=(ber*5)/16;

		if (x>0) {
		    nerr[x-3*1]+=(rer*3)/16;
		    nerr[x-3*1+1]+=(ger*3)/16;
		    nerr[x-3*1+2]+=(ber*3)/16;
		}

		nerr[x+3*1]+=rer/16;
		nerr[x+1+3*1]+=ger/16;
		nerr[x+2+3*1]+=ber/16;
	    }
	    /* skip to next line */
	    terr = err;
	    err = nerr;
	    nerr = terr;

	    ofs += ximg->image->bytes_per_line - image->width;
	}
	free(err);
	free(nerr);
    }
    ximg->image->data = (char*)data;

    return ximg;
}



static RXImage*
image2GrayScale(RContext *ctx, RImage *image)
{
    RXImage *ximg;
    register int x, y, g;
    unsigned char *ptr;
    const int cpc=ctx->attribs->colors_per_channel;
    unsigned short gmask;
    unsigned short *table;
    unsigned char *data;
    int channels = (image->format == RRGBAFormat ? 4 : 3);


    ximg = RCreateXImage(ctx, ctx->depth, image->width, image->height);
    if (!ximg) {
	return NULL;
    }

    ptr = image->data;

    data = (unsigned char *)ximg->image->data;

    if (ctx->vclass == StaticGray)
	gmask = (1<<ctx->depth) - 1; /* use all grays */
    else
	gmask  = cpc*cpc*cpc-1;

    table = computeTable(gmask);

    if (table==NULL) {
	RErrorCode = RERR_NOMEMORY;
        RDestroyXImage(ctx, ximg);
        return NULL;
    }

    if (ctx->attribs->render_mode == RBestMatchRendering) {
        /* fake match */
#ifdef WR_DEBUG
        printf("grayscale match with %d colors per channel\n", cpc);
#endif
	for (y=0; y<image->height; y++) {
	    for (x=0; x<image->width; x++) {
                /* reduce pixel */
                g = table[(*ptr * 30 + *(ptr+1) * 59 + *(ptr+2) * 11)/100];
                ptr += channels;
                /*data[ofs] = ctx->colors[g].pixel;*/
                XPutPixel(ximg->image, x, y, ctx->colors[g].pixel);
	    }
	}
    } else {
	/* dither */
	short *gerr;
	short *ngerr;
	short *terr;
	int ger;
	const int dg=0xff/gmask;

#ifdef WR_DEBUG
        printf("grayscale dither with %d colors per channel\n", cpc);
#endif
	gerr = (short*)malloc((image->width+2)*sizeof(short));
	ngerr = (short*)malloc((image->width+2)*sizeof(short));
	if (!gerr || !ngerr) {
	    if (ngerr)
		free(ngerr);
	    if (gerr)
		free(gerr);
	    RErrorCode = RERR_NOMEMORY;
	    RDestroyXImage(ctx, ximg);
	    return NULL;
	}
	for (x=0, y=0; x<image->width; x++, y+=channels) {
	    gerr[x] = (ptr[y]*30 + ptr[y+1]*59 + ptr[y+2]*11)/100;
	}
	gerr[x] = 0;
	/* convert and dither the image to XImage */
	for (y=0; y<image->height; y++) {
	    if (y<image->height-1) {
		int x1;
		for (x=0, x1=(y+1)*image->width*channels; x<image->width; x++, x1+=channels) {
		    ngerr[x] = (ptr[x1]*30 + ptr[x1+1]*59 + ptr[x1+2]*11)/100;
		}
		/* last column */
		x1-=channels;
		ngerr[x] = (ptr[x1]*30 + ptr[x1+1]*59 + ptr[x1+2]*11)/100;
	    }
	    for (x=0; x<image->width; x++) {
		/* reduce pixel */
                if (gerr[x]>0xff) gerr[x]=0xff; else if (gerr[x]<0) gerr[x]=0;

                g = table[gerr[x]];

                /*data[ofs] = ctx->colors[g].pixel;*/
                XPutPixel(ximg->image, x, y, ctx->colors[g].pixel);
		/* calc error */
		ger = gerr[x] - g*dg;

		/* distribute error */
		g = (ger*3)/8;
		/* x+1, y */
		gerr[x+1]+=g;
		/* x, y+1 */
		ngerr[x]+=g;
		/* x+1, y+1 */
		ngerr[x+1]+=ger-2*g;
	    }
	    /* skip to next line */
	    terr = gerr;
	    gerr = ngerr;
	    ngerr = terr;
	}
	free(gerr);
	free(ngerr);
    }
    ximg->image->data = (char*)data;

    return ximg;
}


static RXImage*
image2Bitmap(RContext *ctx, RImage *image, int threshold)
{
    RXImage *ximg;
    unsigned char *alpha;
    int x, y;

    ximg = RCreateXImage(ctx, 1, image->width, image->height);
    if (!ximg) {
	return NULL;
    }
    alpha = image->data+3;
    
    for (y = 0; y < image->height; y++) {
	for (x = 0; x < image->width; x++) {
	    XPutPixel(ximg->image, x, y, (*alpha <= threshold ? 0 : 1));
	    alpha+=4;
	}
    }
    
    return ximg;
}


#ifdef HAVE_HERMES

static RXImage*
hermesConvert(RContext *context, RImage *image)
{
    HermesFormat source;
    HermesFormat dest;
    RXImage *ximage;
    
    
    ximage = RCreateXImage(context, context->depth, 
			   image->width, image->height);
    if (!ximage) {
	return NULL;
    }
    
    /* The masks look weird for images with alpha. but they work this way.
     * wth does hermes do internally?
     */
    source.bits = (HAS_ALPHA(image) ? 32 : 24);
    if (ximage->image->byte_order==LSBFirst) {
        source.r = 0xff0000;
        source.g = 0x00ff00;
        source.b = 0x0000ff;
    } else {
        if (source.bits == 32) {
            source.r = 0xff000000;
            source.g = 0x00ff0000;
            source.b = 0x0000ff00;
        } else {
            source.r = 0xff0000;
            source.g = 0x00ff00;
            source.b = 0x0000ff;
        }
    }
    source.a = 0; /* Don't care about alpha */
    source.indexed = 0;
    source.has_colorkey = 0;

    /* This is a hack and certainly looks weird, but it works :P
     * it assumes though that green is inbetween red and blue (the mask) */
    if (ximage->image->byte_order==LSBFirst) {
        dest.r = context->visual->blue_mask;
        dest.g = context->visual->green_mask;
        dest.b = context->visual->red_mask;
    } else {
        dest.r = context->visual->red_mask;
        dest.g = context->visual->green_mask;
        dest.b = context->visual->blue_mask;
    }
    dest.a = 0;
    dest.bits = ximage->image->bits_per_pixel;
    if (context->vclass == TrueColor)
	dest.indexed = 0;
    else
	dest.indexed = 1;
    dest.has_colorkey = 0;
    
    /*printf("source r=0x%x, g=0x%x, b=0x%x, a=0x%x, b=%d, i=%d, c=%d\n",
           source.r, source.g, source.b, source.a,
           source.bits, source.indexed, source.has_colorkey);
    printf("dest r=0x%x, g=0x%x, b=0x%x, a=0x%x, b=%d, i=%d, c=%d\n",
           dest.r, dest.g, dest.b, dest.a,
           dest.bits, dest.indexed, dest.has_colorkey);
    */

    Hermes_ConverterRequest(context->hermes_data->converter, &source, &dest);
    
    Hermes_ConverterPalette(context->hermes_data->converter, 
			    context->hermes_data->palette, 0);
    
    Hermes_ConverterCopy(context->hermes_data->converter,
			 image->data, 0, 0, image->width, image->height,
			 image->width * (image->format == RRGBFormat ? 3 : 4),
			 ximage->image->data, 0, 0, 
			 image->width, image->height,
			 ximage->image->bytes_per_line);
    
    return ximage;
}
#endif /* HAVE_HERMES */


int 
RConvertImage(RContext *context, RImage *image, Pixmap *pixmap)
{
    RXImage *ximg=NULL;
#ifdef XSHM
    Pixmap tmp;
#endif
    
    assert(context!=NULL);
    assert(image!=NULL);
    assert(pixmap!=NULL);

    switch (context->vclass) {
    case TrueColor:
#ifdef HAVE_HERMES
        if (context->attribs->render_mode == RBestMatchRendering) {
            ximg = hermesConvert(context, image);
        } else {
            ximg = image2TrueColor(context, image);
        }
#else /* !HAVE_HERMES */
        ximg = image2TrueColor(context, image);
#endif
	break;

    case PseudoColor:
    case StaticColor:
        /* For StaticColor we can also use hermes, but it doesn't dither */
#ifdef BENCH
	cycle_bench(1);
#endif
	if (context->attribs->standard_colormap_mode != RIgnoreStdColormap)
	    ximg = image2StandardPseudoColor(context, image);
	else
	    ximg = image2PseudoColor(context, image);
#ifdef BENCH
	cycle_bench(0);
#endif
	break;

    case GrayScale:
    case StaticGray:
	ximg = image2GrayScale(context, image);
	break;
    }


    if (!ximg) {
	return False;
    }

    *pixmap = XCreatePixmap(context->dpy, context->drawable, image->width,
			    image->height, context->depth);
    
#ifdef XSHM
    if (context->flags.use_shared_pixmap && ximg->is_shared)
        tmp = R_CreateXImageMappedPixmap(context, ximg);
    else
	tmp = None;
    if (tmp) {
	/*
	 * We have to copy the shm Pixmap into a normal Pixmap because
	 * otherwise, we would have to control when Pixmaps are freed so
	 * that we can detach their shm segments. This is a problem if the
	 * program crash, leaving stale shared memory segments in the
	 * system (lots of them). But with some work, we can optimize
	 * things and remove this XCopyArea. This will require
	 * explicitly freeing all pixmaps when exiting or restarting
	 * wmaker.
	 */
	XCopyArea(context->dpy, tmp, *pixmap, context->copy_gc, 0, 0,
		  image->width, image->height, 0, 0);
	XFreePixmap(context->dpy, tmp);
    } else {
	RPutXImage(context, *pixmap, context->copy_gc, ximg, 0, 0, 0, 0,
		   image->width, image->height);
    }
#else /* !XSHM */
    RPutXImage(context, *pixmap, context->copy_gc, ximg, 0, 0, 0, 0,
	       image->width, image->height);
#endif /* !XSHM */

    RDestroyXImage(context, ximg);

    return True;
}


int 
RConvertImageMask(RContext *context, RImage *image, Pixmap *pixmap, 
		  Pixmap *mask, int threshold)
{
    GC gc;
    XGCValues gcv;
    RXImage *ximg=NULL;

    assert(context!=NULL);
    assert(image!=NULL);
    assert(pixmap!=NULL);
    assert(mask!=NULL);

    if (!RConvertImage(context, image, pixmap))
	return False;
    
    if (image->format==RRGBFormat) {
	*mask = None;
	return True;
    }

    ximg = image2Bitmap(context, image, threshold);
    
    if (!ximg) {
	return False;
    }
    *mask = XCreatePixmap(context->dpy, context->drawable, image->width,
			  image->height, 1);
    gcv.foreground = context->black;
    gcv.background = context->white;
    gcv.graphics_exposures = False;
    gc = XCreateGC(context->dpy, *mask, GCForeground|GCBackground
		   |GCGraphicsExposures, &gcv);
    RPutXImage(context, *mask, gc, ximg, 0, 0, 0, 0,
	       image->width, image->height);
    RDestroyXImage(context, ximg);
    XFreeGC(context->dpy, gc);

    return True;
}


Bool
RGetClosestXColor(RContext *context, RColor *color, XColor *retColor)
{
    if (context->vclass == TrueColor) {
	unsigned short rmask, gmask, bmask;
	unsigned short roffs, goffs, boffs;
	unsigned short *rtable, *gtable, *btable;
    
	roffs = context->red_offset;
	goffs = context->green_offset;
	boffs = context->blue_offset;
    
	rmask = context->visual->red_mask >> roffs;
	gmask = context->visual->green_mask >> goffs;
	bmask = context->visual->blue_mask >> boffs;
	
	rtable = computeTable(rmask);
	gtable = computeTable(gmask);
	btable = computeTable(bmask);

        retColor->pixel = (rtable[color->red]<<roffs) |
	    (gtable[color->green]<<goffs) | (btable[color->blue]<<boffs);

	retColor->red = color->red << 8;
	retColor->green = color->green << 8;
	retColor->blue = color->blue << 8;
	retColor->flags = DoRed|DoGreen|DoBlue;

    } else if (context->vclass == PseudoColor
	       || context->vclass == StaticColor) {

	if (context->attribs->standard_colormap_mode != RIgnoreStdColormap) {
	    unsigned int *rtable, *gtable, *btable;

	    rtable = computeStdTable(context->std_rgb_map->red_mult,
				     context->std_rgb_map->red_max);

	    gtable = computeStdTable(context->std_rgb_map->green_mult,
				     context->std_rgb_map->green_max);

	    btable = computeStdTable(context->std_rgb_map->blue_mult,
				     context->std_rgb_map->blue_max);

	    if (rtable==NULL || gtable==NULL || btable==NULL) {
		RErrorCode = RERR_NOMEMORY;
		return False;
	    }

	    retColor->pixel = (rtable[color->red] 
			       + gtable[color->green]
			       + btable[color->blue] 
			       + context->std_rgb_map->base_pixel) & 0xffffffff;
	    retColor->red = color->red<<8;
	    retColor->green = color->green<<8;
	    retColor->blue = color->blue<<8;
	    retColor->flags = DoRed|DoGreen|DoBlue;
	    
	} else {
	    const int cpc=context->attribs->colors_per_channel;
	    const unsigned short rmask = cpc-1; /* different sizes could be used */
	    const unsigned short gmask = rmask; /* for r,g,b */
	    const unsigned short bmask = rmask;
	    unsigned short *rtable, *gtable, *btable;
	    const int cpccpc = cpc*cpc;
	    int index;

	    rtable = computeTable(rmask);
	    gtable = computeTable(gmask);
	    btable = computeTable(bmask);

	    if (rtable==NULL || gtable==NULL || btable==NULL) {
		RErrorCode = RERR_NOMEMORY;
		return False;
	    }
	    index = rtable[color->red]*cpccpc + gtable[color->green]*cpc 
		+ btable[color->blue];
	    *retColor = context->colors[index];
	}

    } else if (context->vclass == GrayScale || context->vclass == StaticGray) {

	const int cpc = context->attribs->colors_per_channel;
	unsigned short gmask;
	unsigned short *table;
	int index;

	if (context->vclass == StaticGray)
	    gmask = (1<<context->depth) - 1; /* use all grays */
	else
	    gmask  = cpc*cpc*cpc-1;

	table = computeTable(gmask);
	if (!table)
	    return False;

	index = table[(color->red*30 + color->green*59 + color->blue*11)/100];
	
	*retColor = context->colors[index];
    } else {
	RErrorCode = RERR_INTERNAL;
	return False;
    }

    return True;
}

