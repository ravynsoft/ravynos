#objdump: -dr
#as: --defsym DIRECTIVE=1
#source: uao.s

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   [0-9a-f]:+	d500417f 	msr	uao, #0x1
   [0-9a-f]:+	d500407f 	msr	uao, #0x0
   [0-9a-f]:+	d5184280 	msr	uao, x0
   [0-9a-f]:+	d5384281 	mrs	x1, uao
