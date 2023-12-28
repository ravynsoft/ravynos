#source: size-2.s
#ld: -T size-2.t
#readelf: -lS --wide
# v850 .tdata/.tbss are not TLS sections
#xfail: v850*-*-*
# The three alternatives for PHDR and LOAD are: 64-bit, 32-bit, 32-bit
# not demand paged.  32-bit LOAD has a variant for spu-elf, which rounds
# load size up to multiples of 16 bytes.

#...
.* \.text +PROGBITS +0+100 [0-9a-f]+ 0+10 00  AX .*
.* \.tdata +PROGBITS +0+110 [0-9a-f]+ 0+20 00 WAT .*
.* \.tbss +NOBITS +0+130 [0-9a-f]+ 0+30 00 WAT .*
.* \.map +PROGBITS +0+130 [0-9a-f]+ 0+c 00 +A .*
#...
 +PHDR +(0x0+40 0x0+40 0x0+40 0x0+a8 0x0+a8|0x0+34 0x0+34 0x0+34 0x0+60 0x0+60|0x0+34 0x0+a0 0x0+a0 0x0+60 0x0+60) R .*
 +LOAD +(0x0+40 0x0+40 0x0+40 0x0+fc 0x0+fc|0x0+34 0x0+34 0x0+34 0x0+1(08|10) 0x0+1(08|10)|0x0+34 0x0+a0 0x0+a0 0x0+9c 0x0+9c) R E .*
 +TLS +0x0+(110|a4) 0x0+110 0x0+110 0x0+20 0x0+50 R .*
#...
.* \.text \.tdata \.map 
.* \.tdata \.tbss 
