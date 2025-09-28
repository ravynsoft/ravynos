#as:
#objdump: -dw
#name: i386 WRMSRNS insns
#source: wrmsrns.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*0f 01 c6\s+wrmsrns
\s*[a-f0-9]+:\s*0f 01 c6\s+wrmsrns
#pass
