#as: -march=z180
#objdump: -d
#name: Z180 specific instructions

.*: .*

Disassembly of section .text:

0+ <.text>:
\s+0:\s+ed 38 05\s+in0 a,\(0x05\)
\s+3:\s+ed 00 05\s+in0 b,\(0x05\)
\s+6:\s+ed 08 05\s+in0 c,\(0x05\)
\s+9:\s+ed 10 05\s+in0 d,\(0x05\)
\s+c:\s+ed 18 05\s+in0 e,\(0x05\)
\s+f:\s+ed 20 05\s+in0 h,\(0x05\)
\s+12:\s+ed 28 05\s+in0 l,\(0x05\)
\s+15:\s+ed 39 05\s+out0 \(0x05\),a
\s+18:\s+ed 01 05\s+out0 \(0x05\),b
\s+1b:\s+ed 09 05\s+out0 \(0x05\),c
\s+1e:\s+ed 11 05\s+out0 \(0x05\),d
\s+21:\s+ed 19 05\s+out0 \(0x05\),e
\s+24:\s+ed 21 05\s+out0 \(0x05\),h
\s+27:\s+ed 29 05\s+out0 \(0x05\),l
\s+2a:\s+ed 4c\s+mlt bc
\s+2c:\s+ed 5c\s+mlt de
\s+2e:\s+ed 6c\s+mlt hl
\s+30:\s+ed 7c\s+mlt sp
\s+32:\s+ed 3c\s+tst a
\s+34:\s+ed 04\s+tst b
\s+36:\s+ed 0c\s+tst c
\s+38:\s+ed 14\s+tst d
\s+3a:\s+ed 1c\s+tst e
\s+3c:\s+ed 24\s+tst h
\s+3e:\s+ed 2c\s+tst l
\s+40:\s+ed 34\s+tst \(hl\)
\s+42:\s+ed 64 0f\s+tst 0x0f
\s+45:\s+ed 74 f0\s+tstio 0xf0
\s+48:\s+ed 76\s+slp
\s+4a:\s+ed 83\s+otim
\s+4c:\s+ed 8b\s+otdm
\s+4e:\s+ed 93\s+otimr
\s+50:\s+ed 9b\s+otdmr
