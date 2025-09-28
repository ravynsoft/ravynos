#name: i386 32-bit wrapping calculations (data)
#source: wrap32.s
#objdump: -rsj .data

.*: +file format .*

RELOCATION RECORDS FOR \[\.data\]:

OFFSET +TYPE +VALUE
0*10 (R_386_|dir)?32 *sym
0*14 (R_386_|dir)?32 *sym
0*18 (R_386_|dir)?32 *sym
0*1c (R_386_|dir)?32 *sym
0*30 (R_386_|dir)?32 *sym
0*34 (R_386_|dir)?32 *sym
0*38 (R_386_|dir)?32 *sym
0*3c (R_386_|dir)?32 *sym

Contents of section .data:
 0+.0 f4 ?00 ?00 ?00 f4 ?00 ?00 ?00 90 ?00 ?00 ?00 90 ?00 ?00 ?00 .*
 0+.0 00 ?ff ?ff ?ff 00 ?ff ?ff ?ff f4 ?00 ?00 ?00 f4 ?00 ?00 ?00 .*
 0+.0 f4 ?02 ?00 ?70 f4 ?00 ?00 ?80 90 ?02 ?00 ?70 90 ?00 ?00 ?80 .*
 0+.0 00 ?01 ?00 ?70 00 ?ff ?ff ?7f f4 ?02 ?00 ?70 f4 ?00 ?00 ?80 .*
