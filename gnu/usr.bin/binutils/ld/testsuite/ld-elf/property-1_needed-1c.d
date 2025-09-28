#source: empty.s
#source: property-1_needed-1.s
#as:
#ld: -shared -z noindirect-extern-access
#readelf: -n
#target: x86_64-*-linux* i?86-*-linux-gnu i?86-*-gnu*

#...
Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: 1_needed: <unknown: 2>
#pass
