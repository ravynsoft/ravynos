#source: size64-2.s
#as: --x32
#ld: -shared -melf32_x86_64
#objdump: -R -s -j .data
#target: x86_64-*-*

.*: +file format .*

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
[[:xdigit:]]+ R_X86_64_SIZE32   xxx
[[:xdigit:]]+ R_X86_64_SIZE32   yyy
[[:xdigit:]]+ R_X86_64_SIZE32   zzz
[[:xdigit:]]+ R_X86_64_SIZE64   zzz-0x0000001e
[[:xdigit:]]+ R_X86_64_SIZE64   zzz\+0x0000001e


Contents of section .data:
 [[:xdigit:]]+ 00000000 00000000 00000000 00000000  ................
 [[:xdigit:]]+ 00000000 00000000 00000000 00000000  ................
 [[:xdigit:]]+ 00000000 00000000                    ........        
