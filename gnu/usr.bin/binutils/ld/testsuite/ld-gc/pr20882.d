#name: --gc-sections with relocations in debug section
#source: pr20882a.s
#source: pr20882b.s
#source: pr20882c.s
#ld: --gc-sections -e main
#readelf: -x .debug_info -x .debug_abbrev

Hex dump of section '\.debug_info':
  0x0+ .*

Hex dump of section '\.debug_abbrev':
  0x0+ 61626364 +abcd
