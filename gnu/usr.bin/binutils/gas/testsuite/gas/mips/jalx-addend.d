#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JAL/JALX addend encoding
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> f600 0001 	jal	04000002 <bar\+0x3ffefe2>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> f200 0001 	jalx	08000004 <bar\+0x7ffefe4>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 76000001 	jalx	08000004 <bar\+0x7ffefe4>
[ 	]*[0-9a-f]+: R_MIPS_26	foo
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0e000001 	jal	08000004 <bar\+0x7ffefe4>
[ 	]*[0-9a-f]+: R_MIPS_26	bar
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
