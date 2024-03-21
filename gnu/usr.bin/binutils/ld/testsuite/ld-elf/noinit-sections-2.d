#name: .noinit sections (ld -r)
#ld: --orphan-handling=warn -e _start -r
#source: noinit-sections.s
#target: [supports_noinit_section]
#readelf: -SW
#warning_output: noinit-sections-2.l

#...
 +\[ *[0-9]+\] \.noinit +NOBITS +[0-9a-f]+ +[0-9a-f]+ [0-9a-f]+ +00 +WA .*
#pass
