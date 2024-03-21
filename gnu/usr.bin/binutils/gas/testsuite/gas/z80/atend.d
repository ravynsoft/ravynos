#objdump: -d
#name: index instructions with label as offset

.*: .*

Disassembly of section .text:

00000000 <begin>:
[       ]+0:[ 	]+3e 20[ 	]+ld a,0x20
[       ]+2:[ 	]+dd 7e 20[ 	]+ld a,\(ix\+32\)
[       ]+5:[ 	]+3e 40[ 	]+ld a,0x40
[       ]+7:[ 	]+dd 7e 40[ 	]+ld a,\(ix\+64\)
[       ]+a:[ 	]+c9[ 	]+ret
