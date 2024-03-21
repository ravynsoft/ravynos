#source: emit-relocs-527.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f2800091 	movk	x17, #0x4
			10000: R_AARCH64_TLSLD_MOVW_DTPREL_G0_NC	v2
