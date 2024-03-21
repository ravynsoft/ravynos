#as:
#objdump: -dw
#name: x86_64 MSRLIST insns
#source: x86-64-msrlist.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*f2 0f 01 c6\s+rdmsrlist
\s*[a-f0-9]+:\s*f3 0f 01 c6\s+wrmsrlist
\s*[a-f0-9]+:\s*f2 0f 01 c6\s+rdmsrlist
\s*[a-f0-9]+:\s*f3 0f 01 c6\s+wrmsrlist
#pass
