/*
Gif-Lib by Gershon Elber,Eric S. Raymond,Toshio Kuratomi
The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#import <stdint.h>

@class NSInputStream, NSOutputStream;

#define HT_SIZE 8192 /* 12bits = 4096 or twice as big! */
#define HT_KEY_MASK 0x1FFF /* 13bits keys */
#define HT_KEY_NUM_BITS 13 /* 13bits keys */
#define HT_MAX_KEY 8191 /* 13bits - 1, maximal code possible */
#define HT_MAX_CODE 4095 /* Biggest code possible in 12 bits. */

/* The 32 bits of the long are divided into two parts for the key & code:   */
/* 1. The code is 12 bits as our compression algorithm is limited to 12bits */
/* 2. The key is 12 bits Prefix code + 8 bit new char or 20 bits.	    */
/* The key is the upper 20 bits.  The code is the lower 12. */
#define HT_GET_KEY(l) (l >> 12)
#define HT_GET_CODE(l) (l & 0x0FFF)
#define HT_PUT_KEY(l) (l << 12)
#define HT_PUT_CODE(l) (l & 0x0FFF)

typedef struct GifHashTableType {
    uint32_t HTable[HT_SIZE];
} GifHashTableType;

#define GIF_ERROR 0
#define GIF_OK 1

#define GIF_STAMP "GIFVER" /* First chars in file - GIF stamp.  */
#define GIF_STAMP_LEN sizeof(GIF_STAMP) - 1
#define GIF_VERSION_POS 3 /* Version first character in stamp. */
#define GIF87_STAMP "GIF87a" /* First chars in file - GIF stamp.  */
#define GIF89_STAMP "GIF89a" /* First chars in file - GIF stamp.  */

#define LZ_MAX_CODE 4095 /* Biggest code possible in 12 bits. */
#define LZ_BITS 12

#define FLUSH_OUTPUT 4096 /* Impossible code, to signal flush. */
#define FIRST_CODE 4097 /* Impossible code, to signal first. */
#define NO_SUCH_CODE 4098 /* Impossible code, to signal empty. */

#define FILE_STATE_WRITE 0x01
#define FILE_STATE_SCREEN 0x02
#define FILE_STATE_IMAGE 0x04
#define FILE_STATE_READ 0x08

#define IS_READABLE(Private) (GifFile->FileState & FILE_STATE_READ)
#define IS_WRITEABLE(Private) (GifFile->FileState & FILE_STATE_WRITE)

typedef unsigned char GifPixelType;
typedef unsigned int GifPrefixType;
typedef int GifWord;

typedef struct GifColorType {
    uint8_t Red, Green, Blue;
} GifColorType;

typedef struct ColorMapObject {
    int ColorCount;
    int BitsPerPixel;
    GifColorType *Colors; /* on malloc(3) heap */
} ColorMapObject;

typedef struct GifImageDesc {
    GifWord Left, Top, Width, Height, /* Current image dimensions. */
        Interlace; /* Sequential/Interlaced lines. */
    ColorMapObject *ColorMap; /* The local color map */
} GifImageDesc;

/* This is the in-core version of an extension record */
typedef struct {
    int ByteCount;
    char *Bytes; /* on malloc(3) heap */
    int Function; /* Holds the type of the Extension block. */
} ExtensionBlock;

/* This holds an image header, its unpacked raster bits, and extensions */
typedef struct SavedImage {
    GifImageDesc ImageDesc;
    unsigned char *RasterBits; /* on malloc(3) heap */
    // int Function;   /* DEPRECATED: Use ExtensionBlocks[x].Function instead */
    int ExtensionBlockCount;
    ExtensionBlock *ExtensionBlocks; /* on malloc(3) heap */
} SavedImage;

/* func type to read gif data from arbitrary sources (TVT) */
typedef struct GifFileType *GifFileTypeRef;

typedef struct GifFileType {
    GifWord SWidth, SHeight, /* Screen dimensions. */
        SColorResolution, /* How many colors can we generate? */
        SBackGroundColor; /* I hope you understand this one... */
    ColorMapObject *SColorMap; /* NULL if not exists. */
    int ImageCount; /* Number of current image */
    GifImageDesc Image; /* Block describing current image */
    struct SavedImage *SavedImages; /* Use this to accumulate file state */

    int GifError;

    GifWord FileState,
        BitsPerPixel, /* Bits per pixel (Codes uses at least this + 1). */
        ClearCode, /* The CLEAR LZ code. */
        EOFCode, /* The EOF LZ code. */
        RunningCode, /* The next code algorithm can generate. */
        RunningBits, /* The number of bits required to represent RunningCode. */
        MaxCode1, /* 1 bigger than max. possible code, in RunningBits bits. */
        LastCode, /* The code before the current code. */
        CrntCode, /* Current algorithm code. */
        StackPtr, /* For character stack (see below). */
        CrntShiftState; /* Number of bits in CrntShiftDWord. */
    unsigned long CrntShiftDWord; /* For bytes decomposition into codes. */
    unsigned long PixelCount; /* Number of pixels in image. */
    NSInputStream *inputStream;
    NSOutputStream *outputStream;
    uint8_t Buf[256]; /* Compressed input is buffered here. */
    uint8_t Stack[LZ_MAX_CODE]; /* Decoded pixels are stacked here. */
    uint8_t Suffix[LZ_MAX_CODE + 1]; /* So we can trace the codes. */
    GifPrefixType Prefix[LZ_MAX_CODE + 1];
    GifHashTableType *HashTable;
} GifFileType;

typedef enum {
    UNDEFINED_RECORD_TYPE,
    SCREEN_DESC_RECORD_TYPE,
    IMAGE_DESC_RECORD_TYPE, /* Begin with ',' */
    EXTENSION_RECORD_TYPE, /* Begin with '!' */
    TERMINATE_RECORD_TYPE /* Begin with ';' */
} GifRecordType;

/******************************************************************************
 *  GIF89 extension function codes                                             
******************************************************************************/

#define COMMENT_EXT_FUNC_CODE 0xfe /* comment */
#define GRAPHICS_EXT_FUNC_CODE 0xf9 /* graphics control */
#define PLAINTEXT_EXT_FUNC_CODE 0x01 /* plaintext */
#define APPLICATION_EXT_FUNC_CODE 0xff /* application block */

GifFileType *EGifOpen(NSOutputStream *stream);

int EGifSpew(GifFileType *GifFile);
void EGifSetGifVersion(const char *Version);
int EGifPutScreenDesc(GifFileType *GifFile, int GifWidth, int GifHeight, int GifColorRes, int GifBackGround, const ColorMapObject *GifColorMap);
int EGifPutImageDesc(GifFileType *GifFile, int GifLeft, int GifTop, int Width, int GifHeight, int GifInterlace, const ColorMapObject *GifColorMap);
int EGifPutLine(GifFileType *GifFile, GifPixelType *GifLine, int GifLineLen);
int EGifPutPixel(GifFileType *GifFile, GifPixelType GifPixel);
int EGifPutComment(GifFileType *GifFile, const char *GifComment);
int EGifPutExtensionFirst(GifFileType *GifFile, int GifExtCode, int GifExtLen, const void *GifExtension);
int EGifPutExtensionNext(GifFileType *GifFile, int GifExtCode, int GifExtLen, const void *GifExtension);
int EGifPutExtensionLast(GifFileType *GifFile, int GifExtCode, int GifExtLen, const void *GifExtension);
int EGifPutExtension(GifFileType *GifFile, int GifExtCode, int GifExtLen, const void *GifExtension);
int EGifPutCode(GifFileType *GifFile, int GifCodeSize, const uint8_t *GifCodeBlock);
int EGifPutCodeNext(GifFileType *GifFile, const uint8_t *GifCodeBlock);
int EGifCloseFile(GifFileType *GifFile);

#define E_GIF_ERR_OPEN_FAILED 1
#define E_GIF_ERR_WRITE_FAILED 2
#define E_GIF_ERR_HAS_SCRN_DSCR 3
#define E_GIF_ERR_HAS_IMAG_DSCR 4
#define E_GIF_ERR_NO_COLOR_MAP 5
#define E_GIF_ERR_DATA_TOO_BIG 6
#define E_GIF_ERR_NOT_ENOUGH_MEM 7
#define E_GIF_ERR_DISK_IS_FULL 8
#define E_GIF_ERR_CLOSE_FAILED 9
#define E_GIF_ERR_NOT_WRITEABLE 10

/******************************************************************************
 * O.K., here are the routines one can access in order to decode GIF file:     
 * (GIF_LIB file DGIF_LIB.C).                              
 *****************************************************************************/

GifFileType *DGifOpen(NSInputStream *stream);
int DGifGetScreenDesc(GifFileType *GifFile);
int DGifGetRecordType(GifFileType *GifFile, GifRecordType *GifType);
int DGifGetImageDesc(GifFileType *GifFile);
int DGifGetLine(GifFileType *GifFile, GifPixelType *GifLine, int GifLineLen);
int DGifGetComment(GifFileType *GifFile, char *GifComment);
int DGifGetExtension(GifFileType *GifFile, int *GifExtCode, uint8_t **GifExtension);
int DGifGetExtensionNext(GifFileType *GifFile, uint8_t **GifExtension);
int DGifGetCode(GifFileType *GifFile, int *GifCodeSize, uint8_t **GifCodeBlock);
int DGifGetCodeNext(GifFileType *GifFile, uint8_t **GifCodeBlock);
int DGifCloseFile(GifFileType *GifFile);
int DGifSlurp(GifFileType *GifFile);

#define D_GIF_ERR_OPEN_FAILED 101
#define D_GIF_ERR_READ_FAILED 102
#define D_GIF_ERR_NOT_GIF_FILE 103
#define D_GIF_ERR_NO_SCRN_DSCR 104
#define D_GIF_ERR_NO_IMAG_DSCR 105
#define D_GIF_ERR_NO_COLOR_MAP 106
#define D_GIF_ERR_WRONG_RECORD 107
#define D_GIF_ERR_DATA_TOO_BIG 108
#define D_GIF_ERR_NOT_ENOUGH_MEM 109
#define D_GIF_ERR_CLOSE_FAILED 110
#define D_GIF_ERR_NOT_READABLE 111
#define D_GIF_ERR_IMAGE_DEFECT 112
#define D_GIF_ERR_EOF_TOO_SOON 113

int QuantizeBuffer(unsigned int Width, unsigned int Height, int *ColorMapSize, uint8_t *RedInput, uint8_t *GreenInput, uint8_t *BlueInput, uint8_t *OutputBuffer, GifColorType *OutputColorMap);

extern int GifLastError(GifFileType *);

extern ColorMapObject *MakeMapObject(int ColorCount, const GifColorType *ColorMap);
extern void FreeMapObject(ColorMapObject *Object);
extern ColorMapObject *UnionColorMap(const ColorMapObject *ColorIn1, const ColorMapObject *ColorIn2, GifPixelType ColorTransIn2[]);

extern void ApplyTranslation(SavedImage *Image, GifPixelType Translation[]);
extern int AddExtensionBlock(SavedImage *New, int Len, unsigned char ExtData[], int Function);
extern void FreeExtension(SavedImage *Image);
extern SavedImage *MakeSavedImage(GifFileType *GifFile, const SavedImage *CopyFrom);
extern void FreeSavedImages(GifFileType *GifFile);
