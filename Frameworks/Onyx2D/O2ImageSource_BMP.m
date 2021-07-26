/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

/*  BMP decode is based on the public domain implementation by Sean Barrett  http://www.nothings.org/stb_image.c  V 1.00 */

#import <Onyx2D/O2ImageSource_BMP.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Image.h>
#import <assert.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned int   uint32;
typedef   signed int    int32;
typedef unsigned int   uint;

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4,
};

typedef unsigned char stbi_uc;
static int32 img_x, img_y;
static int img_n, img_out_n;

static uint8  *out;
static const uint8 *img_buffer, *img_buffer_end;


static char *failure_reason;
static int e(char *str)
{
   failure_reason = str;
   NSLog(@"BMP failure: %s",str);
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


static int get16le(void)
{
   int z = get8();
   return z + (get8() << 8);
}

static uint32 get32le(void)
{
   uint32 z = get16le();
   return z + (get16le() << 16);
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

static int bmp_test(void)
{
   int sz;
   if (get8() != 'B') return 0;
   if (get8() != 'M') return 0;
   get32le(); // discard filesize
   get16le(); // discard reserved
   get16le(); // discard reserved
   get32le(); // discard data offset
   sz = get32le();
   if (sz == 12 || sz == 40 || sz == 56 || sz == 108) return 1;
   return 0;
}

int      stbi_bmp_test_memory      (stbi_uc *buffer, int len)
{
   start_mem(buffer, len);
   return bmp_test();
}

// returns 0..31 for the highest set bit
static int high_bit(unsigned int z)
{
   int n=0;
   if (z == 0) return -1;
   if (z >= 0x10000) n += 16, z >>= 16;
   if (z >= 0x00100) n +=  8, z >>=  8;
   if (z >= 0x00010) n +=  4, z >>=  4;
   if (z >= 0x00004) n +=  2, z >>=  2;
   if (z >= 0x00002) n +=  1, z >>=  1;
   return n;
}

static int bitcount(unsigned int a)
{
   a = (a & 0x55555555) + ((a >>  1) & 0x55555555); // max 2
   a = (a & 0x33333333) + ((a >>  2) & 0x33333333); // max 4
   a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
   a = (a + (a >> 8)); // max 16 per 8 bits
   a = (a + (a >> 16)); // max 32 per 8 bits
   return a & 0xff;
}

static int shiftsigned(int v, int shift, int bits)
{
   int result;
   int z=0;

   if (shift < 0) v <<= -shift;
   else v >>= shift;
   result = v;

   z = bits;
   while (z < 8) {
      result += v >> z;
      z += bits;
   }
   return result;
}

static stbi_uc *bmp_load(int *x, int *y, int *comp, int req_comp)
{
   unsigned int mr=0,mg=0,mb=0,ma=0;
   stbi_uc pal[256][4];
   int psize=0,i,j,compress=0,width;
   int bpp, flip_vertically, pad, target, offset, hsz;
   if (get8() != 'B' || get8() != 'M') return ep("not BMP", "Corrupt BMP");
   get32le(); // discard filesize
   get16le(); // discard reserved
   get16le(); // discard reserved
   offset = get32le();
   hsz = get32le();
   if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108) return ep("unknown BMP", "BMP type not supported: unknown");
   failure_reason = "bad BMP";
   if (hsz == 12) {
      img_x = get16le();
      img_y = get16le();
   } else {
      img_x = get32le();
      img_y = get32le();
   }
   if (get16le() != 1) return 0;
   bpp = get16le();
   if (bpp == 1) return ep("monochrome", "BMP type not supported: 1-bit");
   flip_vertically = img_y > 0;
   img_y = abs(img_y);
   if (hsz == 12) {
      if (bpp < 24)
         psize = (offset - 14 - 24) / 3;
   } else {
      compress = get32le();
      if (compress == 1 || compress == 2) return ep("BMP RLE", "BMP type not supported: RLE");
      get32le(); // discard sizeof
      get32le(); // discard hres
      get32le(); // discard vres
      get32le(); // discard colorsused
      get32le(); // discard max important
      if (hsz == 40 || hsz == 56) {
         if (hsz == 56) {
            get32le();
            get32le();
            get32le();
            get32le();
         }
         if (bpp == 16 || bpp == 32) {
            mr = mg = mb = ma = 0;
            if (compress == 0) {
               if (bpp == 32) {
                  ma = 0xff << 24;
                  mr = 0xff << 16;
                  mg = 0xff <<  8;
                  mb = 0xff <<  0;
               } else {
                  mr = 31 << 10;
                  mg = 31 <<  5;
                  mb = 31 <<  0;
               }
            } else if (compress == 3) {
               mr = get32le();
               mg = get32le();
               mb = get32le();
               // not documented, but generated by photoshop and handled by mspaint
               if (mr == mg && mg == mb) {
                  // ?!?!?
                  return NULL;
               }
            } else
               return NULL;
         }
      } else {
         assert(hsz == 108);
         mr = get32le();
         mg = get32le();
         mb = get32le();
         ma = get32le();
         get32le(); // discard color space
         for (i=0; i < 12; ++i)
            get32le(); // discard color space parameters
      }
      if (bpp < 16)
         psize = (offset - 14 - hsz) >> 2;
   }
   img_n = ma ? 4 : 3;
   if (req_comp && req_comp >= 3) // we can directly decode 3 or 4
      target = req_comp;
   else
      target = img_n; // if they want monochrome, we'll post-convert
   out = (stbi_uc *) NSZoneMalloc(NULL,target * img_x * img_y);
   if (!out) return ep("outofmem", "Out of memory");
   if (bpp < 16) {
      int z=0;
      if (psize == 0 || psize > 256) return ep("invalid", "Corrupt BMP");
      for (i=0; i < psize; ++i) {
         pal[i][2] = get8();
         pal[i][1] = get8();
         pal[i][0] = get8();
         if (hsz != 12) get8();
         pal[i][3] = 255;
      }
      skip(offset - 14 - hsz - psize * (hsz == 12 ? 3 : 4));
      if (bpp == 4) width = (img_x + 1) >> 1;
      else if (bpp == 8) width = img_x;
      else return ep("bad bpp", "Corrupt BMP");
      pad = (-width)&3;
      for (j=0; j < (int) img_y; ++j) {
         for (i=0; i < (int) img_x; i += 2) {
            int v=get8(),v2=0;
            if (bpp == 4) {
               v2 = v & 15;
               v >>= 4;
            }
            out[z++] = pal[v][0];
            out[z++] = pal[v][1];
            out[z++] = pal[v][2];
            if (target == 4) out[z++] = 255;
            if (i+1 == (int) img_x) break;
            v = (bpp == 8) ? get8() : v2;
            out[z++] = pal[v][0];
            out[z++] = pal[v][1];
            out[z++] = pal[v][2];
            if (target == 4) out[z++] = 255;
         }
         skip(pad);
      }
   } else {
      int rshift=0,gshift=0,bshift=0,ashift=0,rcount=0,gcount=0,bcount=0,acount=0;
      int z = 0;
      int easy=0;
      skip(offset - 14 - hsz);
      if (bpp == 24) width = 3 * img_x;
      else if (bpp == 16) width = 2*img_x;
      else /* bpp = 32 and pad = 0 */ width=0;
      pad = (-width) & 3;
      if (bpp == 24) {
         easy = 1;
      } else if (bpp == 32) {
         if (mb == 0xff && mg == 0xff00 && mr == 0xff000000 && ma == 0xff000000)
            easy = 2;
      }
      if (!easy) {
         if (!mr || !mg || !mb) return ep("bad masks", "Corrupt BMP");
         // right shift amt to put high bit in position #7
         rshift = high_bit(mr)-7; rcount = bitcount(mr);
         gshift = high_bit(mg)-7; gcount = bitcount(mr);
         bshift = high_bit(mb)-7; bcount = bitcount(mr);
         ashift = high_bit(ma)-7; acount = bitcount(mr);
      }
      for (j=0; j < (int) img_y; ++j) {
         if (easy) {
            for (i=0; i < (int) img_x; ++i) {
               int a;
               out[z+2] = get8();
               out[z+1] = get8();
               out[z+0] = get8();
               z += 3;
               a = (easy == 2 ? get8() : 255);
               if (target == 4) out[z++] = a;
            }
         } else {
            for (i=0; i < (int) img_x; ++i) {
               unsigned long v = (bpp == 16 ? get16le() : get32le());
               int a;
               out[z++] = shiftsigned(v & mr, rshift, rcount);
               out[z++] = shiftsigned(v & mg, gshift, gcount);
               out[z++] = shiftsigned(v & mb, bshift, bcount);
               a = (ma ? shiftsigned(v & ma, ashift, acount) : 255);
               if (target == 4) out[z++] = a; 
            }
         }
         skip(pad);
      }
   }
   if (flip_vertically) {
      stbi_uc t;
      for (j=0; j < (int) img_y>>1; ++j) {
         stbi_uc *p1 = out +      j     *img_x*target;
         stbi_uc *p2 = out + (img_y-1-j)*img_x*target;
         for (i=0; i < (int) img_x*target; ++i) {
            t = p1[i], p1[i] = p2[i], p2[i] = t;
         }
      }
   }

   if (req_comp && req_comp != target) {
      out = convert_format(out, target, req_comp);
      if (out == NULL) return out; // convert_format frees input on failure
   }

   *x = img_x;
   *y = img_y;
   if (comp) *comp = target;
   return out;
}

stbi_uc *stbi_bmp_load_from_memory (const stbi_uc *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   start_mem(buffer, len);
   return bmp_load(x,y,comp,req_comp);
}

@implementation O2ImageSource_BMP

+(BOOL)isPresentInDataProvider:(O2DataProvider *)provider {
   enum { signatureLength=2 };
   unsigned char signature[signatureLength] = { 'B','M' };
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
   _bmp=(NSData *)O2DataProviderCopyData(provider);
   return self;
}

-(void)dealloc {
   [_bmp release];
   [super dealloc];
}

- (CFStringRef)type
{
    return (CFStringRef)@"com.microsoft.bmp";
}

-(unsigned)count {
   return 1;
}

-(O2Image *)createImageAtIndex:(unsigned)index options:(NSDictionary *)options {
   int            width,height;
   int            comp;
   unsigned char *pixels=stbi_bmp_load_from_memory([_bmp bytes],[_bmp length],&width,&height,&comp,STBI_rgb_alpha);
   int            bitsPerPixel=32;
   int            bytesPerRow=(bitsPerPixel/(sizeof(char)*8))*width;
   NSData        *bitmap;
   
   if(pixels==NULL)
    return nil;

   bitmap=[[NSData alloc] initWithBytesNoCopy:pixels length:bytesPerRow*height];

    O2DataProvider *provider=O2DataProviderCreateWithCFData((CFDataRef)bitmap);
   O2ColorSpace   *colorSpace=O2ColorSpaceCreateDeviceRGB();
    O2Image        *image=[[O2Image alloc] initWithWidth:width height:height bitsPerComponent:8 bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow
                                              colorSpace:colorSpace bitmapInfo:kO2BitmapByteOrder32Big|kO2ImageAlphaPremultipliedLast decoder:NULL
                                                provider:provider decode:NULL interpolate:NO renderingIntent:kO2RenderingIntentDefault];
      
   [colorSpace release];
   [provider release];
   [bitmap release];
   
   return image;
}


@end
