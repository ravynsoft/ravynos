/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

/*  PNG decode is based on the public domain implementation by Sean Barrett  http://www.nothings.org/stb_image.c  V 1.00 */

#import <Onyx2D/O2ImageSource_PNG.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Onyx2D/O2zlib.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Image.h>

#import "O2Defines_libpng.h"
#import <assert.h>

// clang-format off
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned int   uint32;
typedef   signed int    int32;
typedef unsigned int   uint;

enum
{
   SCAN_load=0,
   SCAN_type,
   SCAN_header,
};

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4,
};

#if LIBPNG_PRESENT
#include <png.h>

typedef struct png_data_t {
	const uint8_t *data;
	int len;
} png_data_t;

static void png_read_data(png_structp pngPtr, png_bytep data, png_size_t length) {
    //Here we get our IO pointer back from the read struct.
    //This is the parameter we passed to the png_set_read_fn() function.
    //Our std::istream pointer.
    png_data_t *a = (png_data_t *)png_get_io_ptr(pngPtr);
    //Cast the pointer to std::istream* and read 'length' bytes into 'data'
    memcpy(data, a->data, length);
	a->data += length;
}

// Load an unpacked image into outData - RGBA 8 bits/pixels
// Adapted from libpng sample code
bool load_png_image(const unsigned char *buffer, int length, int *outWidth, int *outHeight, void **outData) {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
	
    /* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
									 NULL, NULL, NULL);
	
    if (png_ptr == NULL) {
        return false;
    }
	
    /* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }
	
    /* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        /* If we get here, we had a
         * problem reading the file */
        return false;
    }
	
    /* Set up the output control if
     * you are using standard C streams */
	png_data_t data = {
		.data = buffer,
		.len = length
	};
	png_set_read_fn(png_ptr,(png_voidp)&data, png_read_data);

    /* If we have already
     * read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);
	
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_SHIFT | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB, NULL);
	
	int nb_comp = png_get_channels(png_ptr, info_ptr);
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
	
	// Adjust the size to adding the alpha if needed
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	if (nb_comp == 3) {
		row_bytes += width; // Add some room for the alpha
	}
    *outData = (unsigned char*) malloc(row_bytes * height);
    if (*outData == NULL) {
        NSLog(@"Can't allocate %d bytes for %dx%d bitmap", row_bytes*height, width, height);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	
    for (int i = 0; i < height; i++) {
		if (nb_comp == 3) {
			// Add the alpha bytes
			uint8_t *src = row_pointers[i];
			uint8_t *dest = *outData+(row_bytes * i);
			for (int j = 0; j < width; ++j, src += 3, dest += 4) {
				dest[0]=src[0];
				dest[1]=src[1];
				dest[2]=src[2];
				dest[3]=0xff;
			}
		} else {
			// Just copy the bytes
			memcpy(*outData+(row_bytes * i), row_pointers[i], row_bytes);
		}
    }
	
	*outWidth = width;
	*outHeight = height;

    /* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
    /* That's it */
    return true;
}

// Note: we ignore req_comp - we always returns RGBA 8 bits/component
unsigned char *stbi_png_load_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
	void* result = NULL;
	if (comp) {
		*comp = 4;
	}
	if (load_png_image(buffer, len, x, y, &result) == NO) {
		result = NULL;
	}
	return result;
}

#else

static uint32 img_x, img_y;
static int img_n, img_out_n;

static const uint8 *img_buffer, *img_buffer_end;

static char *failure_reason;
static int e(char *str)
{
   failure_reason = str;
   NSLog(@"PNG failure: %s",str);
   return 0;
}

   #define e(x,y)  e(x)
#define ep(x,y)   (e(x,y),NULL)   

static void start_mem(const uint8 *buffer, int len)
{
   img_buffer = buffer;
   img_buffer_end = buffer+len;
}

static int get8(void)
{
   if (img_buffer < img_buffer_end)
      return *img_buffer++;
   return 0;
}

static uint8 get8u(void)
{
   return (uint8) get8();
}

static int get16(void)
{
   int z = get8();
   return (z << 8) + get8();
}

static uint32 get32(void)
{
   uint32 z = get16();
   return (z << 16) + get16();
}

static void skip(int n)
{
      img_buffer += n;
}

static uint8 compute_y(int r, int g, int b)
{
	return (uint8) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static unsigned char *convert_format(unsigned char *data, int img_n, int req_comp)
{
   uint i,j;
   unsigned char *good;

   if (req_comp == img_n) return data;
   assert(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) NSZoneMalloc(NULL,req_comp * img_x * img_y);
   if (good == NULL) {
      NSZoneFree(NULL,data);
      return ep("outofmem", "Out of memory");
   }

   for (j=0; j < img_y; ++j) {
      unsigned char *src  = data + j * img_x * img_n   ;
      unsigned char *dest = good + j * img_x * req_comp;

      #define COMBO(a,b)  ((a)*8+(b))
      #define CASE(a,b)   case COMBO(a,b): for(i=0; i < img_x; ++i, src += a, dest += b)

      // convert source image with img_n components to one with req_comp components
      switch(COMBO(img_n, req_comp)) {
         CASE(1,2) dest[0]=src[0], dest[1]=255; break;
         CASE(1,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(1,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=255; break;
         CASE(2,1) dest[0]=src[0]; break;
         CASE(2,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(2,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1]; break;
         CASE(3,4) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255; break;
         CASE(3,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(3,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = 255; break;
         CASE(4,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(4,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = src[3]; break;
         CASE(4,3) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2]; break;
         default: assert(0);
      }
      #undef CASE
   }

   NSZoneFree(NULL,data);
   img_out_n = req_comp;
   return good;
}

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding


typedef struct
{
   unsigned long length;
   unsigned long type;
} chunk;

#define PNG_TYPE(a,b,c,d)  (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

static chunk get_chunk_header(void)
{
   chunk c;
   c.length = get32();
   c.type   = get32();
   return c;
}

static int check_png_header(void)
{
   static uint8 png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
      if (get8() != png_sig[i]) return e("bad png sig","Not a PNG");
   return 1;
}

static uint8 *idata, *expanded, *out;

enum {
   F_none=0, F_sub=1, F_up=2, F_avg=3, F_paeth=4,
   F_avg_first, F_paeth_first,
};

static uint8 first_row_filter[5] =
{
   F_none, F_sub, F_none, F_avg_first, F_paeth_first
};

static int paeth(int a, int b, int c)
{
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

// create the png data from post-deflated data
static int create_png_image(uint8 *raw, uint32 raw_len, int out_n)
{
   uint32 i,j,stride = img_x*out_n;
   int k;
   assert(out_n == img_n || out_n == img_n+1);
   out = (uint8 *) NSZoneMalloc(NULL,img_x * img_y * out_n);
   if (!out) return e("outofmem", "Out of memory");
   if (raw_len != (img_n * img_x + 1) * img_y) return e("not enough pixels","Corrupt PNG");
   for (j=0; j < img_y; ++j) {
      uint8 *cur = out + stride*j;
      uint8 *prior = cur - stride;
      int filter = *raw++;
      if (filter > 4) return e("invalid filter","Corrupt PNG");
      // if first row, use special filter that doesn't sample previous row
      if (j == 0) filter = first_row_filter[filter];
      // handle first pixel explicitly
      for (k=0; k < img_n; ++k) {
         switch(filter) {
            case F_none       : cur[k] = raw[k]; break;
            case F_sub        : cur[k] = raw[k]; break;
            case F_up         : cur[k] = raw[k] + prior[k]; break;
            case F_avg        : cur[k] = raw[k] + (prior[k]>>1); break;
            case F_paeth      : cur[k] = (uint8) (raw[k] + paeth(0,prior[k],0)); break;
            case F_avg_first  : cur[k] = raw[k]; break;
            case F_paeth_first: cur[k] = raw[k]; break;
         }
      }
      if (img_n != out_n) cur[img_n] = 255;
      raw += img_n;
      cur += out_n;
      prior += out_n;
      // this is a little gross, so that we don't switch per-pixel or per-component
      if (img_n == out_n) {
         #define CASE(f) \
             case f:     \
                for (i=1; i < img_x; ++i, raw+=img_n,cur+=img_n,prior+=img_n) \
                   for (k=0; k < img_n; ++k)
         switch(filter) {
            CASE(F_none)  cur[k] = raw[k]; break;
            CASE(F_sub)   cur[k] = raw[k] + cur[k-img_n]; break;
            CASE(F_up)    cur[k] = raw[k] + prior[k]; break;
            CASE(F_avg)   cur[k] = raw[k] + ((prior[k] + cur[k-img_n])>>1); break;
            CASE(F_paeth)  cur[k] = (uint8) (raw[k] + paeth(cur[k-img_n],prior[k],prior[k-img_n])); break;
            CASE(F_avg_first)    cur[k] = raw[k] + (cur[k-img_n] >> 1); break;
            CASE(F_paeth_first)  cur[k] = (uint8) (raw[k] + paeth(cur[k-img_n],0,0)); break;
         }
         #undef CASE
      } else {
         assert(img_n+1 == out_n);
         #define CASE(f) \
             case f:     \
                for (i=1; i < img_x; ++i, cur[img_n]=255,raw+=img_n,cur+=out_n,prior+=out_n) \
                   for (k=0; k < img_n; ++k)
         switch(filter) {
            CASE(F_none)  cur[k] = raw[k]; break;
            CASE(F_sub)   cur[k] = raw[k] + cur[k-out_n]; break;
            CASE(F_up)    cur[k] = raw[k] + prior[k]; break;
            CASE(F_avg)   cur[k] = raw[k] + ((prior[k] + cur[k-out_n])>>1); break;
            CASE(F_paeth)  cur[k] = (uint8) (raw[k] + paeth(cur[k-out_n],prior[k],prior[k-out_n])); break;
            CASE(F_avg_first)    cur[k] = raw[k] + (cur[k-out_n] >> 1); break;
            CASE(F_paeth_first)  cur[k] = (uint8) (raw[k] + paeth(cur[k-out_n],0,0)); break;
         }
         #undef CASE
      }
   }
   return 1;
}

static int compute_transparency(uint8 tc[3], int out_n)
{
   uint32 i, pixel_count = img_x * img_y;
   uint8 *p = out;

   // compute color-based transparency, assuming we've
   // already got 255 as the alpha value in the output
   assert(out_n == 2 || out_n == 4);

   p = out;
   if (out_n == 2) {
      for (i=0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 255);
         p += 2;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int expand_palette(uint8 *palette, int len, int pal_img_n)
{
   uint32 i, pixel_count = img_x * img_y;
   uint8 *p, *temp_out, *orig = out;

   p = (uint8 *) NSZoneMalloc(NULL,pixel_count * pal_img_n);
   if (p == NULL) return e("outofmem", "Out of memory");

   // between here and free(out) below, exitting would leak
   temp_out = p;

   if (pal_img_n == 3) {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p += 3;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p[3] = palette[n+3];
         p += 4;
      }
   }
   NSZoneFree(NULL,out);
   out = temp_out;
   return 1;
}

static int parse_png_file(int scan, int req_comp)
{
   uint8 palette[1024], pal_img_n=0;
   uint8 has_trans=0, tc[3];
   uint32 ioff=0, idata_limit=0, i, pal_len=0;
   int first=1,k;

   if (!check_png_header()) return 0;

   if (scan == SCAN_type) return 1;
   for(;;first=0) {
      chunk c = get_chunk_header();
      if (first && c.type != PNG_TYPE('I','H','D','R'))
         return e("first not IHDR","Corrupt PNG");
      switch (c.type) {
         case PNG_TYPE('I','H','D','R'): {
            int depth,color,interlace,comp,filter;
            if (!first) return e("multiple IHDR","Corrupt PNG");
            if (c.length != 13) return e("bad IHDR len","Corrupt PNG");
            img_x = get32(); if (img_x > (1 << 24)) return e("too large","Very large image (corrupt?)");
            img_y = get32(); if (img_y > (1 << 24)) return e("too large","Very large image (corrupt?)");

            depth = get8();  if (depth != 8)        return e("8bit only","PNG not supported: 8-bit only");
            color = get8();  if (color > 6)         return e("bad ctype","Corrupt PNG");
            if (color == 3) pal_img_n = 3; else if (color & 1) return e("bad ctype","Corrupt PNG");
            comp  = get8();  if (comp) return e("bad comp method","Corrupt PNG");
            filter= get8();  if (filter) return e("bad filter method","Corrupt PNG");
            interlace = get8(); if (interlace) return e("interlaced","PNG not supported: interlaced mode");
            if (!img_x || !img_y) return e("0-pixel image","Corrupt PNG");
            if (!pal_img_n) {
               img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
               if ((1 << 30) / img_x / img_n < img_y) return e("too large", "Image too large to decode");
               if (scan == SCAN_header) return 1;
            } else {
               // if paletted, then pal_n is our final components, and
               // img_n is # components to decompress/filter.
               img_n = 1;
               if ((1 << 30) / img_x / 4 < img_y) return e("too large","Corrupt PNG");
               // if SCAN_header, have to scan to see if we have a tRNS
            }
            break;
         }

         case PNG_TYPE('P','L','T','E'):  {
            if (c.length > 256*3) return e("invalid PLTE","Corrupt PNG");
            pal_len = c.length / 3;
            if (pal_len * 3 != c.length) return e("invalid PLTE","Corrupt PNG");
            for (i=0; i < pal_len; ++i) {
               palette[i*4+0] = get8u();
               palette[i*4+1] = get8u();
               palette[i*4+2] = get8u();
               palette[i*4+3] = 255;
            }
            break;
         }

         case PNG_TYPE('t','R','N','S'): {
            if (idata) return e("tRNS after IDAT","Corrupt PNG");
            if (pal_img_n) {
               if (scan == SCAN_header) { img_n = 4; return 1; }
               if (pal_len == 0) return e("tRNS before PLTE","Corrupt PNG");
               if (c.length > pal_len) return e("bad tRNS len","Corrupt PNG");
               pal_img_n = 4;
               for (i=0; i < c.length; ++i)
                  palette[i*4+3] = get8u();
            } else {
               if (!(img_n & 1)) return e("tRNS with alpha","Corrupt PNG");
               if (c.length != (uint32) img_n*2) return e("bad tRNS len","Corrupt PNG");
               has_trans = 1;
               for (k=0; k < img_n; ++k)
                  tc[k] = (uint8) get16(); // non 8-bit images will be larger
            }
            break;
         }

         case PNG_TYPE('I','D','A','T'): {
            if (pal_img_n && !pal_len) return e("no PLTE","Corrupt PNG");
            if (scan == SCAN_header) { img_n = pal_img_n; return 1; }
            if (ioff + c.length > idata_limit) {
               uint8 *p;
               if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
               while (ioff + c.length > idata_limit)
                  idata_limit *= 2;

               p = (uint8 *) NSZoneRealloc(NULL,idata, idata_limit); if (p == NULL) return e("outofmem", "Out of memory");
               idata = p;
            }
            {
               memcpy(idata+ioff, img_buffer, c.length);
               img_buffer += c.length;
            }
            ioff += c.length;
            break;
         }

         case PNG_TYPE('I','E','N','D'): {
            uint32 raw_len;
            if (scan != SCAN_load) return 1;
            if (idata == NULL) return e("no IDAT","Corrupt PNG");
            expanded = (uint8 *) stbi_zlib_decode_malloc((unsigned char *) idata, ioff, (int *) &raw_len);
            if (expanded == NULL) return 0; // zlib should set error
            NSZoneFree(NULL,idata); idata = NULL;
            if ((req_comp == img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
               img_out_n = img_n+1;
            else
               img_out_n = img_n;
            if (!create_png_image(expanded, raw_len, img_out_n)) return 0;
            if (has_trans)
               if (!compute_transparency(tc, img_out_n)) return 0;
           if (pal_img_n) {
               // pal_img_n == 3 or 4
               img_n = pal_img_n; // record the actual colors we had
               img_out_n = pal_img_n;
               if (req_comp >= 3) img_out_n = req_comp;
               if (!expand_palette(palette, pal_len, img_out_n))
                  return 0;
            }
            NSZoneFree(NULL,expanded); expanded = NULL;
            return 1;
         }

         default:
            // if critical, fail
            if ((c.type & (1 << 29)) == 0) {
               #ifndef STBI_NO_FAILURE_STRINGS
               static char invalid_chunk[] = "XXXX chunk not known";
               invalid_chunk[0] = (uint8) (c.type >> 24);
               invalid_chunk[1] = (uint8) (c.type >> 16);
               invalid_chunk[2] = (uint8) (c.type >>  8);
               invalid_chunk[3] = (uint8) (c.type >>  0);
               #endif
               return e(invalid_chunk, "PNG not supported: unknown chunk type");
            }
            skip(c.length);
            break;
      }
      // end of chunk, read and skip CRC
      get32();
   }
}

static unsigned char *do_png(int *x, int *y, int *n, int req_comp)
{
   unsigned char *result=NULL;
   if (req_comp < 0 || req_comp > 4) return ep("bad req_comp", "Internal error");
   if (parse_png_file(SCAN_load, req_comp)) {
      result = out;
      out = NULL;
      if (req_comp && req_comp != img_out_n) {
         result = convert_format(result, img_out_n, req_comp);
         if (result == NULL) return result;
      }
      *x = img_x;
      *y = img_y;
      if (n) *n = img_n;
   }
   NSZoneFree(NULL,out);      out      = NULL;
   NSZoneFree(NULL,expanded); expanded = NULL;
   NSZoneFree(NULL,idata);    idata    = NULL;

   return result;
}

unsigned char *stbi_png_load_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   start_mem(buffer,len);
   return do_png(x,y,comp,req_comp);
}
#endif

// clang-format on

@implementation O2ImageSource_PNG

+(BOOL)isPresentInDataProvider:(O2DataProvider *)provider {
   enum { signatureLength=8 };
   unsigned char signature[signatureLength] = { 137,80,78,71,13,10,26,10 };
   unsigned char check[signatureLength];
   NSInteger     i,size=[provider getBytes:check range:NSMakeRange(0,signatureLength)];

   if(size!=signatureLength)
    return NO;
    
   for(i=0;i<signatureLength;i++)
    if(signature[i]!=check[i])
     return NO;
     
   return YES;
}

-initWithDataProvider:(O2DataProvider *)provider options:(NSDictionary *)options {
   [super initWithDataProvider:provider options:options];
   _png=(NSData *)O2DataProviderCopyData(provider);
   return self;
}

-(void)dealloc {
   [_png release];
   [super dealloc];
}

- (CFStringRef)type
{
    return (CFStringRef)@"public.png";
}

-(unsigned)count {
   return 1;
}

-(O2Image *)createImageAtIndex:(unsigned)index options:(CFDictionaryRef)options {
   int            width,height;
   int            comp;
   unsigned char *pixels=stbi_png_load_from_memory([_png bytes],[_png length],&width,&height,&comp,STBI_rgb_alpha);
   int            bitsPerPixel=32;
   int            bytesPerRow=(bitsPerPixel/(sizeof(char)*8))*width;
   NSData        *bitmap;
   
   if(pixels==NULL)
    return nil;
    
// clamp premultiplied data, this should probably be moved into the O2Image init
   int i;
   for(i=0;i<bytesPerRow*height;i+=4){
       unsigned char a=pixels[i+3];
       if (a != 0xff) {
           unsigned char r=pixels[i+0];
           unsigned char g=pixels[i+1];
           unsigned char b=pixels[i+2];
           
           pixels[i+0]=MIN(r,a);
           pixels[i+1]=MIN(g,a);
           pixels[i+2]=MIN(b,a);
       }
   }

   bitmap=[[NSData alloc] initWithBytesNoCopy:pixels length:bytesPerRow*height];

    O2DataProvider *provider=O2DataProviderCreateWithCFData((CFDataRef)bitmap);
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   O2Image *image=[[O2Image alloc] initWithWidth:width height:height bitsPerComponent:8 bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow
      colorSpace:colorSpace bitmapInfo:kO2ImageAlphaPremultipliedLast|kO2BitmapByteOrder32Big decoder:NULL provider:provider decode:NULL interpolate:NO renderingIntent:kO2RenderingIntentDefault];
      
   [colorSpace release];
   [provider release];
   [bitmap release];
   
   return image;
}


@end
