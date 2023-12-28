#source: property-x86-isa1.s
#source: simple.s
#as: --64 -defsym __64_bit__=1 -mx86-used-note=no
#ld: -m elf_x86_64 -z x86-64-v4
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: x86-64-baseline, x86-64-v4
	<processor-specific type 0xc0009000 data: 30 00 00 00 >
