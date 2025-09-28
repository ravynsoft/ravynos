#PROG: objcopy
#objcopy: --remove-section .text.foo
#name: copy removing all group member
#objdump: -fw

#...
.*: +file format .*
architecture: .*
#pass
