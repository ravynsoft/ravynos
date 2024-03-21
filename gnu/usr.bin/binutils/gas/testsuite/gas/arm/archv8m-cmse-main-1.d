#name: ARMv8-M Mainline Security Extensions instructions (1)
#source: archv8m-cmse.s
#as: -march=armv8-m.main
#objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb

.*: +file format .*arm.*

Disassembly of section .text:
0+.* <[^>]*> e97f e97f 	sg
0+.* <[^>]*> 47a4      	blxns	r4
0+.* <[^>]*> 47cc      	blxns	r9
0+.* <[^>]*> 4724      	bxns	r4
0+.* <[^>]*> 474c      	bxns	r9
0+.* <[^>]*> e841 f080 	tta	r0, r1
0+.* <[^>]*> e849 f880 	tta	r8, r9
0+.* <[^>]*> e841 f0c0 	ttat	r0, r1
0+.* <[^>]*> e849 f8c0 	ttat	r8, r9
#...
