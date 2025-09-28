#source: emit-relocs-524.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f2a0003d 	movk	x29, #0x1, lsl #16
			10000: R_AARCH64_TLSLD_MOVW_DTPREL_G1	v2
