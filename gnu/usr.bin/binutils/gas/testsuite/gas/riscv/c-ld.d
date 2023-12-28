#as: -march=rv64ic
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+0:[ 	]+6108[ 	]+ld[ 	]+a0,0\(a0\)
[ 	]+2:[ 	]+6108[ 	]+ld[ 	]+a0,0\(a0\)
[ 	]+4:[ 	]+e108[ 	]+sd[ 	]+a0,0\(a0\)
[ 	]+6:[ 	]+e108[ 	]+sd[ 	]+a0,0\(a0\)
[ 	]+8:[ 	]+6502[ 	]+ld[ 	]+a0,0\(sp\)
[ 	]+a:[ 	]+6502[ 	]+ld[ 	]+a0,0\(sp\)
[ 	]+c:[ 	]+e02a[ 	]+sd[ 	]+a0,0\(sp\)
[ 	]+e:[ 	]+e02a[ 	]+sd[ 	]+a0,0\(sp\)
