/*
 * Copyright (c) 2011 Apple Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "basexx.h"
#include <CommonNumerics/CommonBaseXX.h>
#include "CommonBufferingPriv.h"
#include "ccGlobals.h"
#include "cc_macros_priv.h"

const static encoderConstants encoderValue[] = {
    { 16, 4, 1, 2, 0x0f }, // Base16
    { 32, 5, 5, 8, 0x1f }, // Base32
    { 64, 6, 3, 4, 0x3f }, // Base64
};

typedef struct _CNEncoder {
    CoderFrame coderFrame;
    CNEncodingDirection direction;
    CNBufferRef base256buffer;
    CNBufferRef baseXXbuffer;
} CNEncoder;

/*
 * Pre-defined encoders.
 */

#define DEFAULTPAD		'='

const BaseEncoder defaultBase64 = {
    .name = "Base64",
    .encoding = kCNEncodingBase64,
    .baseNum = 64,
    .charMap = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
    .padding = DEFAULTPAD,
    .values = &encoderValue[2]
};

// RFC 4678 Base32Alphabet
const BaseEncoder defaultBase32 = {
    .name = "Base32",
    .encoding = kCNEncodingBase32,
    .baseNum = 32,
    .charMap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567",
    .padding = DEFAULTPAD,
    .values = &encoderValue[1]
};

const BaseEncoder recoveryBase32 = {
    .name = "RecoveryBase32",
    .encoding = kCNEncodingBase32Recovery,
    .baseNum = 32,
    .charMap = "ABCDEFGH8JKLMNOPQR9TUVWXYZ234567",
    .padding = DEFAULTPAD,
    .values = &encoderValue[1]
};

const BaseEncoder hexBase32 = {
    .name = "Base32Hex",
    .encoding = kCNEncodingBase32HEX,
    .baseNum = 32,
    .charMap = "0123456789ABCDEFGHIJKLMNOPQRSTUV",
    .padding = DEFAULTPAD,
    .values = &encoderValue[1]
};


const BaseEncoder defaultBase16 = {
    .name = "Base16",
    .encoding = kCNEncodingBase16,
    .baseNum = 16,
    .charMap = "0123456789ABCDEF",
    .padding = DEFAULTPAD,
    .values = &encoderValue[0]
};


/*
    Utility functions
 */


static inline uint32_t baselog(CNEncoder *coderRef)
{
    if(coderRef && coderRef->coderFrame && coderRef->coderFrame->encoderRef && coderRef->coderFrame->encoderRef->values)
        return coderRef->coderFrame->encoderRef->values->log;
    else return 0;
}

static inline uint32_t basemask(CNEncoder *coderRef)
{
    return coderRef->coderFrame->encoderRef->values->basemask;
}

static inline uint32_t inputBlocksize(CNEncoder *coderRef)
{
    if(coderRef && coderRef->coderFrame && coderRef->coderFrame->encoderRef && coderRef->coderFrame->encoderRef->values)
        return coderRef->coderFrame->encoderRef->values->inputBlocksize;
    else return 0;
}

static inline uint32_t outputBlocksize(CNEncoder *coderRef)
{
    if(coderRef && coderRef->coderFrame && coderRef->coderFrame->encoderRef && coderRef->coderFrame->encoderRef->values)
        return coderRef->coderFrame->encoderRef->values->outputBlocksize;
    else return 0;
}

static inline uint8_t encodeToBase(CNEncoder *coderRef, uint8_t inByte)
{
    if(inByte < coderRef->coderFrame->encoderRef->baseNum) return coderRef->coderFrame->encoderRef->charMap[inByte];
    return 0x80;
}

static inline uint8_t decodeFromBase(CNEncoder *coderRef, uint8_t inByte)
{
    return coderRef->coderFrame->reverseMap[inByte];
}

static inline size_t decodeLen(void *ctx, size_t len)
{
    CNEncoder *coderRef = (CNEncoder *) ctx;
    if(0 == len) return 0;
    return (baselog(coderRef) * len + 8 ) / 8;    
}

static inline size_t encodeLen(void *ctx, size_t len)
{
    CNEncoder *coderRef = (CNEncoder *) ctx;
    if(0 == len || 0 == inputBlocksize(coderRef) || 0 == outputBlocksize(coderRef)) return 0;
    return ((len + inputBlocksize(coderRef) - 1) / inputBlocksize(coderRef)) * outputBlocksize(coderRef);    
}


/*
 * This takes raw data from base XX (where XX is "base") and puts it into base256 form.
 */

static int
deCode(void *ctx, const void *in, size_t srcLen, void *out, size_t *destLen)
{
    const uint8_t *src = in;
    uint8_t *dest = out;
    CNEncoder *coderRef = (CNEncoder *) ctx;
    size_t i;
    size_t dPos = 0;
    int sourceBits = baselog(coderRef);
    
    if(coderRef == NULL || coderRef->coderFrame == NULL || coderRef->coderFrame->encoderRef == NULL) return 0;
    if((*destLen = decodeLen(coderRef, srcLen)) == 0) {
        *dest = 0;
        return 0;
    }

    cc_clear(*destLen, dest);

    for(i=0; i<srcLen; i++) {
        if(src[i] != coderRef->coderFrame->encoderRef->padding) {
            uint8_t srcByte = decodeFromBase(coderRef, src[i]);
            int dBit = (i*sourceBits) % 8; // destination position of Leftmost Bit of source byte
            int shiftl = (8-sourceBits) - dBit; // amount needed to shift left to get bits positioned
            dPos = (i*sourceBits) / 8; // destination byte of leftmost bit of source byte
            
            if(shiftl >= 0) {
                dest[dPos] |= srcByte << shiftl;
            } else if(shiftl < 0) {
                int shiftr = shiftl * (-1);
                dest[dPos] |= srcByte >> shiftr;
                dest[dPos+1] |= srcByte << (8-shiftr);
            } 
        }
    }
    *destLen = (dest[dPos+1]) ? dPos+2: dPos+1;
    return 0;
}

/*
 * This takes "normal" base256 encoding and puts it into baseXX (where XX is "base") raw data.
 */

static int
enCode(void *ctx, const void *in, size_t srcLen, void *out, size_t *destLen)
{
    const uint8_t *src = in;
    uint8_t *dest = out;
    CNEncoder *coderRef = (CNEncoder *) ctx;
    size_t i;
    int destBits = baselog(coderRef);
    int baseShift = 8 - destBits;
    size_t needed, dPos;    

    if((needed = encodeLen(coderRef, srcLen)) == 0) {
        *destLen = 0;
        *dest = 0;
        return 0;
    }
    
    if(*destLen < needed) {
        *destLen = needed;
        return -1;
    }

    *destLen = needed;

    cc_clear(needed, dest);

    dPos = 0;
    for(i=0; i<srcLen; i++) {
        int dBit = (i*8) % destBits;
        dPos = (i*8) / destBits;
        int shiftr = baseShift + dBit;

        dest[dPos] |= (src[i] >> shiftr) & basemask(coderRef);
        if(shiftr > destBits) {
            shiftr = shiftr - destBits;
            dPos++;
            dest[dPos] |= (src[i] >> shiftr) & basemask(coderRef);
        } 
        dest[dPos+1] |= (src[i] << (destBits - shiftr)) & basemask(coderRef);
    }
    dPos+=2;
    
    for(i=0; i<dPos; i++) dest[i] = encodeToBase(coderRef, dest[i]);
    
    for(; dPos < needed; dPos++) dest[dPos] = coderRef->coderFrame->encoderRef->padding;
    dest[dPos] = 0;
    return 0;
}

static CoderFrame
getCodeFrame(CNEncodings encoding)
{
    if(encoding >= CN_STANDARD_BASE_ENCODERS) return NULL;
    
    cc_globals_t globals = _cc_globals();
    return &globals->encoderTab[encoding];
}

/*
 Interface functions
 */

CNStatus CNEncoderCreate(CNEncodings encoding, 
                         CNEncodingDirection direction,
                         CNEncoderRef *encoderRef)
{
    if(direction != kCNEncode && direction != kCNDecode) return kCNParamError;
    if(!encoderRef) return kCNParamError;
    *encoderRef = NULL;
    CoderFrame codeFrameOrigin = getCodeFrame (encoding);
    if(!codeFrameOrigin) return kCNParamError;
    CNEncoder *coderRef = malloc(sizeof(CNEncoder));
    if(!coderRef) return kCNMemoryFailure;

    CoderFrame codeFrame = malloc(sizeof(BaseEncoderFrame));
    if(!codeFrame) {
        free(coderRef);
        return kCNMemoryFailure;
    }
    memcpy(codeFrame, codeFrameOrigin,sizeof(BaseEncoderFrame));

    coderRef->direction = direction;
    coderRef->coderFrame = codeFrame;
    coderRef->base256buffer = NULL;
    coderRef->baseXXbuffer = NULL;
    coderRef->base256buffer = CNBufferCreate(inputBlocksize(coderRef));
    coderRef->baseXXbuffer = CNBufferCreate(outputBlocksize(coderRef));
    if(!coderRef->base256buffer || !coderRef->baseXXbuffer) {
        if(coderRef->base256buffer) CNBufferRelease(&coderRef->base256buffer);
        if(coderRef->baseXXbuffer) CNBufferRelease(&coderRef->baseXXbuffer);
        free(coderRef);
        return kCNMemoryFailure;
    }
    *encoderRef = coderRef;
    return kCNSuccess;
}


void setReverseMap(CoderFrame frame)
{
    int i;
    memset(frame->reverseMap, 0x80, CC_BASE_REVERSE_MAP_SIZE);
    for(i=0; i<frame->encoderRef->baseNum; i++) {
        int idx;
        idx = (unsigned char)frame->encoderRef->charMap[i];
        frame->reverseMap[idx] = (uint8_t)i;
    }
}

CNStatus CNEncoderCreateCustom(const void *name,
                               const uint8_t baseNum,
                               const void *charMap,
                               const char padChar,
                               CNEncodingDirection direction,
                               CNEncoderRef *encoderRef)
{
    if(direction != kCNEncode && direction != kCNDecode) return kCNParamError;
    if(baseNum != 16 && baseNum != 32 && baseNum != 64) return kCNParamError;
    if(!encoderRef || !charMap) return kCNParamError;
    *encoderRef = NULL;

    CoderFrame codeFrame = malloc(sizeof(BaseEncoderFrame));
    BaseEncoderRefCustom customEncoder = malloc(sizeof(BaseEncoder));
    CNEncoder *coderRef = malloc(sizeof(CNEncoder));

    if(coderRef) {
        coderRef->base256buffer = NULL;
        coderRef->baseXXbuffer = NULL;
    }
    
    CNStatus retval = kCNMemoryFailure;
    if(!codeFrame || !customEncoder || !coderRef) goto errOut;
    
    customEncoder->baseNum = baseNum;
    customEncoder->values = &encoderValue[baseNum/32];
    customEncoder->name = name;
    customEncoder->charMap = charMap;
    customEncoder->padding = padChar;
    customEncoder->encoding = kCNEncodingCustom;

    codeFrame->encoderRef = customEncoder;
    setReverseMap(codeFrame);

    coderRef->coderFrame = codeFrame;
    coderRef->direction = direction;
    coderRef->base256buffer = CNBufferCreate(inputBlocksize(coderRef));
    coderRef->baseXXbuffer = CNBufferCreate(outputBlocksize(coderRef));
    if(!coderRef->base256buffer || !coderRef->baseXXbuffer) goto errOut;
    
    *encoderRef = coderRef;
    return kCNSuccess;
    
errOut:
    free(codeFrame);
    free(customEncoder);

    if(coderRef) {
        if(coderRef->base256buffer) CNBufferRelease(&coderRef->base256buffer);
        if(coderRef->baseXXbuffer) CNBufferRelease(&coderRef->baseXXbuffer);
        free(coderRef);
    }
    return retval;

}

CNStatus CNEncoderRelease(CNEncoderRef *encoderRef)
{
    CNEncoder *coderRef = *encoderRef;
    *encoderRef = NULL;
    if(coderRef) {
        CoderFrame codeFrame = coderRef->coderFrame;
        if(codeFrame->encoderRef && kCNEncodingCustom == codeFrame->encoderRef->encoding) {
            BaseEncoderRefCustom customRef = (BaseEncoderRefCustom) codeFrame->encoderRef;
            free((void *) customRef);
        }
        if(coderRef->base256buffer) CNBufferRelease(&coderRef->base256buffer);
        if(coderRef->baseXXbuffer) CNBufferRelease(&coderRef->baseXXbuffer);
        free(codeFrame);
        free(coderRef);
    }
    return kCNSuccess;
}


size_t
CNEncoderGetOutputLength(CNEncoderRef encoderRef, const size_t inLen)
{
    CNEncoder *coderRef = encoderRef;
    size_t retval = 0;
    
    __Require_Quiet(NULL != coderRef, errOut);

    if(coderRef->direction == kCNEncode) {
        retval = encodeLen(coderRef, inLen + coderRef->base256buffer->bufferPos) + 1;
    } else if(coderRef->direction == kCNDecode) {
        retval = decodeLen(coderRef, inLen + coderRef->baseXXbuffer->bufferPos);
    }
    
errOut:
    return retval;
}


size_t
CNEncoderGetOutputLengthFromEncoding(CNEncodings encoding, CNEncodingDirection direction, const size_t inLen)
{
    size_t retval = 0;
    CNEncoderRef coder;
    CNStatus status;
    
    if((status = CNEncoderCreate(encoding, direction, &coder))) return 0;
    
    if(direction == kCNEncode) {
        retval = encodeLen(coder, inLen) + 1;
    } else if(direction == kCNDecode) {
        retval = decodeLen(coder, inLen);
    }
    
    CNEncoderRelease(&coder);
    
    return retval;
}


CNStatus
CNEncoderUpdate(CNEncoderRef coderRef, const void *in, const size_t inLen, void *out, size_t *outLen)
{
    CNStatus retval = kCNParamError;
    CNEncoder *encoderRef = coderRef;
//    size_t outputLen, outputAvailable;
    
    
    __Require_Quiet(NULL != coderRef, errOut);
    __Require_Quiet(NULL != out, errOut);
    __Require_Quiet(NULL != outLen, errOut);
    
    if(NULL == in) {
        if(0 == inLen) {
            *outLen = 0;
            return kCNSuccess;
        }
        return kCNParamError;
    }
    
//    outputAvailable = outputLen = *outLen;
    
    if(encoderRef->direction == kCNEncode) {
        retval = CNBufferProcessData(coderRef->base256buffer, coderRef, in, inLen, out, outLen, enCode, encodeLen);
    } else if(encoderRef->direction == kCNDecode) {
        retval = CNBufferProcessData(coderRef->baseXXbuffer, coderRef, in, inLen, out, outLen, deCode, decodeLen);
    } else {
        retval = kCNParamError;
    }
    
errOut:
    return retval;
}

CNStatus
CNEncoderFinal(CNEncoderRef coderRef, void *out, size_t *outLen)
{
    CNStatus retval = kCNParamError;
    CNEncoder *encoderRef = coderRef;

    __Require_Quiet(NULL != coderRef, errOut);
    __Require_Quiet(NULL != out, errOut);
    __Require_Quiet(NULL != outLen, errOut);
    
    if(encoderRef->direction == kCNEncode) {
        if((encodeLen(coderRef, coderRef->baseXXbuffer->bufferPos)+1) > *outLen) {
            // We need room for the final '\0' on the encoded string.
            retval = kCNBufferTooSmall;
            goto errOut;
        }
        retval = CNBufferFlushData(coderRef->base256buffer, coderRef, out, outLen, enCode, encodeLen);
        if(kCNSuccess == retval) {
            ((uint8_t *)out)[*outLen] = 0;
        }
    } else if(encoderRef->direction == kCNDecode) {
        retval = CNBufferFlushData(coderRef->baseXXbuffer, coderRef, out, outLen, deCode, decodeLen);
    } else {
        retval = kCNParamError;
    }
    

errOut:
    return retval;
    
}


CNStatus
CNEncoderBlocksize(CNEncodings encoding, size_t *inputSize, size_t *outputSize)
{
    CNEncoderRef coder;
    CNStatus status;
    
    __Require_Quiet(NULL != inputSize, errOut);
    __Require_Quiet(NULL != outputSize, errOut);
    
    if((status = CNEncoderCreate(encoding, kCNEncode, &coder))) return status;

    *inputSize = inputBlocksize(coder);
    *outputSize = outputBlocksize(coder);
    CNEncoderRelease(&coder);

    return kCNSuccess;
errOut:
    return kCNParamError;
}

CNStatus
CNEncoderBlocksizeFromRef(CNEncoderRef encoderRef, size_t *inputSize, size_t *outputSize)
{
    __Require_Quiet(NULL != encoderRef, errOut);
    __Require_Quiet(NULL != inputSize, errOut);
    __Require_Quiet(NULL != outputSize, errOut);
    
    *inputSize = inputBlocksize(encoderRef);
    *outputSize = outputBlocksize(encoderRef);
    return kCNSuccess;
errOut:
    return kCNParamError;
}

CNStatus CNEncode(CNEncodings encoding, 
                  CNEncodingDirection direction,
                  const void *in, const size_t inLen, 
                  void *out,  size_t *outLen)
{
    CNStatus retval;
    size_t outAvailable, currentlyAvailable;
    CNEncoderRef encoder;
    uint8_t *outPtr = out;
    
    retval = kCNParamError;
    __Require_Quiet(NULL != out, outReturn);
    __Require_Quiet(NULL != outLen, outReturn);
    __Require_Quiet(NULL != in, outReturn);
    
    retval = CNEncoderCreate(encoding, direction, &encoder);
    __Require_Quiet(kCNSuccess == retval, outReturn);
    
    currentlyAvailable = outAvailable = *outLen;
    *outLen = 0;
    
    retval = CNEncoderUpdate(encoder, in, inLen, outPtr, &currentlyAvailable);
    __Require_Quiet(kCNSuccess == retval, outReturn);
        
    *outLen = currentlyAvailable;
    outAvailable -= currentlyAvailable;
    outPtr += currentlyAvailable;
    currentlyAvailable = outAvailable;
    
    retval = CNEncoderFinal(encoder, outPtr, &currentlyAvailable);
    __Require_Quiet(kCNSuccess == retval, outReturn);
        
    *outLen += currentlyAvailable;
    
    retval = CNEncoderRelease(&encoder);
    
outReturn:
    return retval;

}

