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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <libgen.h>
#include <ctype.h>
#include <AssertMacros.h>
#include <CommonNumerics/CommonCRC.h>
#include <CommonNumerics/CommonBaseXX.h>

#define CN_NAME "cn"
#define ENCODE_DEFAULT_WIDTH 64

#define PRINT(...)  if (context->verbose) { fprintf(context->out_file, __VA_ARGS__); }

#define CN_ITEM(item)   { #item, item }

typedef struct _cnItem
{
    const char  *name;
    uint32_t    alg;
} cnItem;

static cnItem crcMap[] = {
    CN_ITEM(kCN_CRC_8),
    CN_ITEM(kCN_CRC_8_ICODE),
    CN_ITEM(kCN_CRC_8_ITU),
    CN_ITEM(kCN_CRC_8_ROHC),
    CN_ITEM(kCN_CRC_8_WCDMA),
    CN_ITEM(kCN_CRC_16),
    CN_ITEM(kCN_CRC_16_CCITT_TRUE),
    CN_ITEM(kCN_CRC_16_CCITT_FALSE),
    CN_ITEM(kCN_CRC_16_USB),
    CN_ITEM(kCN_CRC_16_XMODEM),
    CN_ITEM(kCN_CRC_16_DECT_R),
    CN_ITEM(kCN_CRC_16_DECT_X),
    CN_ITEM(kCN_CRC_16_ICODE),
    CN_ITEM(kCN_CRC_16_VERIFONE),
    CN_ITEM(kCN_CRC_16_A),
    CN_ITEM(kCN_CRC_16_B),
    CN_ITEM(kCN_CRC_16_Fletcher),
    CN_ITEM(kCN_CRC_32_Adler),
    CN_ITEM(kCN_CRC_32),
    CN_ITEM(kCN_CRC_32_CASTAGNOLI),
    CN_ITEM(kCN_CRC_32_BZIP2),
    CN_ITEM(kCN_CRC_32_MPEG_2),
    CN_ITEM(kCN_CRC_32_POSIX),
    CN_ITEM(kCN_CRC_32_XFER),
    CN_ITEM(kCN_CRC_64_ECMA_182)
};

static cnItem basexxMap[] = {
    CN_ITEM(kCNEncodingBase64),
    CN_ITEM(kCNEncodingBase32),
    CN_ITEM(kCNEncodingBase32Recovery),
    CN_ITEM(kCNEncodingBase32HEX),
    CN_ITEM(kCNEncodingBase16),
};

enum {
    cmdOpCRC = 1,
    cmdOpEncode,
    cmdOpDecode
};
typedef uint32_t cmdOp; //operation

typedef struct _cnCmd
{
    const char * name;
    cmdOp op;
    const char * options; 
    const char * description;
    const char * usage;
    uint32_t    algDefault;
} cnCmd, *cnCmdPtr;

typedef struct _cnContext
{
    cnCmdPtr    cmd;
    FILE        *out_file;
    int         fd;
    const char  *file;
    const char  **files;
    uint32_t    filesCount;
    const char  *string;
    uint32_t    alg;
    bool        showDecimal;
    bool        dumpTable;
    int         pageSize;
    int         width;
    uint64_t    totalBytes;
    bool        verbose;
} cnContext, *cnContextPtr;

static cnCmd cmdMap[] =
{
    {   .name = "crc",
        .op = cmdOpCRC,
        .options = "a:ds:Th?v",
        .description = "Generate a checksum CRC",
        .usage =    "[file ...]\n",
        .algDefault = kCN_CRC_64_ECMA_182,
    },
    {   .name = "encode",
        .op = cmdOpEncode,
        .options = "a:s:h?v",
        .description = "Encode using BaseXX representation",
        .usage = "[file ...]\n",
        .algDefault = kCNEncodingBase64,
    },
    {   .name = "decode",
        .op = cmdOpDecode,
        .options = "a:s:h?v",
        .description = "Decode using BaseXX representation",
        .usage = "[file ...]\n",
        .algDefault = kCNEncodingBase64,
    }
};

static cnCmdPtr getcmd(const char * str)
{
    if (!str)
        return NULL;
    
    int num = sizeof(cmdMap)/sizeof(cnCmd);
    size_t cmdlen = strlen(str);
    int count = 0;
    cnCmdPtr foundCmd = NULL;
    for (int x = 0; x < num; x++) {
        if (strncasecmp(cmdMap[x].name, str, cmdlen) == 0) {
            count++;
            foundCmd = &cmdMap[x];
        }
    }
    
    if (count > 1) {
        fprintf(stderr, "error: %s is ambiguous found %d matches\n", str, count);
        return NULL;
    } else {
        return foundCmd;
    }
}

static uint32_t getalg(cnContextPtr context, const char * name)
{
    if (!name)
        return 0;
    const cnItem * algMap = NULL;
    int num;
    
    switch (context->cmd->op) {
        case cmdOpCRC:
            algMap = crcMap;
            num = sizeof(crcMap)/sizeof(cnItem);
            break;
        case cmdOpDecode:
        case cmdOpEncode:
            algMap = basexxMap;
            num = sizeof(basexxMap)/sizeof(cnItem);
            break;
        default:
            return 0;
    }
    
    uint32_t alg = 0;
    for (int x = 0; x < num; x++) {
        size_t nlen = strlen(algMap[x].name);
        if (strncasecmp(algMap[x].name, name, nlen) == 0) {
            alg = algMap[x].alg;
            break;
        }
    }
    
    return alg;
}

#define USAGE_SPACE "16"

static void usage(cnContextPtr context)
{
    if (!context)
        return;
    
    if (context->cmd == NULL) {
        fprintf(stderr, "usage: cn [command] [opt ...]\n\n");
        fprintf(stderr, "'cn command -h' for more info\n\n");
        fprintf(stderr, "commands:\n");
        int num = sizeof(cmdMap)/sizeof(cnCmd);
        for (int x = 0; x < num; x++) {
            if (cmdMap[x].description != NULL) {
                fprintf(stderr, "  %-20s%-s\n", cmdMap[x].name, cmdMap[x].description);
            }
        }
    } else {
        cnCmdPtr cmd = context->cmd;
        char * optBuf = (char*)malloc(strlen(cmd->options));
        char * cur = optBuf;
        const char * pos = cmd->options;
        while (*pos != '\0') {
            if (*pos != ':' && *pos != '?') {
                *cur = *pos;
                cur++;
            }
            pos++;
        }
        *cur = '\0';
        
        if (cmd->usage) {
            fprintf(stderr, "usage: %s [-%s] %s\n", cmd->name, optBuf, cmd->usage);
        } else {
            fprintf(stderr, "usage: %s [-%s]\n", cmd->name, optBuf);
        }
        free(optBuf);
        
        const char * ch = cmd->options;
        while (*ch != '\0') {
            switch (*ch) {
                case 'a':
                    fprintf(stderr, "  %-"USAGE_SPACE"s%-s\n", "-a <num|string>", "Operate with a specific Algorithm");
                    break;
                case 'd':
                    fprintf(stderr, "  %-"USAGE_SPACE"s%-s\n", "-d", "Display CRC in decimal");
                    break;
                case 'h':
                    fprintf(stderr, "  %-"USAGE_SPACE"s%-s\n", "-h, -?", "Show help");
                    break;
                case 's':
                    fprintf(stderr, "  %-"USAGE_SPACE"s%-s\n", "-s <string>", "Operate on a specified string");
                    break;
                case 'v':
                    fprintf(stderr, "  %-"USAGE_SPACE"s%-s\n", "-v", "verbose");
                    break;
                default:
                    break;
            }
            ch++;
        }
        
        switch (cmd->op) {
            case cmdOpCRC:
                {
                    fprintf(stderr, "\nCRC Algorithms\n");
                    int c = sizeof(crcMap)/sizeof(cnItem);
                    for (int x = 0; x < c; x++) {
                        fprintf(stderr, "%s %-1i - %s\n", crcMap[x].alg == context->alg ? "*" : " ", crcMap[x].alg, crcMap[x].name);
                    }
                }
                break;
            case cmdOpDecode:
            case cmdOpEncode:
                {
                    fprintf(stderr, "\nBaseXX Algorithms\n");
                    int c = sizeof(basexxMap)/sizeof(cnItem);
                    for (int x = 0; x < c; x++) {
                        fprintf(stderr, "%s %-1i - %s\n", basexxMap[x].alg == context->alg ? "*" : " ", basexxMap[x].alg, basexxMap[x].name);
                    }
                }
                break;
            default:
                break;
        }
    }
    
}

static int getEnvInt(const char * str)
{
    if (!str)
        return 0;
    
    const char * env = getenv(str);
    return env ? atoi(env) : 0;
}

static uint32_t parseAlg(cnContextPtr context, const char * str)
{
    uint32_t alg = 0;
    
    if (!str)
        return 0;
    
    alg = atoi(str);
    if (!alg)
        alg = getalg(context, str);
    if (!alg)
        fprintf(stderr, "Algorithm not found %s\n", str);
    
    return alg;
}

static bool parseArgs(int argc, const char * argv[], cnContextPtr context)
{
    bool result = false;
    int ch;
    char * cmd = NULL;
    int envInt;

    cmd = basename((char*)argv[0]);
    if (strcasecmp(CN_NAME, cmd) == 0) {
        cmd = NULL;
    }
    require_quiet(cmd || argc > 1, done);
    
    context->cmd = getcmd(cmd ? cmd : argv[1]);
    require_quiet(context->cmd != NULL, done);
    
    if (!cmd) {
        argc -= 1;
        argv += 1;
    }

    context->alg = parseAlg(context, getenv("CN_ALG"));
    
    if (!context->alg) {
        context->alg = context->cmd->algDefault;
    }
    
    context->out_file = fdopen(STDOUT_FILENO, "a");
    
    while ((ch = getopt(argc, (char**)argv,context->cmd->options)) != -1) {
        switch (ch) {
            case 'a':
                context->alg = parseAlg(context, optarg);
                break;
            case 'd':
                context->showDecimal = true;
                break;
            case 'T':
                context->dumpTable = true;
                context->string = "";
                break;
            case 's':
                context->string = optarg;
                break;
            case 'v':
                context->verbose = true;
                break;
            case '?':
            case 'h':
            default:
                goto done;
                break;
        }
    }
    
    envInt = getEnvInt("CN_READ_SIZE");
    context->pageSize = envInt ? envInt : getpagesize();
    
    envInt = getEnvInt("CN_WIDTH");
    context->width = envInt ? envInt : ENCODE_DEFAULT_WIDTH;
    
    argc -= optind;
    argv += optind;
    
    if (argc > 0) {
        context->filesCount = argc;
        context->files = calloc(1u,argc*sizeof(char*)+1);
        for (int i = 0; i < argc; i++) {
            context->files[i] = argv[i];
        }
    }
    
    result = true;
    
done:
    if (!result) {
        usage(context);
    }
    return result;
}

static void pcrc(cnContextPtr context, uint64_t crc)
{
    if (context->showDecimal) {
        fprintf(context->out_file, "%llu", crc);
    } else {
        fprintf(context->out_file, "%llx", crc);
    }
    PRINT(" %i", context->alg);
    if (context->file)
        PRINT(" %s", context->file);
    PRINT(" %llu", context->totalBytes);
    fprintf(context->out_file, "\n");
}

static CNStatus crcOp(cnContextPtr context)
{
    CNStatus status = kCNSuccess;
    CNCRCRef crcRef = NULL;
    uint64_t crc = 0;
    
    if(context->dumpTable) {
        status = CNCRCDumpTable(context->alg);
        require_noerr(status, done);
        return status;
    } else  if (context->string) {
        size_t sLen = strlen(context->string);
        status = CNCRC(context->alg, context->string, sLen, &crc);
        require_noerr(status, done);
        
        context->totalBytes = sLen;
    } else {
        status = CNCRCInit(context->alg, &crcRef);
        require_noerr(status, done);
        
        char buf[context->pageSize];
        size_t nr;
        
        while ((nr = read(context->fd, buf, sizeof(buf))) > 0) {
            status = CNCRCUpdate(crcRef, buf, nr);
            require_noerr(status, done);
            
            context->totalBytes += nr;
        }
        
        status = CNCRCFinal(crcRef, &crc);
        require_noerr(status, done);
    }
    
    pcrc(context, crc);

done:
    if (crcRef) {
        CNCRCRelease(crcRef);
    }
    return status;
}

static void pbase(cnContextPtr context, CNEncodingDirection direction, uint8_t * buf, size_t buf_len)
{
    switch(direction) {
        case kCNEncode:
            {
                size_t pos = 0, size;
                while(buf_len) {
                    size = buf_len > (size_t) context->width ? context->width : buf_len;
                    fwrite(&buf[pos], size, 1, context->out_file);
                    fprintf(context->out_file, "\n");
                    buf_len -= size;
                    pos += size;
                }
            }
            break;
        
        case kCNDecode:
            fwrite(buf, buf_len, 1, context->out_file);
            fflush(context->out_file);
            break;
        default:
            break;
    }
}

static void _getBlockReadSize(cnContextPtr context, size_t * readSize, size_t * encodeSize)
{
    size_t encode, in, out;
    
    CNEncoderBlocksize(context->alg, &in, &out);

    encode = (context->pageSize / in) * out;
    encode = encode - (encode % context->width);
    *readSize = (encode / out) * in;
    *encodeSize = encode;
}

static void _sanitizeBuf(uint8_t * buf, size_t bufLen, uint8_t * outBuf, size_t * outLen)
{
    uint8_t * cur = outBuf;
    size_t len = bufLen;
    for (size_t i = 0; i < bufLen; i++) {
        if (buf[i] != '\n') {
            *cur = buf[i];
            cur++;
        } else {
            len--;
        }
    }
    *outLen = len;
}

static CNStatus basexxOp(cnContextPtr context)
{
    CNStatus status = kCNSuccess;
    CNEncoderRef encoder = NULL;
    uint8_t * encodeBuf = NULL;
    uint8_t * sanitizedBuf = NULL;
    size_t encodedSize = 0;
    CNEncodingDirection direction = context->cmd->op == cmdOpEncode ? kCNEncode : kCNDecode;
        
    if (context->string) {
        size_t sLen = strlen(context->string);
        encodedSize = CNEncoderGetOutputLengthFromEncoding(context->alg, direction, sLen);
        
        encodeBuf = calloc(1u, encodedSize);
        require(encodeBuf != NULL, done);
        
        CNEncode(context->alg, direction, context->string, sLen, encodeBuf, &encodedSize);
        require_noerr_action(status, done, status = kCNDecodeError);
        
        pbase(context, direction, encodeBuf, encodedSize);
        
        context->totalBytes = sLen;
    } else {
        uint8_t readBuf[context->pageSize];
        size_t bytesRead;
        size_t pos;
        size_t encodeLen = 0;

        status = CNEncoderCreate(context->alg, direction, &encoder);
        require_noerr_action(status, done, status = kCNDecodeError);
        
        size_t readBufSize = context->pageSize, expectedSize = 0;
        if (direction == kCNEncode) {
            _getBlockReadSize(context, &readBufSize, &expectedSize); // align reads to context->width
            assert(readBufSize <= (size_t) context->pageSize);
        }
        
        encodeLen = CNEncoderGetOutputLength(encoder, readBufSize);
        encodeBuf = calloc(1u, encodeLen);
        require(encodeBuf != NULL, done);
        
        while ((bytesRead = read(context->fd, readBuf, readBufSize)) > 0) {
            encodedSize = encodeLen;
            
            if (direction == kCNDecode) {
                if (!sanitizedBuf) {
                    sanitizedBuf = calloc(1u, context->pageSize);
                }
                size_t sanitizedSize = 0;
                _sanitizeBuf(readBuf, bytesRead, sanitizedBuf, &sanitizedSize);
                status = CNEncoderUpdate(encoder, sanitizedBuf, sanitizedSize, encodeBuf, &encodedSize);
                expectedSize = encodedSize; 
            } else {
                status = CNEncoderUpdate(encoder, readBuf, bytesRead, encodeBuf, &encodedSize);
            }
            require_noerr_action(status, done, status = kCNDecodeError);
            
            // during encode only print if we got the full expected encode size (we might need to add padding)
            // during decode print all decoded bytes
            if (encodedSize == expectedSize) {
                pbase(context, direction, encodeBuf, encodedSize);
            }
            
            context->totalBytes += bytesRead;
        }
        
        pos = encodedSize;
        encodedSize = encodeLen - pos;
        
        status = CNEncoderFinal(encoder, &encodeBuf[pos], &encodedSize);
        require_noerr_action(status, done, status = kCNDecodeError);
        
        // If we had to add padding or didn't print the full buffer from above do so now
        if (direction == kCNEncode && (encodedSize || pos < expectedSize)) {
            size_t left = pos < expectedSize ?  pos + encodedSize : encodedSize;
            // if pos < expectedSize start from 0 else just print the padding from pos
            pbase(context, direction, &encodeBuf[pos < expectedSize ? 0 : pos], left);
        }
    }
    
    PRINT("\n%llu", context->totalBytes);
    if (context->file)
        PRINT(" %s", context->file);
    PRINT("\n");
    
done:
    if (encoder) {
        CNEncoderRelease(&encoder);
    }
    if (encodeBuf) {
        free(encodeBuf);
    }
    if (sanitizedBuf) {
        free(sanitizedBuf);
    }
    
    return status;
}

static void cnContextFree(cnContextPtr context) {
    if (context->files) {
        free(context->files);
    }
    if (context->out_file) {
        fclose(context->out_file);
    }
    
    free(context);
}

typedef CNStatus (*op_f)(cnContextPtr context);

int main(int argc, const char * argv[])
{
    int rc = kCNSuccess;
    op_f op = NULL;
    
    cnContextPtr context = calloc(1u, sizeof(cnContext));
    
    context->fd = STDIN_FILENO;
    
    require_action_quiet(parseArgs(argc, argv, context) == true, done, rc = kCNParamError);

    switch (context->cmd->op) {
        case cmdOpCRC:
            op = crcOp;
            break;
        case cmdOpEncode:
        case cmdOpDecode:
            op = basexxOp;
            break;
        default:
            break;
    }
    
    require_action(op != NULL, done, rc = kCNParamError);

    const char ** pos = context->files;
    do {
        context->totalBytes = 0;
        
        if (!context->string && pos) {
            context->file = *pos++;
            
            if ((context->fd = open(context->file, O_RDONLY, 0)) < 0) {
                fprintf(stderr, "failed to open %s\n", context->file);
                rc = kCNFailure;
                continue;
            }
        }
        if (op(context) != kCNSuccess) {
            rc = kCNFailure;
        }
        close(context->fd);
        
    } while (!context->string && pos && *pos);
    

done:
    cnContextFree(context);
    return rc;
}

