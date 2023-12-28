#objdump: -dtw
#name: SVR4 comment char escape handling

.*: +file format .*

SYMBOL TABLE:
0+00 .* \.text[ 	]+0+ svr4
0+04 .* \*ABS\*[ 	]+0+ a
0+03 .* \*ABS\*[ 	]+0+ b
0+4c .* \*ABS\*[ 	]+0+ c

Disassembly of section .text:

0+0 <svr4>:
[	 ]*[0-9a-f]+:[	 ]+b0 07[	 ]+mov    \$0x7,%al
[	 ]*[0-9a-f]+:[	 ]+b0 01[	 ]+mov    \$0x1,%al
[	 ]*[0-9a-f]+:[	 ]+b0 1e[	 ]+mov    \$0x1e,%al
[	 ]*[0-9a-f]+:[	 ]+b0 05[	 ]+mov    \$0x5,%al
[	 ]*[0-9a-f]+:[	 ]+b0 02[	 ]+mov    \$0x2,%al
[	 ]*[0-9a-f]+:[	 ]+b0 33[	 ]+mov    \$0x33,%al
#pass
