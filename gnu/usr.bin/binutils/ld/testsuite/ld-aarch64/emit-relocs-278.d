#source: emit-relocs-278.s
#ld: -T relocs.ld --defsym tempy=0x11018 --defsym tempy2=0x45038 --defsym tempy3=0x1234  -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	3d406064 	ldr	b4, \[x3,.*
	+10008: R_AARCH64_LDST8_ABS_LO12_NC	tempy
 +1000c:	3d40e067 	ldr	b7, \[x3,.*
	+1000c: R_AARCH64_LDST8_ABS_LO12_NC	tempy2
 +10010:	3d48d071 	ldr	b17, \[x3,.*
	+10010: R_AARCH64_LDST8_ABS_LO12_NC	tempy3

