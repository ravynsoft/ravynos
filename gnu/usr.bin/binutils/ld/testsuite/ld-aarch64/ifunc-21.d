#source: ifunc-21.s
#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv -z nocombreloc
#objdump: -d -s -j .got.plt -j .text

# Ensure the .got.plt slot used is correct

.*:     file format elf64-(little|big)aarch64

Contents of section .text:
 [0-9a-f]+ .*
Contents of section .got.plt:
 [0-9a-f]+ 0+ 0+ 0+ 0+  .*
 (10298|102b8) 0+ 0+ [0-9a-f]+ [0-9a-f]+  .*

Disassembly of section .text:

.* <ifunc>:
 .*:	d65f03c0 	ret

.* <bar>:
 .*:	90000080 	adrp	x0, 10000 <.*>
 .*:	.* 	ldr	x0, \[x0, #(672|704)\]
 .*:	d65f03c0 	ret

#pass
