#as:
#objdump: -dr
#skip: loongarch32-*-*

.*:[    ]+file format .*


Disassembly of section .text:

00000000.* <_start>:
[ 	]+0:[ 	]+03803c06[ 	]+li\.w[ 	]+\$a2,[ 	]+0xf
[ 	]+4:[ 	]+1a000005[ 	]+pcalau12i[ 	]+\$a1,[ 	]+0
[ 	]+4:[ 	]+R_LARCH_PCALA_HI20[ 	]+msg
[ 	]+4:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+8:[ 	]+02c000a5[ 	]+addi\.d[ 	]+\$a1,[ 	]+\$a1,[ 	]+0
[ 	]+8:[ 	]+R_LARCH_PCALA_LO12[ 	]+msg
[ 	]+8:[ 	]+R_LARCH_RELAX[ 	]+\*ABS\*
[ 	]+c:[ 	]+03800404[ 	]+li\.w[ 	]+\$a0,[ 	]+0x1
[ 	]+10:[ 	]+0381000b[ 	]+li\.w[ 	]+\$a7,[ 	]+0x40
[ 	]+14:[ 	]+002b0000[ 	]+syscall[ 	]+0x0
[ 	]+18:[ 	]+00150004[ 	]+move[ 	]+\$a0,[ 	]+\$zero
[ 	]+1c:[ 	]+0381740b[ 	]+li\.w[ 	]+\$a7,[ 	]+0x5d
[ 	]+20:[ 	]+002b0000[ 	]+syscall[ 	]+0x0
