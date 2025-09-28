#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS mips5 instructions
#source: mips5-fp.s
#warning_output: mips5-fp.l

# Check MIPS V instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 5402 437b 	abs\.ps	\$f0,\$f2
[0-9a-f]+ <[^>]*> 54c4 1230 	add\.ps	\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 5548 30d9 	alnv\.ps	\$f6,\$f8,\$f10,\$3
[0-9a-f]+ <[^>]*> 5548 08bc 	c\.eq\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 48bc 	c\.eq\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 083c 	c\.f\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 483c 	c\.f\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0bbc 	c\.le\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4bbc 	c\.le\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0b3c 	c\.lt\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4b3c 	c\.lt\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0b7c 	c\.nge\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4b7c 	c\.nge\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0afc 	c\.ngl\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4afc 	c\.ngl\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0a7c 	c\.ngle\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4a7c 	c\.ngle\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0bfc 	c\.ngt\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4bfc 	c\.ngt\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 09bc 	c\.ole\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 49bc 	c\.ole\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 093c 	c\.olt\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 493c 	c\.olt\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0abc 	c\.seq\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4abc 	c\.seq\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 0a3c 	c\.sf\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 4a3c 	c\.sf\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 08fc 	c\.ueq\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 48fc 	c\.ueq\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 09fc 	c\.ule\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 49fc 	c\.ule\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 097c 	c\.ult\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 497c 	c\.ult\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 5548 087c 	c\.un\.ps	\$f8,\$f10
[0-9a-f]+ <[^>]*> 558a 487c 	c\.un\.ps	\$fcc2,\$f10,\$f12
[0-9a-f]+ <[^>]*> 560e 6180 	cvt\.ps\.s	\$f12,\$f14,\$f16
[0-9a-f]+ <[^>]*> 5612 213b 	cvt\.s\.pl	\$f16,\$f18
[0-9a-f]+ <[^>]*> 5654 293b 	cvt\.s\.pu	\$f18,\$f20
[0-9a-f]+ <[^>]*> 5485 a148 	luxc1	\$f20,\$4\(\$5\)
[0-9a-f]+ <[^>]*> 5758 a591 	madd\.ps	\$f20,\$f22,\$f24,\$f26
[0-9a-f]+ <[^>]*> 571a 407b 	mov\.ps	\$f24,\$f26
[0-9a-f]+ <[^>]*> 575c 4420 	movf\.ps	\$f26,\$f28,\$fcc2
[0-9a-f]+ <[^>]*> 547c d238 	movn\.ps	\$f26,\$f28,\$3
[0-9a-f]+ <[^>]*> 579e 8460 	movt\.ps	\$f28,\$f30,\$fcc4
[0-9a-f]+ <[^>]*> 54be e278 	movz\.ps	\$f28,\$f30,\$5
[0-9a-f]+ <[^>]*> 5482 f031 	msub\.ps	\$f30,\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 54c4 12b0 	mul\.ps	\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> 54c8 4b7b 	neg\.ps	\$f6,\$f8
[0-9a-f]+ <[^>]*> 558a 3212 	nmadd\.ps	\$f6,\$f8,\$f10,\$f12
[0-9a-f]+ <[^>]*> 558a 3232 	nmsub\.ps	\$f6,\$f8,\$f10,\$f12
[0-9a-f]+ <[^>]*> 55cc 5080 	pll\.ps	\$f10,\$f12,\$f14
[0-9a-f]+ <[^>]*> 5650 70c0 	plu\.ps	\$f14,\$f16,\$f18
[0-9a-f]+ <[^>]*> 5692 8100 	pul\.ps	\$f16,\$f18,\$f20
[0-9a-f]+ <[^>]*> 5716 a140 	puu\.ps	\$f20,\$f22,\$f24
[0-9a-f]+ <[^>]*> 5758 b270 	sub\.ps	\$f22,\$f24,\$f26
[0-9a-f]+ <[^>]*> 54c7 d188 	suxc1	\$f26,\$6\(\$7\)
[0-9a-f]+ <[^>]*> 558a 68bc 	c\.eq\.ps	\$fcc3,\$f10,\$f12
[0-9a-f]+ <[^>]*> 575c 6420 	movf\.ps	\$f26,\$f28,\$fcc3
	\.\.\.
