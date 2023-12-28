#source: size32-1.s
#as: --32
#ld: -shared -melf_i386
#objdump: -R -s -j .data
#target: x86_64-*-* i?86-*-*

.*: +file format .*

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
[[:xdigit:]]+ R_386_SIZE32      xxx
[[:xdigit:]]+ R_386_SIZE32      xxx
[[:xdigit:]]+ R_386_SIZE32      xxx
[[:xdigit:]]+ R_386_SIZE32      yyy
[[:xdigit:]]+ R_386_SIZE32      zzz


Contents of section .data:
 [[:xdigit:]]+ 00000000 e2ffffff 1e000000 00000000  ................
 [[:xdigit:]]+ 00000000 00000000 00000000 00000000  ................
 [[:xdigit:]]+ 00000000 00000000 00000000 00000000  ................
 [[:xdigit:]]+ 00000000 00000000 00000000           ............    
