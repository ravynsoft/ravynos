#source: gc-start.s
#source: gc-relocs-257.s
#target: [check_shared_lib_support]
#ld: --defsym tempy=0x11012 --defsym tempy2=0x45034 --defsym tempy3=0x1234   -T aarch64.ld   -shared  --gc-sections
#objdump: -R -d

# This tests if the linker is able to remove dynamic relocs created
# for R_AARCH64_ABS64 while removing a gc section.  The section is
# also removed.  So after gc, we should be left with the startup code.

.*:     file format elf64-(little|big)aarch64


Disassembly of section .text:

0+8000 \<_start\>:
    8000:	d503201f 	nop
