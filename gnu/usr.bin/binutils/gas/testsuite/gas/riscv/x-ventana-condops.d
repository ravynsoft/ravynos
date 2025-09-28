#as: -march=rv64i_xventanacondops1p0
#source: x-ventana-condops.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+0:[ 	]+00c5e57b[ 	]+vt.maskc[ 	]+a0,a1,a2
[ 	]+4:[ 	]+00e6f57b[ 	]+vt.maskcn[ 	]+a0,a3,a4
