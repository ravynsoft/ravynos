#name: Valid Armv8.1-M Mainline <spec_reg> change
#source: armv8_1-m-spec-reg.s
#as: -march=armv8.1-m.main+mve
#objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> eef1 0a10 	vmrs	r0, fpscr
0[0-9a-f]+ <[^>]+> eef2 1a10 	vmrs	r1, fpscr_nzcvqc
0[0-9a-f]+ <[^>]+> eefc 2a10 	vmrs	r2, vpr
0[0-9a-f]+ <[^>]+> eefd 3a10 	vmrs	r3, p0
0[0-9a-f]+ <[^>]+> eefe 4a10 	vmrs	r4, fpcxt_ns
0[0-9a-f]+ <[^>]+> eeff 5a10 	vmrs	r5, fpcxt_s
0[0-9a-f]+ <[^>]+> eee1 0a10 	vmsr	fpscr, r0
0[0-9a-f]+ <[^>]+> eee2 1a10 	vmsr	fpscr_nzcvqc, r1
0[0-9a-f]+ <[^>]+> eeec 2a10 	vmsr	vpr, r2
0[0-9a-f]+ <[^>]+> eeed 3a10 	vmsr	p0, r3
0[0-9a-f]+ <[^>]+> eeee 4a10 	vmsr	fpcxt_ns, r4
0[0-9a-f]+ <[^>]+> eeef 5a10 	vmsr	fpcxt_s, r5
