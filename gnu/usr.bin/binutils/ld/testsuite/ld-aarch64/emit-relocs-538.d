#source: emit-relocs-538.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f9400482 	ldr	x2, \[x4, #8\]
			10000: R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC	v2
   10004:	f940062e 	ldr	x14, \[x17, #8\]
			10004: R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC	v3
