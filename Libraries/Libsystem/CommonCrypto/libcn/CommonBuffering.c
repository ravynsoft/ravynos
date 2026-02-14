//
//  CommonBuffering.c
//  CommonCrypto
//

#include <stdlib.h>
#include <stdio.h>
#include <corecrypto/cc.h>
#include "CommonBufferingPriv.h"
#include "../lib/cc_macros_priv.h"

CNBufferRef
CNBufferCreate(size_t chunksize)
{
    CNBufferRef retval = malloc(sizeof(CNBuffer));
    __Require_Quiet(NULL != retval, errOut);
    retval->chunksize = chunksize;
    retval->bufferPos = 0;
    retval->buf = malloc(chunksize);
    __Require_Quiet(NULL != retval->buf, errOut);
    return retval;
    
errOut:
    if(retval) {
        if(retval->buf) free(retval->buf);
        free(retval);
    }
    return NULL;
}

CNStatus
CNBufferRelease(CNBufferRef *bufRef)
{
    CNBufferRef ref;
    
    __Require_Quiet(NULL != bufRef, out);

    ref = *bufRef;
    if(ref->buf) free(ref->buf);
    if(ref) free(ref);
out:
    return kCNSuccess;
}



CNStatus
CNBufferProcessData(CNBufferRef bufRef, 
                    void *ctx, const void *in, const size_t inLen, void *out, size_t *outLen, 
                    cnProcessFunction pFunc, cnSizeFunction sizeFunc)
{
    size_t  blocksize = bufRef->chunksize;
    const uint8_t *input = in;
    uint8_t *output = out;
    size_t inputLen = inLen, outputLen, inputUsing, outputAvailable;
    
    outputAvailable = outputLen = *outLen;
    
    if(sizeFunc(ctx, bufRef->bufferPos + inLen) > outputAvailable) return  kCNBufferTooSmall;
    *outLen = 0;
    if(bufRef->bufferPos > 0) {
        inputUsing = CC_MIN(blocksize - bufRef->bufferPos, inputLen);
        memcpy(&bufRef->buf[bufRef->bufferPos], in, inputUsing);
        bufRef->bufferPos += inputUsing;
        if(bufRef->bufferPos < blocksize) {
            return kCNSuccess;
        }
        pFunc(ctx, bufRef->buf, blocksize, output, &outputLen);
        inputLen -= inputUsing; input += inputUsing;
        output += outputLen; *outLen = outputLen; outputAvailable -= outputLen;
        bufRef->bufferPos = 0;
    }
    
    inputUsing = inputLen - inputLen % blocksize;
    if(inputUsing > 0) {
        outputLen = outputAvailable;
        pFunc(ctx, input, inputUsing, output, &outputLen);
        inputLen -= inputUsing; input += inputUsing;
        *outLen += outputLen;
    }
    
    if(inputLen > blocksize) {
        return kCNAlignmentError;
    } else if(inputLen > 0) {
        memcpy(bufRef->buf, input, inputLen);
        bufRef->bufferPos = inputLen;
    }
    return kCNSuccess;
    
}

CNStatus
CNBufferFlushData(CNBufferRef bufRef,
                  void *ctx, void *out, size_t *outLen,
                  cnProcessFunction pFunc, cnSizeFunction sizeFunc)
{
//    size_t outputLen, outputAvailable;
//    outputAvailable = outputLen = *outLen;

    if(bufRef->bufferPos > 0) {
        if(bufRef->bufferPos > bufRef->chunksize) return kCNAlignmentError;
        if(sizeFunc(ctx, bufRef->bufferPos) > *outLen) return kCNBufferTooSmall;
        pFunc(ctx, bufRef->buf, bufRef->bufferPos, out, outLen);
    } else {
        *outLen = 0;
    }
    return kCNSuccess;
}



bool
CNBufferEmpty(CNBufferRef bufRef)
{
    return bufRef->bufferPos == 0;
}
