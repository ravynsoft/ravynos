# objdump: -wd
# name: Check that both encodings of mov.l (disp32) are accepted (PR 3134)

.*: *file format elf32-h8300.*

Disassembly of section \.text:

0+00 <\.text>:
 .*:[ 	]+01 00 78 80 6b a0 00 00 00 00[ 	]+mov.l[ 	]+er0,@\(0x0:32,er0\)
 .*:[ 	]+01 00 78 80 6b a0 00 00 00 00[ 	]+mov.l[ 	]+er0,@\(0x0:32,er0\)
 .*:[ 	]+01 00 78 00 6b a0 00 00 00 00[ 	]+mov.l[ 	]+er0,@\(0x0:32,er0\)
