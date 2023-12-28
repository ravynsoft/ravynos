#as: -march=rv64i_zknh
#source: zknh-64.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+10251513[ 	]+sha256sig0[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10351513[ 	]+sha256sig1[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10051513[ 	]+sha256sum0[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10151513[ 	]+sha256sum1[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10651513[ 	]+sha512sig0[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10751513[ 	]+sha512sig1[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10451513[ 	]+sha512sum0[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10551513[ 	]+sha512sum1[ 	]+a0,a0
