#objdump: -dr
#as: --pic --underscore --em=criself
#source: rd-bcnst2.s

# Catches an error in the relaxation machinery.

.*:     file format elf32.*-cris

Disassembly of section \.text:

0+ <\.text>:
[ 	]+0:[ 	]+0ae0[ 	]+ba 0xc
[ 	]+2:[ 	]+0f05[ 	]+nop 
[ 	]+4:[ 	]+6ffd 0000 0000 3f0e[ 	]+move \[pc=pc\+0x0\],p0
[ 	]+6:[ 	]+R_CRIS_32_PCREL[ 	]+x0x42
[ 	]+c:[ 	]+f770[ 	]+bmi 0x4
[ 	]+e:[ 	]+0f05[ 	]+nop 
