#name: .noinit sections
#ld: --orphan-handling=warn -e _start
#source: noinit-sections.s
#target: [supports_noinit_section]
#readelf: -SW

#...
 +\[ *[0-9]+\] \.noinit +NOBITS +[0-9a-f]+ +[0-9a-f]+ [0-9a-f]+ +00 +WA .*
#pass
