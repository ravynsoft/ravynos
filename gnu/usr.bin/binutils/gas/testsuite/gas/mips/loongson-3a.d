#as: -march=loongson3a -mabi=o64
#objdump: -M reg-names=numeric -dr
#name: Loongson-3A tests

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <movz_insns>:
.*:	0064100b 	movn	\$2,\$3,\$4

[0-9a-f]+ <integer_insns>:
.*:	70641010 	gsmult	\$2,\$3,\$4
.*:	70c72812 	gsmultu	\$5,\$6,\$7
.*:	712a4011 	gsdmult	\$8,\$9,\$10
.*:	718d5813 	gsdmultu	\$11,\$12,\$13
.*:	71f07014 	gsdiv	\$14,\$15,\$16
.*:	72538816 	gsdivu	\$17,\$18,\$19
.*:	72b6a015 	gsddiv	\$20,\$21,\$22
.*:	7319b817 	gsddivu	\$23,\$24,\$25
.*:	737cd01c 	gsmod	\$26,\$27,\$28
.*:	73dfe81e 	gsmodu	\$29,\$30,\$31
.*:	7064101d 	gsdmod	\$2,\$3,\$4
.*:	70c7281f 	gsdmodu	\$5,\$6,\$7
#pass
