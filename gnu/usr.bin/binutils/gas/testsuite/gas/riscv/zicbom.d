#as: -march=rv64g_zicbom
#source: zicbom.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+0010a00f[ 	]+cbo\.clean[ 	]+\(ra\)
[ 	]+[0-9a-f]+:[ 	]+001f200f[ 	]+cbo\.clean[ 	]+\(t5\)
[ 	]+[0-9a-f]+:[ 	]+0020a00f[ 	]+cbo\.flush[ 	]+\(ra\)
[ 	]+[0-9a-f]+:[ 	]+002f200f[ 	]+cbo\.flush[ 	]+\(t5\)
[ 	]+[0-9a-f]+:[ 	]+0000a00f[ 	]+cbo\.inval[ 	]+\(ra\)
[ 	]+[0-9a-f]+:[ 	]+000f200f[ 	]+cbo\.inval[ 	]+\(t5\)
