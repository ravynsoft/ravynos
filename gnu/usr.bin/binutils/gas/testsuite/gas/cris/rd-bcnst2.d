#objdump: -dr
#as: --underscore --em=criself

# Consistency check, corresponding to rd-bcnst2-pic.d.

.*:     file format .*-cris

Disassembly of section \.text:

0+ <\.text>:
[ 	]+0:[ 	]+08e0[ 	]+ba 0xa
[ 	]+2:[ 	]+0f05[ 	]+nop 
[ 	]+4:[ 	]+3f0d 0000 0000[ 	]+jump 0x0
[ 	]+6:[ 	]+R_CRIS_32[ 	]+x0x42
[ 	]+a:[ 	]+f970[ 	]+bmi 0x4
[ 	]+c:[ 	]+0f05[ 	]+nop 
[ 	]+\.\.\.
