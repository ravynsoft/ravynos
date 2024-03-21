#source: emit-relocs-277.s
#ld: -T relocs.ld --defsym tempy=0x11018 --defsym tempy2=0x45038 --defsym tempy3=0x1234  -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	91006064 	add	x4, x3,.*
	+10008: R_AARCH64_ADD_ABS_LO12_NC	tempy
 +1000c:	9100e067 	add	x7, x3,.*
	+1000c: R_AARCH64_ADD_ABS_LO12_NC	tempy2
 +10010:	9108d071 	add	x17, x3,.*
	+10010: R_AARCH64_ADD_ABS_LO12_NC	tempy3

