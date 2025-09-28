#as: -march=rv32i_zknh
#source: zknh-32.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+10251513[ 	]+sha256sig0[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10351513[ 	]+sha256sig1[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10051513[ 	]+sha256sum0[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+10151513[ 	]+sha256sum1[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+5cc58533[ 	]+sha512sig0h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+54c58533[ 	]+sha512sig0l[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+5ec58533[ 	]+sha512sig1h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+56c58533[ 	]+sha512sig1l[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+50c58533[ 	]+sha512sum0r[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+52c58533[ 	]+sha512sum1r[ 	]+a0,a1,a2
