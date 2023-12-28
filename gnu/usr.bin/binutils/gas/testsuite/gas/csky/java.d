# name: csky - java
#as: -mcpu=ck802j
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]:\s*14ac\s*bpop\.h\s*r3
\s*[0-9a-f]:\s*14ae\s*bpop\.w\s*r3
\s*[0-9a-f]:\s*14ec\s*bpush\.h\s*r3
\s*[0-9a-f]:\s*14ee\s*bpush\.w\s*r3
#...
