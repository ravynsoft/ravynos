#source: gc-start.s
#source: gc-relocs-257.s
#ld: --defsym tempy=0x11012 --defsym tempy2=0x45034 --defsym tempy3=0x1234   -T aarch64.ld  --gc-sections
#objdump:  -d

# This tests if linker is able to remove gc section containing
# R_AARCH64_ABS64 relocs.  So after gc, we should be left with
# only the startup code.

.*:     file format elf64-(little|big)aarch64


Disassembly of section .text:

0+8000 \<_start\>:
    8000:	d503201f 	nop
