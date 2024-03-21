#source: emit-relocs-311.s
#ld: -T relocs.ld --defsym tempy=0x11000 --defsym tempy2=0x45000 --defsym tempy3=0x1234 -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	b0000002 	adrp	x2, 11000 <tempy>
	+10008: R_AARCH64_ADR_PREL_PG_HI21	tempy
 +1000c:	b00001a7 	adrp	x7, 45000 <tempy2>
	+1000c: R_AARCH64_ADR_PREL_PG_HI21	tempy2
 +10010:	b0ffff91 	adrp	x17, 1000 <tempy3-0x234>
	+10010: R_AARCH64_ADR_PREL_PG_HI21	tempy3
 +10014:	90000083 	adrp	x3, 20000 <_GLOBAL_OFFSET_TABLE_>
	+10014: R_AARCH64_ADR_GOT_PAGE	gempy
