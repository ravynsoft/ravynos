#source: asciz.s
#ld: -T asciz.t
#objdump: -s -j .data
#target: [is_elf_format] [is_coff_format]
#notarget: tic4x-*-* tic54x-*-*

.*:     file format .*

Contents of section .data:
 .... 54686973 20697320 61207374 72696e67  This is a string
 .... 00546869 73206973 20616e6f 74686572  .This is another
 .... 0a537472 696e6700 006e6f71 756f7465  .String..noquote
 .... 7300[ 0]*  s.*
#pass
