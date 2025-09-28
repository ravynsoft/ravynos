#as: -march=z80-full+sli
#objdump: -d
#name: Z80 instruction SLI/SLL

.*:.*

Disassembly of section .text:

0+ <.text>:
\s+0:\s+cb 37\s+sli a
\s+2:\s+cb 30\s+sli b
\s+4:\s+cb 31\s+sli c
\s+6:\s+cb 32\s+sli d
\s+8:\s+cb 33\s+sli e
\s+a:\s+cb 34\s+sli h
\s+c:\s+cb 35\s+sli l
\s+e:\s+cb 36\s+sli \(hl\)
\s+10:\s+dd cb 07 36\s+sli \(ix\+7\)
\s+14:\s+fd cb f7 36\s+sli \(iy\-9\)
\s+18:\s+cb 37\s+sli a
\s+1a:\s+cb 30\s+sli b
\s+1c:\s+cb 31\s+sli c
\s+1e:\s+cb 32\s+sli d
\s+20:\s+cb 33\s+sli e
\s+22:\s+cb 34\s+sli h
\s+24:\s+cb 35\s+sli l
\s+26:\s+cb 36\s+sli \(hl\)
\s+28:\s+dd cb 07 36\s+sli \(ix\+7\)
\s+2c:\s+fd cb f7 36\s+sli \(iy\-9\)
