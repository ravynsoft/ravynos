/*
 *  printByteBuffer.h
 *  byteutils
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _BYTEBUFFER_H_
#define _BYTEBUFFER_H_

typedef struct byte_buf {
    size_t  len;
    uint8_t  *bytes;
} byteBufferStruct, *byteBuffer;

void printByteBuffer(byteBuffer bb, char *name);

void printBytes(uint8_t *buff, size_t len, char *name);

byteBuffer
mallocByteBuffer(size_t len);

byteBuffer
hexStringToBytes(const char *inhex);

byteBuffer
hexStringToBytesWithSpaces(char *inhex, int breaks);

static inline byteBuffer
hexStringToBytesIfNotNULL(char *inhex) {
    if(inhex) return hexStringToBytes(inhex);
    return NULL;
}

char
*bytesToHexStringWithSpaces(byteBuffer bb, int breaks);

byteBuffer
bytesToBytes(void *bytes, size_t len);

int
bytesAreEqual(byteBuffer b1, byteBuffer b2);

char
*bytesToHexString(byteBuffer bytes);

byteBuffer
genRandomByteBuffer(size_t minSize, size_t maxSize);

size_t
genRandomSize(size_t minSize, size_t maxSize);

#endif /* _BYTEBUFFER_H_ */
