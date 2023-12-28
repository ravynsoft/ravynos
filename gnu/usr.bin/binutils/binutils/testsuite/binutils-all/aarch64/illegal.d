#PROG: objcopy
#objdump: -d
#name: Check that the disassembler complains about illegal instructions.

.*: +file format .*aarch64.*

Disassembly of section \.text:

0+000 <.*>:
[ 	]+0:[ 	]+68ea18cc[ 	]+.inst[ 	]+0x68ea18cc ; undefined
#pass

