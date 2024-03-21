#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: uld2 -EB
#source: uld2.s
#as: -EB

# Further checks of uld macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6085 4000 	ldl	\$4,0\(\$5\)
[0-9a-f]+ <[^>]*> 6085 5007 	ldr	\$4,7\(\$5\)
[0-9a-f]+ <[^>]*> 6085 4001 	ldl	\$4,1\(\$5\)
[0-9a-f]+ <[^>]*> 6085 5008 	ldr	\$4,8\(\$5\)
[0-9a-f]+ <[^>]*> 6025 4000 	ldl	\$1,0\(\$5\)
[0-9a-f]+ <[^>]*> 6025 5007 	ldr	\$1,7\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
[0-9a-f]+ <[^>]*> 6025 4001 	ldl	\$1,1\(\$5\)
[0-9a-f]+ <[^>]*> 6025 5008 	ldr	\$1,8\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
	\.\.\.
