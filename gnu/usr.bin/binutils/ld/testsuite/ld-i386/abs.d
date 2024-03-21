#name: Absolute non-overflowing relocs
#as: --32
#source: abs.s
#source: zero.s
#ld: -melf_i386 -z noseparate-code
#objdump: -rs

.*:     file format .*

Contents of section \.text:
[ 	][0-9a-f]+ c800fff0 c8000110 c9c3.*
