#import "O2ImageDecoder_JPEG_stb.h"

/*  JPEG decode is based on the public domain implementation by Sean Barrett  http://www.nothings.org/stb_image.c  V 1.14 */
// clang-format off
#define STBI_NO_STDIO

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned int   uint32;
typedef   signed int    int32;
typedef unsigned int   uint;

typedef struct
{
    uint32 img_x, img_y;
    int img_n, img_out_n;
    
#ifndef STBI_NO_STDIO
    FILE  *img_file;
#endif
    uint8 *img_buffer, *img_buffer_end;
} stbi;

// huffman decoding acceleration
#define FAST_BITS   9  // larger handles more cases; smaller stomps less cache

typedef struct
{
    uint8  fast[1 << FAST_BITS];
    // weirdly, repacking this into AoS is a 10% speed loss, instead of a win
    uint16 code[256];
    uint8  values[256];
    uint8  size[257];
    unsigned int maxcode[18];
    int    delta[17];   // old 'firstsymbol' - old 'firstcode'
} huffman;

typedef struct
{
#if STBI_SIMD
    unsigned short dequant2[4][64];
#endif
    stbi s;
    huffman huff_dc[4];
    huffman huff_ac[4];
    uint8 dequant[4][64];
    
    // sizes for components, interleaved MCUs
    int img_h_max, img_v_max;
    int img_mcu_x, img_mcu_y;
    int img_mcu_w, img_mcu_h;
    
    // definition of jpeg image component
    struct
    {
        int id;
        int h,v;
        int tq;
        int hd,ha;
        int dc_pred;
        
        int x,y,w2,h2;
        uint8 *data;
        void *raw_data;
        uint8 *linebuf;
    } img_comp[4];
    
    uint32         code_buffer; // jpeg entropy-coded buffer
    int            code_bits;   // number of valid bits
    unsigned char  marker;      // marker seen while filling entropy buffer
    int            nomore;      // flag if we saw a marker so must stop
    
    int scan_n, order[4];
    int restart_interval, todo;
} jpeg;

@implementation O2ImageDecoder_JPEG_stb
    
enum
{
	STBI_default = 0, // only used for req_comp
	
	STBI_grey       = 1,
	STBI_grey_alpha = 2,
	STBI_rgb        = 3,
	STBI_rgb_alpha  = 4,
};

typedef unsigned char stbi_uc;

// this is not threadsafe
static char *failure_reason;

static int e(char *str)
{
    failure_reason = str;
    return 0;
}

#ifdef STBI_NO_FAILURE_STRINGS
#define e(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
#define e(x,y)  e(y)
#else
#define e(x,y)  e(x)
#endif

#define epf(x,y)   ((float *) (e(x,y)?NULL:NULL))
#define epuc(x,y)  ((unsigned char *) (e(x,y)?NULL:NULL))

//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
    SCAN_load=0,
    SCAN_type,
    SCAN_header,
};


static void start_mem(stbi *s, uint8 const *buffer, int len)
{
    s->img_buffer = (uint8 *) buffer;
    s->img_buffer_end = (uint8 *) buffer+len;
}

static int get8(stbi *s)
{
    if (s->img_buffer < s->img_buffer_end)
        return *s->img_buffer++;
    return 0;
}

static int at_eof(stbi *s)
{
    return s->img_buffer >= s->img_buffer_end;   
}

static uint8 get8u(stbi *s)
{
    return (uint8) get8(s);
}

static void skip(stbi *s, int n)
{
    s->img_buffer += n;
}

static int get16(stbi *s)
{
    int z = get8(s);
    return (z << 8) + get8(s);
}


//////////////////////////////////////////////////////////////////////////////
//
//  "baseline" JPEG/JFIF decoder (not actually fully baseline implementation)
//
//    simple implementation
//      - channel subsampling of at most 2 in each dimension
//      - doesn't support delayed output of y-dimension
//      - simple interface (only one output format: 8-bit interleaved RGB)
//      - doesn't try to recover corrupt jpegs
//      - doesn't allow partial loading, loading multiple at once
//      - still fast on x86 (copying globals into locals doesn't help x86)
//      - allocates lots of intermediate memory (full size of all components)
//        - non-interleaved case requires this anyway
//        - allows good upsampling (see next)
//    high-quality
//      - upsampled channels are bilinearly interpolated, even across blocks
//      - quality integer IDCT derived from IJG's 'slow'
//    performance
//      - fast huffman; reasonable integer IDCT
//      - uses a lot of intermediate memory, could cache poorly
//      - load http://nothings.org/remote/anemones.jpg 3 times on 2.8Ghz P4
//          stb_jpeg:   1.34 seconds (MSVC6, default release build)
//          stb_jpeg:   1.06 seconds (MSVC6, processor = Pentium Pro)
//          IJL11.dll:  1.08 seconds (compiled by intel)
//          IJG 1998:   0.98 seconds (MSVC6, makefile provided by IJG)
//          IJG 1998:   0.95 seconds (MSVC6, makefile + proc=PPro)


static int build_huffman(huffman *h, int *count)
{
    int i,j,k=0,code;
    // build size list for each symbol (from JPEG spec)
    for (i=0; i < 16; ++i)
        for (j=0; j < count[i]; ++j)
            h->size[k++] = (uint8) (i+1);
    h->size[k] = 0;
    
    // compute actual symbols (from jpeg spec)
    code = 0;
    k = 0;
    for(j=1; j <= 16; ++j) {
        // compute delta to add to code to compute symbol id
        h->delta[j] = k - code;
        if (h->size[k] == j) {
            while (h->size[k] == j)
                h->code[k++] = (uint16) (code++);
            if (code-1 >= (1 << j)) return e("bad code lengths","Corrupt JPEG");
        }
        // compute largest code + 1 for this size, preshifted as needed later
        h->maxcode[j] = code << (16-j);
        code <<= 1;
    }
    h->maxcode[j] = 0xffffffff;
    
    // build non-spec acceleration table; 255 is flag for not-accelerated
    memset(h->fast, 255, 1 << FAST_BITS);
    for (i=0; i < k; ++i) {
        int s = h->size[i];
        if (s <= FAST_BITS) {
            int c = h->code[i] << (FAST_BITS-s);
            int m = 1 << (FAST_BITS-s);
            for (j=0; j < m; ++j) {
                h->fast[c+j] = (uint8) i;
            }
        }
    }
    return 1;
}

static void grow_buffer_unsafe(jpeg *j)
{
    do {
        int b = j->nomore ? 0 : get8(&j->s);
        if (b == 0xff) {
            int c = get8(&j->s);
            if (c != 0) {
                j->marker = (unsigned char) c;
                j->nomore = 1;
                return;
            }
        }
        j->code_buffer = (j->code_buffer << 8) | b;
        j->code_bits += 8;
    } while (j->code_bits <= 24);
}

// (1 << n) - 1
static uint32 bmask[17]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

// decode a jpeg huffman value from the bitstream
static int decode(jpeg *j, huffman *h)
{
    unsigned int temp;
    int c,k;
    
    if (j->code_bits < 16) grow_buffer_unsafe(j);
    
    // look at the top FAST_BITS and determine what symbol ID it is,
    // if the code is <= FAST_BITS
    c = (j->code_buffer >> (j->code_bits - FAST_BITS)) & ((1 << FAST_BITS)-1);
    k = h->fast[c];
    if (k < 255) {
        if (h->size[k] > j->code_bits)
            return -1;
        j->code_bits -= h->size[k];
        return h->values[k];
    }
    
    // naive test is to shift the code_buffer down so k bits are
    // valid, then test against maxcode. To speed this up, we've
    // preshifted maxcode left so that it has (16-k) 0s at the
    // end; in other words, regardless of the number of bits, it
    // wants to be compared against something shifted to have 16;
    // that way we don't need to shift inside the loop.
    if (j->code_bits < 16)
        temp = (j->code_buffer << (16 - j->code_bits)) & 0xffff;
    else
        temp = (j->code_buffer >> (j->code_bits - 16)) & 0xffff;
    for (k=FAST_BITS+1 ; ; ++k)
        if (temp < h->maxcode[k])
            break;
    if (k == 17) {
        // error! code not found
        j->code_bits -= 16;
        return -1;
    }
    
    if (k > j->code_bits)
        return -1;
    
    // convert the huffman code to the symbol id
    c = ((j->code_buffer >> (j->code_bits - k)) & bmask[k]) + h->delta[k];
    assert((((j->code_buffer) >> (j->code_bits - h->size[c])) & bmask[h->size[c]]) == h->code[c]);
    
    // convert the id to a symbol
    j->code_bits -= k;
    return h->values[c];
}

// combined JPEG 'receive' and JPEG 'extend', since baseline
// always extends everything it receives.
static int extend_receive(jpeg *j, int n)
{
    unsigned int m = 1 << (n-1);
    unsigned int k;
    if (j->code_bits < n) grow_buffer_unsafe(j);
    k = (j->code_buffer >> (j->code_bits - n)) & bmask[n];
    j->code_bits -= n;
    // the following test is probably a random branch that won't
    // predict well. I tried to table accelerate it but failed.
    // maybe it's compiling as a conditional move?
    if (k < m)
        return (-1 << n) + k + 1;
    else
        return k;
}

// given a value that's at position X in the zigzag stream,
// where does it appear in the 8x8 matrix coded as row-major?
static uint8 dezigzag[64+15] =
{
    0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
    // let corrupt input sample past end
    63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63
};

// decode one 64-entry block--
static int decode_block(jpeg *j, short data[64], huffman *hdc, huffman *hac, int b)
{
    int diff,dc,k;
    int t = decode(j, hdc);
    if (t < 0) return e("bad huffman code","Corrupt JPEG");
    
    // 0 all the ac values now so we can do it 32-bits at a time
    memset(data,0,64*sizeof(data[0]));
    
    diff = t ? extend_receive(j, t) : 0;
    dc = j->img_comp[b].dc_pred + diff;
    j->img_comp[b].dc_pred = dc;
    data[0] = (short) dc;
    
    // decode AC components, see JPEG spec
    k = 1;
    do {
        int r,s;
        int rs = decode(j, hac);
        if (rs < 0) return e("bad huffman code","Corrupt JPEG");
        s = rs & 15;
        r = rs >> 4;
        if (s == 0) {
            if (rs != 0xf0) break; // end block
            k += 16;
        } else {
            k += r;
            // decode into unzigzag'd location
            data[dezigzag[k++]] = (short) extend_receive(j,s);
        }
    } while (k < 64);
    return 1;
}

// take a -128..127 value and clamp it and convert to 0..255
static uint8 clamp(int x)
{
    x += 128;
    // trick to use a single test to catch both cases
    if ((unsigned int) x > 255) {
        if (x < 0) return 0;
        if (x > 255) return 255;
    }
    return (uint8) x;
}

#define f2f(x)  (int) (((x) * 4096 + 0.5))
#define fsh(x)  ((x) << 12)

// derived from jidctint -- DCT_ISLOW
#define IDCT_1D(s0,s1,s2,s3,s4,s5,s6,s7)       \
int t0,t1,t2,t3,p1,p2,p3,p4,p5,x0,x1,x2,x3; \
p2 = s2;                                    \
p3 = s6;                                    \
p1 = (p2+p3) * f2f(0.5411961f);             \
t2 = p1 + p3*f2f(-1.847759065f);            \
t3 = p1 + p2*f2f( 0.765366865f);            \
p2 = s0;                                    \
p3 = s4;                                    \
t0 = fsh(p2+p3);                            \
t1 = fsh(p2-p3);                            \
x0 = t0+t3;                                 \
x3 = t0-t3;                                 \
x1 = t1+t2;                                 \
x2 = t1-t2;                                 \
t0 = s7;                                    \
t1 = s5;                                    \
t2 = s3;                                    \
t3 = s1;                                    \
p3 = t0+t2;                                 \
p4 = t1+t3;                                 \
p1 = t0+t3;                                 \
p2 = t1+t2;                                 \
p5 = (p3+p4)*f2f( 1.175875602f);            \
t0 = t0*f2f( 0.298631336f);                 \
t1 = t1*f2f( 2.053119869f);                 \
t2 = t2*f2f( 3.072711026f);                 \
t3 = t3*f2f( 1.501321110f);                 \
p1 = p5 + p1*f2f(-0.899976223f);            \
p2 = p5 + p2*f2f(-2.562915447f);            \
p3 = p3*f2f(-1.961570560f);                 \
p4 = p4*f2f(-0.390180644f);                 \
t3 += p1+p4;                                \
t2 += p2+p3;                                \
t1 += p2+p4;                                \
t0 += p1+p3;

#if !STBI_SIMD
// .344 seconds on 3*anemones.jpg
static void idct_block(uint8 *out, int out_stride, short data[64], uint8 *dequantize)
{
    int i,val[64],*v=val;
    uint8 *o,*dq = dequantize;
    short *d = data;
    
    // columns
    for (i=0; i < 8; ++i,++d,++dq, ++v) {
        // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
        if (d[ 8]==0 && d[16]==0 && d[24]==0 && d[32]==0
            && d[40]==0 && d[48]==0 && d[56]==0) {
            //    no shortcut                 0     seconds
            //    (1|2|3|4|5|6|7)==0          0     seconds
            //    all separate               -0.047 seconds
            //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
            int dcterm = d[0] * dq[0] << 2;
            v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
        } else {
            IDCT_1D(d[ 0]*dq[ 0],d[ 8]*dq[ 8],d[16]*dq[16],d[24]*dq[24],
                    d[32]*dq[32],d[40]*dq[40],d[48]*dq[48],d[56]*dq[56])
            // constants scaled things up by 1<<12; let's bring them back
            // down, but keep 2 extra bits of precision
            x0 += 512; x1 += 512; x2 += 512; x3 += 512;
            v[ 0] = (x0+t3) >> 10;
            v[56] = (x0-t3) >> 10;
            v[ 8] = (x1+t2) >> 10;
            v[48] = (x1-t2) >> 10;
            v[16] = (x2+t1) >> 10;
            v[40] = (x2-t1) >> 10;
            v[24] = (x3+t0) >> 10;
            v[32] = (x3-t0) >> 10;
        }
    }
    
    for (i=0, v=val, o=out; i < 8; ++i,v+=8,o+=out_stride) {
        // no fast case since the first 1D IDCT spread components out
        IDCT_1D(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7])
        // constants scaled things up by 1<<12, plus we had 1<<2 from first
        // loop, plus horizontal and vertical each scale by sqrt(8) so together
        // we've got an extra 1<<3, so 1<<17 total we need to remove.
        x0 += 65536; x1 += 65536; x2 += 65536; x3 += 65536;
        o[0] = clamp((x0+t3) >> 17);
        o[7] = clamp((x0-t3) >> 17);
        o[1] = clamp((x1+t2) >> 17);
        o[6] = clamp((x1-t2) >> 17);
        o[2] = clamp((x2+t1) >> 17);
        o[5] = clamp((x2-t1) >> 17);
        o[3] = clamp((x3+t0) >> 17);
        o[4] = clamp((x3-t0) >> 17);
    }
}
#else
static void idct_block(uint8 *out, int out_stride, short data[64], unsigned short *dequantize)
{
    int i,val[64],*v=val;
    uint8 *o;
    unsigned short *dq = dequantize;
    short *d = data;
    
    // columns
    for (i=0; i < 8; ++i,++d,++dq, ++v) {
        // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
        if (d[ 8]==0 && d[16]==0 && d[24]==0 && d[32]==0
            && d[40]==0 && d[48]==0 && d[56]==0) {
            //    no shortcut                 0     seconds
            //    (1|2|3|4|5|6|7)==0          0     seconds
            //    all separate               -0.047 seconds
            //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
            int dcterm = d[0] * dq[0] << 2;
            v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
        } else {
            IDCT_1D(d[ 0]*dq[ 0],d[ 8]*dq[ 8],d[16]*dq[16],d[24]*dq[24],
                    d[32]*dq[32],d[40]*dq[40],d[48]*dq[48],d[56]*dq[56])
            // constants scaled things up by 1<<12; let's bring them back
            // down, but keep 2 extra bits of precision
            x0 += 512; x1 += 512; x2 += 512; x3 += 512;
            v[ 0] = (x0+t3) >> 10;
            v[56] = (x0-t3) >> 10;
            v[ 8] = (x1+t2) >> 10;
            v[48] = (x1-t2) >> 10;
            v[16] = (x2+t1) >> 10;
            v[40] = (x2-t1) >> 10;
            v[24] = (x3+t0) >> 10;
            v[32] = (x3-t0) >> 10;
        }
    }
    
    for (i=0, v=val, o=out; i < 8; ++i,v+=8,o+=out_stride) {
        // no fast case since the first 1D IDCT spread components out
        IDCT_1D(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7])
        // constants scaled things up by 1<<12, plus we had 1<<2 from first
        // loop, plus horizontal and vertical each scale by sqrt(8) so together
        // we've got an extra 1<<3, so 1<<17 total we need to remove.
        x0 += 65536; x1 += 65536; x2 += 65536; x3 += 65536;
        o[0] = clamp((x0+t3) >> 17);
        o[7] = clamp((x0-t3) >> 17);
        o[1] = clamp((x1+t2) >> 17);
        o[6] = clamp((x1-t2) >> 17);
        o[2] = clamp((x2+t1) >> 17);
        o[5] = clamp((x2-t1) >> 17);
        o[3] = clamp((x3+t0) >> 17);
        o[4] = clamp((x3-t0) >> 17);
    }
}
static stbi_idct_8x8 stbi_idct_installed = idct_block;

extern void stbi_install_idct(stbi_idct_8x8 func)
{
    stbi_idct_installed = func;
}
#endif

#define MARKER_none  0xff
// if there's a pending marker from the entropy stream, return that
// otherwise, fetch from the stream and get a marker. if there's no
// marker, return 0xff, which is never a valid marker value
static uint8 get_marker(jpeg *j)
{
    uint8 x;
    if (j->marker != MARKER_none) { x = j->marker; j->marker = MARKER_none; return x; }
    x = get8u(&j->s);
    if (x != 0xff) return MARKER_none;
    while (x == 0xff)
        x = get8u(&j->s);
    return x;
}

// in each scan, we'll have scan_n components, and the order
// of the components is specified by order[]
#define RESTART(x)     ((x) >= 0xd0 && (x) <= 0xd7)

// after a restart interval, reset the entropy decoder and
// the dc prediction
static void reset(jpeg *j)
{
    j->code_bits = 0;
    j->code_buffer = 0;
    j->nomore = 0;
    j->img_comp[0].dc_pred = j->img_comp[1].dc_pred = j->img_comp[2].dc_pred = 0;
    j->marker = MARKER_none;
    j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
    // no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
    // since we don't even allow 1<<30 pixels
}

static int parse_entropy_coded_data(jpeg *z)
{
    reset(z);
    if (z->scan_n == 1) {
        int i,j;
#if STBI_SIMD
        __declspec(align(16))
#endif
        short data[64];
        int n = z->order[0];
        // non-interleaved data, we just need to process one block at a time,
        // in trivial scanline order
        // number of blocks to do just depends on how many actual "pixels" this
        // component has, independent of interleaved MCU blocking and such
        int w = (z->img_comp[n].x+7) >> 3;
        int h = (z->img_comp[n].y+7) >> 3;
        for (j=0; j < h; ++j) {
            for (i=0; i < w; ++i) {
                if (!decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+z->img_comp[n].ha, n)) return 0;
#if STBI_SIMD
                stbi_idct_installed(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data, z->dequant2[z->img_comp[n].tq]);
#else
                idct_block(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data, z->dequant[z->img_comp[n].tq]);
#endif
                // every data block is an MCU, so countdown the restart interval
                if (--z->todo <= 0) {
                    if (z->code_bits < 24) grow_buffer_unsafe(z);
                    // if it's NOT a restart, then just bail, so we get corrupt data
                    // rather than no data
                    if (!RESTART(z->marker)) return 1;
                    reset(z);
                }
            }
        }
    } else { // interleaved!
        int i,j,k,x,y;
        short data[64];
        for (j=0; j < z->img_mcu_y; ++j) {
            for (i=0; i < z->img_mcu_x; ++i) {
                // scan an interleaved mcu... process scan_n components in order
                for (k=0; k < z->scan_n; ++k) {
                    int n = z->order[k];
                    // scan out an mcu's worth of this component; that's just determined
                    // by the basic H and V specified for the component
                    for (y=0; y < z->img_comp[n].v; ++y) {
                        for (x=0; x < z->img_comp[n].h; ++x) {
                            int x2 = (i*z->img_comp[n].h + x)*8;
                            int y2 = (j*z->img_comp[n].v + y)*8;
                            if (!decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+z->img_comp[n].ha, n)) return 0;
#if STBI_SIMD
                            stbi_idct_installed(z->img_comp[n].data+z->img_comp[n].w2*y2+x2, z->img_comp[n].w2, data, z->dequant2[z->img_comp[n].tq]);
#else
                            idct_block(z->img_comp[n].data+z->img_comp[n].w2*y2+x2, z->img_comp[n].w2, data, z->dequant[z->img_comp[n].tq]);
#endif
                        }
                    }
                }
                // after all interleaved components, that's an interleaved MCU,
                // so now count down the restart interval
                if (--z->todo <= 0) {
                    if (z->code_bits < 24) grow_buffer_unsafe(z);
                    // if it's NOT a restart, then just bail, so we get corrupt data
                    // rather than no data
                    if (!RESTART(z->marker)) return 1;
                    reset(z);
                }
            }
        }
    }
    return 1;
}

static int process_marker(jpeg *z, int m)
{
    int L;
    switch (m) {
        case MARKER_none: // no marker found
            return e("expected marker","Corrupt JPEG");
            
        case 0xC2: // SOF - progressive
            return e("progressive jpeg","JPEG format not supported (progressive)");
            
        case 0xDD: // DRI - specify restart interval
            if (get16(&z->s) != 4) return e("bad DRI len","Corrupt JPEG");
            z->restart_interval = get16(&z->s);
            return 1;
            
        case 0xDB: // DQT - define quantization table
            L = get16(&z->s)-2;
            while (L > 0) {
                int q = get8(&z->s);
                int p = q >> 4;
                int t = q & 15,i;
                if (p != 0) return e("bad DQT type","Corrupt JPEG");
                if (t > 3) return e("bad DQT table","Corrupt JPEG");
                for (i=0; i < 64; ++i)
                    z->dequant[t][dezigzag[i]] = get8u(&z->s);
#if STBI_SIMD
                for (i=0; i < 64; ++i)
                    z->dequant2[t][i] = dequant[t][i];
#endif
                L -= 65;
            }
            return L==0;
            
        case 0xC4: // DHT - define huffman table
            L = get16(&z->s)-2;
            while (L > 0) {
                uint8 *v;
                int sizes[16],i,m=0;
                int q = get8(&z->s);
                int tc = q >> 4;
                int th = q & 15;
                if (tc > 1 || th > 3) return e("bad DHT header","Corrupt JPEG");
                for (i=0; i < 16; ++i) {
                    sizes[i] = get8(&z->s);
                    m += sizes[i];
                }
                L -= 17;
                if (tc == 0) {
                    if (!build_huffman(z->huff_dc+th, sizes)) return 0;
                    v = z->huff_dc[th].values;
                } else {
                    if (!build_huffman(z->huff_ac+th, sizes)) return 0;
                    v = z->huff_ac[th].values;
                }
                for (i=0; i < m; ++i)
                    v[i] = get8u(&z->s);
                L -= m;
            }
            return L==0;
    }
    // check for comment block or APP blocks
    
    if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
        int size=get16(&z->s);
        
        // in progress
        // Exif is stored as an embedded TIFF file 
        if(m==0xE1){
#if 0
            int i;
            NSData *data=[NSData dataWithBytes:z->s.img_buffer+6 length:size-6];
            
            O2ImageSource *tiffSource=[[O2ImageSource_TIFF alloc] initWithData:data options:nil];
            O2Image       *image=[tiffSource imageAtIndex:0 options:nil];
#endif       
            // skip(&z->s, size-2);
        }
        skip(&z->s, size-2);
        return 1;
    }
    return 0;
}

// after we see SOS
static int process_scan_header(jpeg *z)
{
    int i;
    int Ls = get16(&z->s);
    z->scan_n = get8(&z->s);
    if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int) z->s.img_n) return e("bad SOS component count","Corrupt JPEG");
    if (Ls != 6+2*z->scan_n) return e("bad SOS len","Corrupt JPEG");
    for (i=0; i < z->scan_n; ++i) {
        int id = get8(&z->s), which;
        int q = get8(&z->s);
        for (which = 0; which < z->s.img_n; ++which)
            if (z->img_comp[which].id == id)
                break;
        if (which == z->s.img_n) return 0;
        z->img_comp[which].hd = q >> 4;   if (z->img_comp[which].hd > 3) return e("bad DC huff","Corrupt JPEG");
        z->img_comp[which].ha = q & 15;   if (z->img_comp[which].ha > 3) return e("bad AC huff","Corrupt JPEG");
        z->order[i] = which;
    }
    if (get8(&z->s) != 0) return e("bad SOS","Corrupt JPEG");
    get8(&z->s); // should be 63, but might be 0
    if (get8(&z->s) != 0) return e("bad SOS","Corrupt JPEG");
    
    return 1;
}

static int process_frame_header(jpeg *z, int scan)
{
    stbi *s = &z->s;
    int Lf,p,i,q, h_max=1,v_max=1;
    Lf = get16(s);         if (Lf < 11) return e("bad SOF len","Corrupt JPEG"); // JPEG
    p  = get8(s);          if (p != 8) return e("only 8-bit","JPEG format not supported: 8-bit only"); // JPEG baseline
    s->img_y = get16(s);   if (s->img_y == 0) return e("no header height", "JPEG format not supported: delayed height"); // Legal, but we don't handle it--but neither does IJG
    s->img_x = get16(s);   if (s->img_x == 0) return e("0 width","Corrupt JPEG"); // JPEG requires
    s->img_n = get8(s);
    if (s->img_n != 3 && s->img_n != 1) return e("bad component count","Corrupt JPEG");    // JFIF requires
    
    if (Lf != 8+3*s->img_n) return e("bad SOF len","Corrupt JPEG");
    
    for (i=0; i < s->img_n; ++i) {
        z->img_comp[i].id = get8(s);
        if (z->img_comp[i].id != i+1)   // JFIF requires
            if (z->img_comp[i].id != i)  // some version of jpegtran outputs non-JFIF-compliant files!
                return e("bad component ID","Corrupt JPEG");
        q = get8(s);
        z->img_comp[i].h = (q >> 4);  if (!z->img_comp[i].h || z->img_comp[i].h > 4) return e("bad H","Corrupt JPEG");
        z->img_comp[i].v = q & 15;    if (!z->img_comp[i].v || z->img_comp[i].v > 4) return e("bad V","Corrupt JPEG");
        z->img_comp[i].tq = get8(s);  if (z->img_comp[i].tq > 3) return e("bad TQ","Corrupt JPEG");
    }
    
    if (scan != SCAN_load) return 1;
    
    if ((1 << 30) / s->img_x / s->img_n < s->img_y) return e("too large", "Image too large to decode");
    
    for (i=0; i < s->img_n; ++i) {
        if (z->img_comp[i].h > h_max) h_max = z->img_comp[i].h;
        if (z->img_comp[i].v > v_max) v_max = z->img_comp[i].v;
    }
    
    // compute interleaved mcu info
    z->img_h_max = h_max;
    z->img_v_max = v_max;
    z->img_mcu_w = h_max * 8;
    z->img_mcu_h = v_max * 8;
    z->img_mcu_x = (s->img_x + z->img_mcu_w-1) / z->img_mcu_w;
    z->img_mcu_y = (s->img_y + z->img_mcu_h-1) / z->img_mcu_h;
    
    for (i=0; i < s->img_n; ++i) {
        // number of effective pixels (e.g. for non-interleaved MCU)
        z->img_comp[i].x = (s->img_x * z->img_comp[i].h + h_max-1) / h_max;
        z->img_comp[i].y = (s->img_y * z->img_comp[i].v + v_max-1) / v_max;
        // to simplify generation, we'll allocate enough memory to decode
        // the bogus oversized data from using interleaved MCUs and their
        // big blocks (e.g. a 16x16 iMCU on an image of width 33); we won't
        // discard the extra data until colorspace conversion
        z->img_comp[i].w2 = z->img_mcu_x * z->img_comp[i].h * 8;
        z->img_comp[i].h2 = z->img_mcu_y * z->img_comp[i].v * 8;
        z->img_comp[i].raw_data = malloc(z->img_comp[i].w2 * z->img_comp[i].h2+15);
        if (z->img_comp[i].raw_data == NULL) {
            for(--i; i >= 0; --i) {
                free(z->img_comp[i].raw_data);
                z->img_comp[i].data = NULL;
            }
            return e("outofmem", "Out of memory");
        }
        // align blocks for installable-idct using mmx/sse
        z->img_comp[i].data = (uint8*) (((size_t) z->img_comp[i].raw_data + 15) & ~15);
        z->img_comp[i].linebuf = NULL;
    }
    
    return 1;
}

// use comparisons since in some cases we handle more than one case (e.g. SOF)
#define DNL(x)         ((x) == 0xdc)
#define SOI(x)         ((x) == 0xd8)
#define EOI(x)         ((x) == 0xd9)
#define SOF(x)         ((x) == 0xc0 || (x) == 0xc1)
#define SOS(x)         ((x) == 0xda)

static int decode_jpeg_header(jpeg *z, int scan)
{
    int m;
    z->marker = MARKER_none; // initialize cached marker to empty
    m = get_marker(z);
    if (!SOI(m)) return e("no SOI","Corrupt JPEG");
    if (scan == SCAN_type) return 1;
    m = get_marker(z);
    while (!SOF(m)) {
        if (!process_marker(z,m)) return 0;
        m = get_marker(z);
        while (m == MARKER_none) {
            // some files have extra padding after their blocks, so ok, we'll scan
            if (at_eof(&z->s)) return e("no SOF", "Corrupt JPEG");
            m = get_marker(z);
        }
    }
    if (!process_frame_header(z, scan)) return 0;
    return 1;
}

static int decode_jpeg_image(jpeg *j)
{
    int m;
    j->restart_interval = 0;
    if (!decode_jpeg_header(j, SCAN_load)) return 0;
    m = get_marker(j);
    while (!EOI(m)) {
        if (SOS(m)) {
            if (!process_scan_header(j)) return 0;
            if (!parse_entropy_coded_data(j)) return 0;
        } else {
            if (!process_marker(j, m)) return 0;
        }
        m = get_marker(j);
    }
    return 1;
}

// static jfif-centered resampling (across block boundaries)

typedef uint8 *(*resample_row_func)(uint8 *out, uint8 *in0, uint8 *in1,
int w, int hs);

#define div4(x) ((uint8) ((x) >> 2))

static uint8 *resample_row_1(uint8 *out, uint8 *in_near, uint8 *in_far, int w, int hs)
{
    return in_near;
}

static uint8* resample_row_v_2(uint8 *out, uint8 *in_near, uint8 *in_far, int w, int hs)
{
    // need to generate two samples vertically for every one in input
    int i;
    for (i=0; i < w; ++i)
        out[i] = div4(3*in_near[i] + in_far[i] + 2);
    return out;
}

static uint8*  resample_row_h_2(uint8 *out, uint8 *in_near, uint8 *in_far, int w, int hs)
{
    // need to generate two samples horizontally for every one in input
    int i;
    uint8 *input = in_near;
    if (w == 1) {
        // if only one sample, can't do any interpolation
        out[0] = out[1] = input[0];
        return out;
    }
    
    out[0] = input[0];
    out[1] = div4(input[0]*3 + input[1] + 2);
    for (i=1; i < w-1; ++i) {
        int n = 3*input[i]+2;
        out[i*2+0] = div4(n+input[i-1]);
        out[i*2+1] = div4(n+input[i+1]);
    }
    out[i*2+0] = div4(input[w-2]*3 + input[w-1] + 2);
    out[i*2+1] = input[w-1];
    return out;
}

#define div16(x) ((uint8) ((x) >> 4))

static uint8 *resample_row_hv_2(uint8 *out, uint8 *in_near, uint8 *in_far, int w, int hs)
{
    // need to generate 2x2 samples for every one in input
    int i,t0,t1;
    if (w == 1) {
        out[0] = out[1] = div4(3*in_near[0] + in_far[0] + 2);
        return out;
    }
    
    t1 = 3*in_near[0] + in_far[0];
    out[0] = div4(t1+2);
    for (i=1; i < w; ++i) {
        t0 = t1;
        t1 = 3*in_near[i]+in_far[i];
        out[i*2-1] = div16(3*t0 + t1 + 8);
        out[i*2  ] = div16(3*t1 + t0 + 8);
    }
    out[w*2-1] = div4(t1+2);
    return out;
}

static uint8 *resample_row_generic(uint8 *out, uint8 *in_near, uint8 *in_far, int w, int hs)
{
    // resample with nearest-neighbor
    int i,j;
    for (i=0; i < w; ++i)
        for (j=0; j < hs; ++j)
            out[i*hs+j] = in_near[i];
    return out;
}

#define float2fixed(x)  ((int) ((x) * 65536 + 0.5))

// 0.38 seconds on 3*anemones.jpg   (0.25 with processor = Pro)
// VC6 without processor=Pro is generating multiple LEAs per multiply!
static void YCbCr_to_RGB_row(uint8 *out, uint8 *y, uint8 *pcb, uint8 *pcr, int count, int step)
{
    int i;
    for (i=0; i < count; ++i) {
        int y_fixed = (y[i] << 16) + 32768; // rounding
        int r,g,b;
        int cr = pcr[i] - 128;
        int cb = pcb[i] - 128;
        r = y_fixed + cr*float2fixed(1.40200f);
        g = y_fixed - cr*float2fixed(0.71414f) - cb*float2fixed(0.34414f);
        b = y_fixed                            + cb*float2fixed(1.77200f);
        r >>= 16;
        g >>= 16;
        b >>= 16;
        if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
        if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
        if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
        out[0] = (uint8)r;
        out[1] = (uint8)g;
        out[2] = (uint8)b;
        out[3] = 255;
        out += step;
    }
}

#if STBI_SIMD
static stbi_YCbCr_to_RGB_run stbi_YCbCr_installed = YCbCr_to_RGB_row;

void stbi_install_YCbCr_to_RGB(stbi_YCbCr_to_RGB_run func)
{
    stbi_YCbCr_installed = func;
}
#endif


// clean up the temporary component buffers
static void cleanup_jpeg(jpeg *j)
{
    int i;
    for (i=0; i < j->s.img_n; ++i) {
        if (j->img_comp[i].data) {
            free(j->img_comp[i].raw_data);
            j->img_comp[i].data = NULL;
        }
        if (j->img_comp[i].linebuf) {
            free(j->img_comp[i].linebuf);
            j->img_comp[i].linebuf = NULL;
        }
    }
}

typedef struct
{
    resample_row_func resample;
    uint8 *line0,*line1;
    int hs,vs;   // expansion factor in each axis
    int w_lores; // horizontal pixels pre-expansion 
    int ystep;   // how far through vertical expansion we are
    int ypos;    // which pre-expansion row we're on
} stbi_resample;

static uint8 *load_jpeg_image(jpeg *z, int *out_x, int *out_y, int *comp, int req_comp)
{
    int n, decode_n;
    // validate req_comp
    if (req_comp < 0 || req_comp > 4) return epuc("bad req_comp", "Internal error");
    
    // load a jpeg image from whichever source
    if (!decode_jpeg_image(z)) { cleanup_jpeg(z); return NULL; }
    
    // determine actual number of components to generate
    n = req_comp ? req_comp : z->s.img_n;
    
    if (z->s.img_n == 3 && n < 3)
        decode_n = 1;
    else
        decode_n = z->s.img_n;
    
    // resample and color-convert
    {
        int k;
        uint i,j;
        uint8 *output;
        uint8 *coutput[4];
        
        stbi_resample res_comp[4];
        
        for (k=0; k < decode_n; ++k) {
            stbi_resample *r = &res_comp[k];
            
            // allocate line buffer big enough for upsampling off the edges
            // with upsample factor of 4
            z->img_comp[k].linebuf = (uint8 *) malloc(z->s.img_x + 3);
            if (!z->img_comp[k].linebuf) { cleanup_jpeg(z); return epuc("outofmem", "Out of memory"); }
            
            r->hs      = z->img_h_max / z->img_comp[k].h;
            r->vs      = z->img_v_max / z->img_comp[k].v;
            r->ystep   = r->vs >> 1;
            r->w_lores = (z->s.img_x + r->hs-1) / r->hs;
            r->ypos    = 0;
            r->line0   = r->line1 = z->img_comp[k].data;
            
            if      (r->hs == 1 && r->vs == 1) r->resample = resample_row_1;
            else if (r->hs == 1 && r->vs == 2) r->resample = resample_row_v_2;
            else if (r->hs == 2 && r->vs == 1) r->resample = resample_row_h_2;
            else if (r->hs == 2 && r->vs == 2) r->resample = resample_row_hv_2;
            else                               r->resample = resample_row_generic;
        }
        
        // can't error after this so, this is safe
        output = (uint8 *) malloc(n * z->s.img_x * z->s.img_y + 1);
        if (!output) { cleanup_jpeg(z); return epuc("outofmem", "Out of memory"); }
        
        // now go ahead and resample
        for (j=0; j < z->s.img_y; ++j) {
            uint8 *out = output + n * z->s.img_x * j;
            for (k=0; k < decode_n; ++k) {
                stbi_resample *r = &res_comp[k];
                int y_bot = r->ystep >= (r->vs >> 1);
                coutput[k] = r->resample(z->img_comp[k].linebuf,
                                         y_bot ? r->line1 : r->line0,
                                         y_bot ? r->line0 : r->line1,
                                         r->w_lores, r->hs);
                if (++r->ystep >= r->vs) {
                    r->ystep = 0;
                    r->line0 = r->line1;
                    if (++r->ypos < z->img_comp[k].y)
                        r->line1 += z->img_comp[k].w2;
                }
            }
            if (n >= 3) {
                uint8 *y = coutput[0];
                if (z->s.img_n == 3) {
#if STBI_SIMD
                    stbi_YCbCr_installed(out, y, coutput[1], coutput[2], z->s.img_x, n);
#else
                    YCbCr_to_RGB_row(out, y, coutput[1], coutput[2], z->s.img_x, n);
#endif
                } else
                    for (i=0; i < z->s.img_x; ++i) {
                        out[0] = out[1] = out[2] = y[i];
                        out[3] = 255; // not used if n==3
                        out += n;
                    }
            } else {
                uint8 *y = coutput[0];
                if (n == 1)
                    for (i=0; i < z->s.img_x; ++i) out[i] = y[i];
                else
                    for (i=0; i < z->s.img_x; ++i) *out++ = y[i], *out++ = 255;
            }
        }
        cleanup_jpeg(z);
        *out_x = z->s.img_x;
        *out_y = z->s.img_y;
        if (comp) *comp  = z->s.img_n; // report original components, not output
        return output;
    }
}

static unsigned char *stbi_jpeg_load_from_memory(jpeg *j,stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
    start_mem(&(j->s), buffer,len);
    return load_jpeg_image(j, x,y,comp,req_comp);
}
// clang-format on

-initWithDataProvider:(O2DataProviderRef)dataProvider {
    
    _compressionType=O2ImageCompressionJPEG;
    _dataProvider=[dataProvider retain];
    
    CFDataRef encodedData=O2DataProviderCopyData(dataProvider);
    CFIndex encodedLength=CFDataGetLength(encodedData);
    const uint8_t *encodedBytes=CFDataGetBytePtr(encodedData);
    
    int      comp;
    uint8_t *bitmap;
    
    jpeg jpeg_decoder;
    int width,height;
    
    bitmap=stbi_jpeg_load_from_memory(&jpeg_decoder,encodedBytes,encodedLength,&width,&height,&comp,STBI_rgb_alpha);
    
    CFRelease(encodedData);

    if(bitmap==NULL){
        [self dealloc];
        return nil;
    }
    
    _width=width;
    _height=height;
    _bitsPerComponent=8;
    _bitsPerPixel=32;
    _bytesPerRow=(_bitsPerPixel/(sizeof(char)*8))*_width;
    _colorSpace=O2ColorSpaceCreateDeviceRGB();
    _bitmapInfo=kO2BitmapByteOrder32Big|kO2ImageAlphaPremultipliedLast;
    
    _pixelData=CFDataCreateWithBytesNoCopy(NULL, bitmap, _bytesPerRow*_height, NULL);
    _pixelData=(CFDataRef)[[NSData alloc] initWithBytesNoCopy:bitmap length:_bytesPerRow*_height freeWhenDone:YES];

    return self;
}

-(void)dealloc {
    [_dataProvider release];
    [_colorSpace release];
    CFRelease(_pixelData);
    [super dealloc];
}

-(CFDataRef)createPixelData {
    return CFRetain(_pixelData);
}

@end