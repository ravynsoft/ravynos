#source: emit-relocs-528.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	91002934 	add	x20, x9, #0xa
			10000: R_AARCH64_TLSLD_ADD_DTPREL_HI12	v2
