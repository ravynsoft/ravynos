#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: ulw2 -EL interlocked
#source: ulw2.s
#as: -EL -32

# Further checks of ulw macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6085 0003 	lwl	\$4,3\(\$5\)
[0-9a-f]+ <[^>]*> 6085 1000 	lwr	\$4,0\(\$5\)
[0-9a-f]+ <[^>]*> 6085 0004 	lwl	\$4,4\(\$5\)
[0-9a-f]+ <[^>]*> 6085 1001 	lwr	\$4,1\(\$5\)
[0-9a-f]+ <[^>]*> 6025 0003 	lwl	\$1,3\(\$5\)
[0-9a-f]+ <[^>]*> 6025 1000 	lwr	\$1,0\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
[0-9a-f]+ <[^>]*> 6025 0004 	lwl	\$1,4\(\$5\)
[0-9a-f]+ <[^>]*> 6025 1001 	lwr	\$1,1\(\$5\)
[0-9a-f]+ <[^>]*> 0ca1      	move	\$5,\$1
	\.\.\.
