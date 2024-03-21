#name: .persistent sections (ld -r)
#ld: --orphan-handling=warn -e _start -r
#source: persistent-sections.s
#target: [supports_persistent_section]
#readelf: -SW
#warning_output: persistent-sections-2.l

#...
 +\[ *[0-9]+\] \.persistent +PROGBITS +[0-9a-f]+ +[0-9a-f]+ [0-9a-f]+ +00 +WA .*
#pass
