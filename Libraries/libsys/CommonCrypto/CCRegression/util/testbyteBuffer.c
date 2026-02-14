/*
 *  printByteBuffer.c
 *  byteutils
 *
 *  Created by Richard Murphy on 3/7/10.
 *  Copyright 2010 McKenzie-Murphy. All rights reserved.
 *
 */

#include "testbyteBuffer.h"
#include <CommonCrypto/CommonRandomSPI.h>

void printBytes(uint8_t *buff, size_t len, char *name)
{
	size_t i;
	printf("Dumping %d bytes from %s\n", (int) len, name);
	for(i=0; i<len; i++) {
		if(i > 0 && !(i%8)) putchar(' ');
		if(i > 0 && !(i%64)) putchar('\n');
		printf("%02x", buff[i]);
	}
	putchar('\n');
}

void printByteBuffer(byteBuffer bb, char *name)
{
    printBytes(bb->bytes, bb->len, name);
}


byteBuffer
mallocByteBuffer(size_t len)
{
	byteBuffer retval;
	if((retval = (byteBuffer) malloc(sizeof(byteBufferStruct) + len + 1)) == NULL) return NULL;
    retval->len = len;
    retval->bytes = (uint8_t *) (retval + 1) ; /* just past the byteBuffer in malloc'ed space */
    return retval;
}

size_t
genRandomSize(size_t minSize, size_t maxSize)
{
    size_t randomInt;
    
    if(minSize == maxSize) return minSize;
    // make theSize > minSize < maxSize;
    while(CCRandomGenerateBytes(&randomInt, sizeof(uint32_t)) == -1) {
        printf("got -1 from CCRandomGenerateBytes\n");
    }
    randomInt = (randomInt % (maxSize - minSize)) + minSize;
    return randomInt;
}

byteBuffer
genRandomByteBuffer(size_t minSize, size_t maxSize)
{
	byteBuffer retval;
    size_t randomInt;
    CCCryptorStatus err;
    
    randomInt = genRandomSize(minSize, maxSize);
        
    retval = mallocByteBuffer(randomInt);
    if(retval == NULL) return NULL;
    
    if(retval->len != randomInt) return NULL;
    cc_clear(retval->len, retval->bytes);

    // fill bytes randomly
    while((err = CCRandomGenerateBytes(retval->bytes, retval->len)) != kCCSuccess) {
        printf("got %d from CCRandomCopyBytes\n", err);
    }
    
    return retval;    
}

/* utility function to convert hex character representation to their nibble (4 bit) values */
static uint8_t
nibbleFromChar(char c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 255;
}

/* Convert a string of characters representing a hex buffer into a series of bytes of that real value */
byteBuffer
hexStringToBytes(const char *inhex)
{
    byteBuffer retval;
    const uint8_t *p;
    int len,inhex_len, i=0;

    if(!inhex) inhex = "";
    inhex_len=(int)strlen(inhex);

    len = (inhex_len+1) / 2;
    if((retval = mallocByteBuffer(len)) == NULL) return NULL;

    // Special for odd length strings
    if ((inhex_len & 1) && len) {
        retval->bytes[i++] = nibbleFromChar(*(inhex));
        inhex++;
    }
    for(p = (const uint8_t *) inhex; i<len; i++) {
        retval->bytes[i] = (uint8_t)(nibbleFromChar(*p) << 4) | nibbleFromChar(*(p+1));
        p += 2;
    }
    retval->bytes[len] = 0;
    return retval;
}



byteBuffer
bytesToBytes(void *bytes, size_t len)
{
    byteBuffer retval = mallocByteBuffer(len);
    if(retval && bytes) memcpy(retval->bytes, bytes, len);
    return retval;
}

int
bytesAreEqual(byteBuffer b1, byteBuffer b2)
{
    if(b1->len != b2->len) return 0;
    return (memcmp(b1->bytes, b2->bytes, b1->len) == 0);
}


static char byteMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
static int byteMapLen = sizeof(byteMap);

/* Utility function to convert nibbles (4 bit values) into a hex character representation */
static char
nibbleToChar(uint8_t nibble)
{
	if(nibble < byteMapLen) return byteMap[nibble];
	return '*';
}

/* Convert a buffer of binary values into a hex string representation */
char
*bytesToHexString(byteBuffer bb)
{
	char *retval;
	size_t i;
	
	retval = malloc(bb->len*2 + 1);
	for(i=0; i<bb->len; i++) {
		retval[i*2] = nibbleToChar(bb->bytes[i] >> 4);
		retval[i*2+1] = nibbleToChar(bb->bytes[i] & 0x0f);
	}
    retval[bb->len*2] = 0;
	return retval;
}

char
*bytesToHexStringWithSpaces(byteBuffer bb, int breaks)
{
	char *retval;
	size_t i, j;
    
    if(breaks == 0) {
        return bytesToHexString(bb);
    }
	
    breaks /= 2;
	retval = malloc(bb->len*2 + 1 + (bb->len*2 / breaks) + 10);
	for(i=0, j=0; i<bb->len; i++, j+=2) {
		retval[j] = nibbleToChar(bb->bytes[i] >> 4);
		retval[j+1] = nibbleToChar(bb->bytes[i] & 0x0f);
        if(((i+1) % breaks) == 0) {
            retval[j+2] = ' ';
            retval[j+3] = 0;
            j++;
        }
	}
	return retval;
}


