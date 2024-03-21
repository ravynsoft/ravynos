#name: x86-64 property 1
#source: property-1.s
#as: -mx86-used-note=no -defsym __64_bit__=1 --generate-missing-build-notes=no
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA used: *
