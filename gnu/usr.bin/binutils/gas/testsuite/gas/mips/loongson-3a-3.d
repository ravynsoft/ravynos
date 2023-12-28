#as: -march=loongson3a -mabi=o64
#objdump: -M reg-names=numeric -dr
#name: Loongson delay slot tests

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <.text>:
.*:	c8c50024 	gslq	\$4,\$5,0\(\$6\)
.*:	10800002 	beqz	\$4,0x10
.*:	00000000 	nop
#pass
