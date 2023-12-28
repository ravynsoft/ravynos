#name: PR ld/22269
#source: pr22269.s
#ld: -pie --no-dynamic-linker
#readelf: -r -x .data.rel.ro
#target: *-*-linux* *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
# The BFIN target always generates a relocation.
#xfail: ![check_pie_support] || bfin-*-*

There are no relocations in this file.

Hex dump of section '.data.rel.ro':
  0x[a-f0-9]+ [0 ]+[ ]+.+
