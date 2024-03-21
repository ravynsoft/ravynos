#objdump: -dr
#name: i386 memory operands w/ unary operators

.*: +file format .*

Disassembly of section .text:

0+ <unary>:
[ 	]*[a-f0-9]+:[ 	]*8b 40 01[	 ]+mov    0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 40 ff[	 ]+mov    -0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 00[	 ]+mov    \(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 40 fe[	 ]+mov    -0x2\(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 40 01[	 ]+mov    0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 40 ff[	 ]+mov    -0x1\(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 00[	 ]+mov    \(%eax\),%eax
[ 	]*[a-f0-9]+:[ 	]*8b 40 fe[	 ]+mov    -0x2\(%eax\),%eax
#pass
