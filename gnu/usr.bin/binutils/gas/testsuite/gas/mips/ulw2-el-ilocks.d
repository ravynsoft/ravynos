#as: -EL -32
#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: ulw2 -EL interlocked
#source: ulw2.s

# Further checks of ulw macro.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 88a40003 	lwl	\$4,3\(\$5\)
0+0004 <[^>]*> 98a40000 	lwr	\$4,0\(\$5\)
0+0008 <[^>]*> 88a40004 	lwl	\$4,4\(\$5\)
0+000c <[^>]*> 98a40001 	lwr	\$4,1\(\$5\)
0+0010 <[^>]*> 88a10003 	lwl	\$1,3\(\$5\)
0+0014 <[^>]*> 98a10000 	lwr	\$1,0\(\$5\)
0+0018 <[^>]*> 00202825 	move	\$5,\$1
0+001c <[^>]*> 88a10004 	lwl	\$1,4\(\$5\)
0+0020 <[^>]*> 98a10001 	lwr	\$1,1\(\$5\)
0+0024 <[^>]*> 00202825 	move	\$5,\$1
	\.\.\.
