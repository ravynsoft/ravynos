#source: weakdef-1.s
#target: i*86-*-cygwin* i*86-*-pe i*86-*-mingw*
#ld: -e _start --gc-sections
#objdump: -d

#...
  401003:	a1 00 20 40 00       	mov    0x402000,%eax
#pass
