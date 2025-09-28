#ld: -shared -Map tmpdir/ifunc-1-local-x86.map --hash-style=sysv
#objdump: -dw
#target: x86_64-*-* i?86-*-*
#map: ifunc-1-local-x86.map

#...
[ \t0-9a-f]+:[ \t0-9a-f]+call[ \t0-9a-fq]+<\*ABS\*(\+0x[0-9a-f]+|)@plt>
#pass
