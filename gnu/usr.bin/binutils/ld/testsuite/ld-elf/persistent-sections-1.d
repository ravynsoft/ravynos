#name: .persistent sections
#ld: --orphan-handling=warn -e _start
#source: persistent-sections.s
#target: [supports_persistent_section]
#readelf: -SW

#...
 +\[ *[0-9]+\] \.persistent +PROGBITS +[0-9a-f]+ +[0-9a-f]+ [0-9a-f]+ +00 +WA .*
#pass
