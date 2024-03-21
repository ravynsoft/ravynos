#as: --size-check=warning
#objdump: -dw
#name: Check bad size directive
#warning_output: bad-size.warn

.*: +file format .*


Disassembly of section .text:

0+ <_test_nop>:
[ 	]*[a-f0-9]+:	90                   	nop

Disassembly of section .text.entry.continue:

0+ <.text.entry.continue>:
[ 	]*[a-f0-9]+:	90                   	nop
#pass
