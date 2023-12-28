#as: --x32
#ld: -m elf32_x86_64 -Ttext-segment 0xe0000000 -Ttext 0xe0010000
#objdump: -s -j .text

.*: +file format .*

Contents of section .text:
 e0010000 000001e0 00000000                    ........        
#pass
