#source: emit-relocs-537.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f9400520 	ldr	x0, \[x9, #8\]
			10000: R_AARCH64_TLSLD_LDST64_DTPREL_LO12	v2
