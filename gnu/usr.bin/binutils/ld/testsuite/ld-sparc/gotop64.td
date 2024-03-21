#source: gotop64.s
#as: --64 -K PIC
#ld: -shared -melf64_sparc
#objdump: -sj.data
#target: sparc*-*-*

.*: +file format elf64-sparc.*

Contents of section .data:
 103000 12345678 deadbeef                    .*
#pass
