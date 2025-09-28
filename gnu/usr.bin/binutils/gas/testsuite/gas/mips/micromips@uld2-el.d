#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: uld2 -EL
#source: uld2.s
#as: -EL

# Further checks of uld macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6085 4007 	ldl	\$4,7\(\$5\)
[0-9a-f]+ <[^>]*> 6085 5000 	ldr	\$4,0\(\$5\)
[0-9a-f]+ <[^>]*> 6085 4008 	ldl	\$4,8\(\$5\)
[0-9a-f]+ <[^>]*> 6085 5001 	ldr	\$4,1\(\$5\)
[0-9a-f]+ <[^>]*> 6025 4007 	ldl	\$1,7\(\$5\)
[0-9a-f]+ <[^>]*> 6025 5000 	ldr	\$1,0\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
[0-9a-f]+ <[^>]*> 6025 4008 	ldl	\$1,8\(\$5\)
[0-9a-f]+ <[^>]*> 6025 5001 	ldr	\$1,1\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
	\.\.\.
