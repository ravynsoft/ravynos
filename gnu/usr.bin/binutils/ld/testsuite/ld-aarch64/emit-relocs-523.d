#source: emit-relocs-523.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f2c0001d 	movk	x29, #0x0, lsl #32
			10000: R_AARCH64_TLSLD_MOVW_DTPREL_G2	v2
