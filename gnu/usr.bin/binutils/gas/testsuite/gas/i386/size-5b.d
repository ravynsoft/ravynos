#name: i386 size 5 (data)
#source: size-5.s
#objdump: -rsj .data

.*: +file format .*

RELOCATION RECORDS FOR \[\.data\]:

OFFSET +TYPE +VALUE
0*18 R_386_SIZE32 *ext
0*1c R_386_SIZE32 *ext

Contents of section .data:
 0+00 43 ?00 ?00 ?00 43 ?10 ?00 ?00 bd ?ff ?ff ?ff bd ?ff ?ff ?ff .*
 0+10 bd ?00 ?00 ?00 bd ?0f ?00 ?00 00 ?00 ?00 ?00 00 ?10 ?00 ?00 .*
