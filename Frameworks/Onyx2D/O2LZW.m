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

#import <Onyx2D/O2LZW.h>
#import <Onyx2D/O2DataConsumer.h>

#define LZ_MAX_CODE         4095    /* Biggest code possible in 12 bits. */
#define LZ_BITS             12
#define NO_SUCH_CODE        4098    /* Impossible code, to signal empty. */

#define HT_SIZE			8192	   /* 12bits = 4096 or twice as big! */

#define GIF_ERROR   0
#define GIF_OK      1

#define D_GIF_ERR_READ_FAILED    102
#define D_GIF_ERR_DATA_TOO_BIG   108
#define D_GIF_ERR_NOT_READABLE   111
#define D_GIF_ERR_IMAGE_DEFECT   112
#define D_GIF_ERR_EOF_TOO_SOON   113

static inline int READ(LZWFileType *  _gif,uint8_t *_buf,int _len){

   return [(_gif)->inputStream read:_buf maxLength:_len];
}


/******************************************************************************
 * Setup the LZ decompression for this image:
 *****************************************************************************/
 int DLZWSetupDecompress(LZWFileType * LZWFile) {
 
    int i, BitsPerPixel;
    LZWPrefixType *Prefix;

    BitsPerPixel =  8;
    LZWFile->BitsPerPixel = BitsPerPixel;
    LZWFile->ClearCode = (1 << BitsPerPixel);
    LZWFile->EOFCode = LZWFile->ClearCode + 1;
    LZWFile->RunningCode = LZWFile->EOFCode + 1;
    LZWFile->RunningBits = BitsPerPixel + 1;    /* Number of bits per code. */
    LZWFile->MaxCode1 = 1 << LZWFile->RunningBits;    /* Max. code + 1. */
    LZWFile->StackPtr = 0;    /* No pixels on the pixel stack. */
    LZWFile->LastCode = NO_SUCH_CODE;
    LZWFile->CrntShiftState = 0;    /* No information in CrntShiftDWord. */
    LZWFile->CrntShiftDWord = 0;

    Prefix = LZWFile->Prefix;
    for (i = 0; i <= LZ_MAX_CODE; i++)
        Prefix[i] = NO_SUCH_CODE;

    return GIF_OK;
}

/******************************************************************************
 * This routines read one gif data block at a time and buffers it internally
 * so that the decompression routine could access it.
 * The routine returns the next byte from its internal buffer (or read next
 * block in if buffer empty) and returns GIF_OK if succesful.
 *****************************************************************************/
static int DLZWBufferedInput(LZWFileType * LZWFile,
                  uint8_t * NextByte) {
if(READ(LZWFile, NextByte, 1)==-1)
 return GIF_ERROR;
 
return GIF_OK;
}

/******************************************************************************
 * The LZ decompression input routine:
 * This routine is responsable for the decompression of the bit stream from
 * 8 bits (bytes) packets, into the real codes.
 * Returns GIF_OK if read succesfully.
 *****************************************************************************/
static int DLZWDecompressInput(LZWFileType * LZWFile,int *Code) {
    uint8_t NextByte;
    static unsigned short CodeMasks[] = {
        0x0000, 0x0001, 0x0003, 0x0007,
        0x000f, 0x001f, 0x003f, 0x007f,
        0x00ff, 0x01ff, 0x03ff, 0x07ff,
        0x0fff
    };
    /* The image can't contain more than LZ_BITS per code. */
    if (LZWFile->RunningBits > LZ_BITS) {
        LZWFile->LZWError = D_GIF_ERR_IMAGE_DEFECT;
        return GIF_ERROR;
    }
    
    while (LZWFile->CrntShiftState < LZWFile->RunningBits) {
        /* Needs to get more bytes from input stream for next code: */
        if (DLZWBufferedInput(LZWFile, &NextByte) == GIF_ERROR) {
            return GIF_ERROR;
        }
        LZWFile->CrntShiftDWord<<=8;
        LZWFile->CrntShiftDWord|=(unsigned)NextByte;
        LZWFile->CrntShiftState += 8;
    }
    *Code = (LZWFile->CrntShiftDWord>>(LZWFile->CrntShiftState-LZWFile->RunningBits)) & CodeMasks[LZWFile->RunningBits];
 //   LZWFile->CrntShiftDWord >>= LZWFile->RunningBits;
    LZWFile->CrntShiftState -= LZWFile->RunningBits;

    /* If code cannot fit into RunningBits bits, must raise its size. Note
     * however that codes above 4095 are used for special signaling.
     * If we're using LZ_BITS bits already and we're at the max code, just
     * keep using the table as it is, don't increment LZWFile->RunningCode.
     */
    if (LZWFile->RunningCode < LZ_MAX_CODE + 2){
      LZWFile->RunningCode++;
      if(LZWFile->RunningCode >=LZWFile->MaxCode1 && LZWFile->RunningBits < LZ_BITS) {
        LZWFile->MaxCode1 <<= 1;
        LZWFile->RunningBits++;
      }
    }

    return GIF_OK;
}

/******************************************************************************
 * Routine to trace the Prefixes linked list until we get a prefix which is
 * not code, but a pixel value (less than ClearCode). Returns that pixel value.
 * If image is defective, we might loop here forever, so we limit the loops to
 * the maximum possible if image O.k. - LZ_MAX_CODE times.
 *****************************************************************************/
static int DLZWGetPrefixChar(LZWPrefixType *Prefix,int Code,int ClearCode) {

    int i = 0;

    while (Code > ClearCode && i++ <= LZ_MAX_CODE) {
        if (Code > LZ_MAX_CODE) {
            return NO_SUCH_CODE;
        }
        Code = Prefix[Code];
    }
    return Code;
}

int DLZWDecompressLine(LZWFileType * LZWFile,O2DataConsumerRef consumer,int LineLen) {

    int i = 0;
    int j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr;
    uint8_t *Stack, *Suffix;
    LZWPrefixType *Prefix;

    StackPtr = LZWFile->StackPtr;
    Prefix = LZWFile->Prefix;
    Suffix = LZWFile->Suffix;
    Stack = LZWFile->Stack;
    EOFCode = LZWFile->EOFCode;
    ClearCode = LZWFile->ClearCode;
    LastCode = LZWFile->LastCode;

    while (i < LineLen) {    /* Decode LineLen items. */
        if (DLZWDecompressInput(LZWFile, &CrntCode) == GIF_ERROR){
         NSLog(@"error at %s %d",__FILE__,__LINE__);
            return GIF_ERROR;
        }

        if (CrntCode == EOFCode) {
            /* Note however that usually we will not be here as we will stop
             * decoding as soon as we got all the pixel, or EOF code will
             * not be read at all, and DLZWGetLine/Pixel clean everything.  */
            if (i != LineLen - 1 || LZWFile->PixelCount != 0) {
                NSLog(@"error at %s %d",__FILE__,__LINE__);
                LZWFile->LZWError = D_GIF_ERR_EOF_TOO_SOON;
                return GIF_ERROR;
            }
            i++;
        } else if (CrntCode == ClearCode) {
                    /* We need to start over again: */
            for (j = 0; j <= LZ_MAX_CODE; j++)
                Prefix[j] = NO_SUCH_CODE;
            LZWFile->RunningCode = LZWFile->EOFCode + 1;
            LZWFile->RunningBits = LZWFile->BitsPerPixel + 1;
            LZWFile->MaxCode1 = 1 << LZWFile->RunningBits;
            LastCode = LZWFile->LastCode = NO_SUCH_CODE;
            LastCode = CrntCode;
        } else {
            if (CrntCode < ClearCode) {
                /* This is simple - its pixel scalar, so add it to output: */
                uint8_t bytes[1];
                bytes[0]=CrntCode;
                O2DataConsumerPutBytes(consumer,bytes,1);

                i++;
            } else 
#define RUNNING_CODE_MINUS 2
            {
                /* Its a code to needed to be traced: trace the linked list
                 * until the prefix is a pixel, while pushing the suffix
                 * pixels on our stack. If we done, pop the stack in reverse
                 * (thats what stack is good for!) order to output.  */
                if (Prefix[CrntCode] == NO_SUCH_CODE) {
                    /* Only allowed if CrntCode is exactly the running code:
                     * In that case CrntCode = XXXCode, CrntCode or the
                     * prefix code is last code and the suffix char is
                     * exactly the prefix of last code! */
                    if (CrntCode == LZWFile->RunningCode - RUNNING_CODE_MINUS) {
                        CrntPrefix = LastCode;
                        Suffix[LZWFile->RunningCode - RUNNING_CODE_MINUS] =
                           Stack[StackPtr++] = DLZWGetPrefixChar(Prefix,
                                                                 LastCode,
                                                                 ClearCode);
                    } else {
                       LZWFile->LZWError = D_GIF_ERR_IMAGE_DEFECT;
                       NSLog(@"error at %s %d",__FILE__,__LINE__);
                        return GIF_ERROR;
                    }
                } else
                    CrntPrefix = CrntCode;
 
                /* Now (if image is O.K.) we should not get an NO_SUCH_CODE
                 * During the trace. As we might loop forever, in case of
                 * defective image, we count the number of loops we trace
                 * and stop if we got LZ_MAX_CODE. obviously we can not
                 * loop more than that.  */
                j = 0;
                while (j++ <= LZ_MAX_CODE && CrntPrefix > ClearCode && CrntPrefix <= LZ_MAX_CODE) {
                    Stack[StackPtr++] = Suffix[CrntPrefix];
                    CrntPrefix = Prefix[CrntPrefix];
                }
                if (j >= LZ_MAX_CODE || CrntPrefix > LZ_MAX_CODE) {
                    LZWFile->LZWError = D_GIF_ERR_IMAGE_DEFECT;
                    NSLog(@"error at %s %d",__FILE__,__LINE__);
                    return GIF_ERROR;
                }
                /* Push the last character on stack: */
                Stack[StackPtr++] = CrntPrefix;

                /* Now lets pop all the stack into output: */
                while (StackPtr != 0 && i < LineLen){
                 uint8_t bytes[1];
                 bytes[0]=Stack[--StackPtr];
                 
                 O2DataConsumerPutBytes(consumer,bytes,1);
                
                    i++;
                 }
            }
            if (LastCode != NO_SUCH_CODE) {
                Prefix[LZWFile->RunningCode - RUNNING_CODE_MINUS] = LastCode;

                if (CrntCode == LZWFile->RunningCode - RUNNING_CODE_MINUS) {
                    /* Only allowed if CrntCode is exactly the running code:
                     * In that case CrntCode = XXXCode, CrntCode or the
                     * prefix code is last code and the suffix char is
                     * exactly the prefix of last code! */
                    Suffix[LZWFile->RunningCode - RUNNING_CODE_MINUS] =DLZWGetPrefixChar(Prefix, LastCode, ClearCode);
                } else {
                    Suffix[LZWFile->RunningCode - RUNNING_CODE_MINUS] =DLZWGetPrefixChar(Prefix, CrntCode, ClearCode);
                }
            }
            LastCode = CrntCode;
        }
    }

    LZWFile->LastCode = LastCode;
    LZWFile->StackPtr = StackPtr;

    return GIF_OK;
}

NSData *LZWDecodeWithExpectedResultLength(NSData *data,unsigned stripLength){
   NSMutableData    *outputData=[NSMutableData data];
   O2DataConsumerRef consumer=O2DataConsumerCreateWithCFData((CFMutableDataRef)outputData);
   LZWFileType lzwStream;

   lzwStream.inputStream=[NSInputStream inputStreamWithData:data];
   [lzwStream.inputStream open];
   lzwStream.PixelCount=stripLength;
   
   DLZWSetupDecompress(&lzwStream);
   int error;
   
   if((error=DLZWDecompressLine(&lzwStream,consumer,stripLength))==0)
    NSLog(@"error=%d",error);
   
   O2DataConsumerRelease(consumer);
   
   return outputData;
}


