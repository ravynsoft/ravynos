#as:
#objdump: -dw -Mintel
#name: x86_64 FRED insns (Intel disassembly)
#source: x86-64-fred.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*f2 0f 01 ca\s+erets
\s*[a-f0-9]+:\s*f3 0f 01 ca\s+eretu
\s*[a-f0-9]+:\s*f2 0f 01 ca\s+erets
\s*[a-f0-9]+:\s*f3 0f 01 ca\s+eretu
#pass
