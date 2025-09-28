#source: ifunc-22.s
#objdump: -s -j .got
#ld: -static
#target: aarch64*-*-*

# Ensure GOT is populated correctly in static link

.*:     file format elf64-(little|big)aarch64

Contents of section \.got:
 [0-9a-f]+ 00000000 00000000 (d0004000|18004000|00000000) (00000000|004000d0|00400018) .*
