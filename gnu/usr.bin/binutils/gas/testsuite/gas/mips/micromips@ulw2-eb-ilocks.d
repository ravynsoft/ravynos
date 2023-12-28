#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: ulw2 -EB interlocked
#source: ulw2.s
#as: -EB -32

# Further checks of ulw macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6085 0000 	lwl	\$4,0\(\$5\)
[0-9a-f]+ <[^>]*> 6085 1003 	lwr	\$4,3\(\$5\)
[0-9a-f]+ <[^>]*> 6085 0001 	lwl	\$4,1\(\$5\)
[0-9a-f]+ <[^>]*> 6085 1004 	lwr	\$4,4\(\$5\)
[0-9a-f]+ <[^>]*> 6025 0000 	lwl	\$1,0\(\$5\)
[0-9a-f]+ <[^>]*> 6025 1003 	lwr	\$1,3\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
[0-9a-f]+ <[^>]*> 6025 0001 	lwl	\$1,1\(\$5\)
[0-9a-f]+ <[^>]*> 6025 1004 	lwr	\$1,4\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
	\.\.\.
