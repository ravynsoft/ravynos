#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS32 odd single-precision float registers
#source: mips32-sf32.s
#as: -32

# Check MIPS32 instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41a1 3f80 	lui	\$1,0x3f80
[0-9a-f]+ <[^>]*> 5421 283b 	mtc1	\$1,\$f1
[0-9a-f]+ <[^>]*> 9c7c 0000 	lwc1	\$f3,0\(\$28\)
[ 	]*[0-9a-f]+: R_MICROMIPS_LITERAL	\.lit4
[0-9a-f]+ <[^>]*> 5461 2830 	add\.s	\$f5,\$f1,\$f3
[0-9a-f]+ <[^>]*> 5507 137b 	cvt\.d\.s	\$f8,\$f7
[0-9a-f]+ <[^>]*> 5507 337b 	cvt\.d\.w	\$f8,\$f7
[0-9a-f]+ <[^>]*> 54e8 1b7b 	cvt\.s\.d	\$f7,\$f8
[0-9a-f]+ <[^>]*> 54e8 6b3b 	trunc\.w\.d	\$f7,\$f8
	\.\.\.
