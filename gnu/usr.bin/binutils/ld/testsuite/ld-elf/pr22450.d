#source: pr22450.s
#readelf: --notes --wide
#ld: -r
#xfail: avr-*-* crx-*-* h8300-*-* ip2k-*-* m68hc11-*-* z80-*-*
# Fails on H8300 because it does not generate the correct relocs for the size fields.
# Fails on AVR, IP2K, and M68HC11 because the assembler does not calculate the correct values for the differences of local symbols.
# Fails on CRX because readelf does not know how to apply CRX reloc number 20 (R_CRX_SWITCH32).

#...
Displaying notes found in: \.note\.gnu
[ 	]+Owner[ 	]+Data size[ 	]+Description
[ 	]+GNU[ 	]+0x0+0..[ 	]+NT_GNU_PROPERTY_TYPE_0[ 	]+Properties: stack size: 0x8000
#pass
