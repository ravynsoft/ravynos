#name: Absolute non-overflowing relocs
#source: abs.s
#source: zero.s
#as: --32 -march=iamcu
#ld: -m elf_iamcu -z noseparate-code
#objdump: -rs -j .text

.*:     file format .*

Contents of section \.text:
[ 	][0-9a-f]+ c800fff0 c8000110 c9c3.*
