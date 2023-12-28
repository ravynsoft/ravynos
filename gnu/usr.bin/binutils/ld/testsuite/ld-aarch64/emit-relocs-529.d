#source: emit-relocs-529.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	91001134 	add	x20, x9, #0x4
			10000: R_AARCH64_TLSLD_ADD_DTPREL_LO12	v2
