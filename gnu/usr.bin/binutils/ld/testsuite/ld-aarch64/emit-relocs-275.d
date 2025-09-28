#source: emit-relocs-275.s
#ld: -T relocs.ld --defsym tempy=0x200011000 --defsym tempy2=0x45000 --defsym tempy3=0x1234 -e0 --emit-relocs
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_ADR_PREL_PG_HI21 against symbol `tempy'
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	b0000002 	adrp	x2, .*
	+10008: R_AARCH64_ADR_PREL_PG_HI21	tempy
 +1000c:	b00001a7 	adrp	x7, .*
	+1000c: R_AARCH64_ADR_PREL_PG_HI21	tempy2
 +10010:	b0ffff91 	adrp	x17, .*
	+10010: R_AARCH64_ADR_PREL_PG_HI21	tempy3


