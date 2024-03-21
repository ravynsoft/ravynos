#as: -mintel64
#objdump: -dw -Mamd64
#name: x86-64 sysenter (Intel64/AMD64)

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:[ 	]+0f 34[ 	]+\(bad\)
[ 	]*[a-f0-9]+:[ 	]+0f 35[ 	]+\(bad\)
[ 	]*[a-f0-9]+:[ 	]+48 0f 35[ 	]+\(bad\)
[ 	]*[a-f0-9]+:[ 	]+0f 34[ 	]+\(bad\)
[ 	]*[a-f0-9]+:[ 	]+0f 35[ 	]+\(bad\)
[ 	]*[a-f0-9]+:[ 	]+48 0f 35[ 	]+\(bad\)
#pass
