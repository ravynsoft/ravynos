#name: x86-64 rela relocs w/ non-zero relocated fields
#as: --64
#source: ${srcdir}/../../../gas/testsuite/gas/i386/rela.s
#ld: -melf_x86_64
#objdump: -sj .data

.*: +file format .*

Contents of section .data:
 *[0-9a-f]*0 .8 ?.. ?.. ?.. 00 ?00 ?00 ?00 .0 ?.. ?.. ?.. 44 ?44 ?44 ?44 .*
