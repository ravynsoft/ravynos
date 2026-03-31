/*
 * lzvndecode - Command line tool to quickly decode LZVN files
 *
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <arpa/inet.h>
#include "lzvn_decode.h"

int main(int argc, char **argv)
{
  uint32_t signature, comptype, compsize, uncompsize, crc;
  char *src, *dst;
  int isBE = 0;

  if(argc != 3) {
    printf("usage: lzvndecode infile outfile\n");
    return 1;
  }
  
  FILE *fp = fopen(argv[1], "r");
  fseek(fp, 0, SEEK_END);
  size_t len = ftell(fp);
  rewind(fp);

  src = malloc(len);
  if(!src) {
    perror("malloc failed");
    return 1;
  }

  printf("read %d bytes\n", fread(src, 1, len, fp));
  fclose(fp);

  PrelinkedKernelHeader *pkh = (PrelinkedKernelHeader *)src;
  signature = pkh->signature;
  if(signature == 0x636f6d70 /* comp */)
    isBE = 0;
  else if(signature == 0x706d6f63 /* pmoc */)
    isBE = 1;
  else {
    puts("unrecognized file header");
    return 1;
  }

  compsize = isBE ? ntohl(pkh->compressedSize) : pkh->compressedSize;
  uncompsize = isBE ? ntohl(pkh->uncompressedSize) : pkh->uncompressedSize;
  crc = isBE ? ntohl(pkh->adler32) : pkh->adler32;

  printf("prelinked kernel: %d bytes comp, %d bytes uncomp [crc %08x]\n",
	 compsize, uncompsize, crc);

  dst = calloc(1, uncompsize); // zero fill the sections for us
  if(!dst) {
    perror("malloc failed");
    free(src);
    return 1;
  }

  lzvn_decoder_state state = {0};
  state.src = src + sizeof(PrelinkedKernelHeader);
  state.src_end = state.src + compsize;
  state.dst = dst;
  state.dst_begin = dst;
  state.dst_end = dst + uncompsize;

  lzvn_decode(&state);
  
  fp = fopen(argv[2], "w");
  printf("wrote %d bytes to `%s`\n", fwrite(dst, 1, uncompsize, fp), argv[2]);
  fclose(fp);

  free(dst);
  free(src);
  return 0;
}
