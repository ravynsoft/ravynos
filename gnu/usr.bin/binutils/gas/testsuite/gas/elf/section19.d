#readelf: -SW
#name: linked-to section 2

#...
 +\[ *[0-9]+\] +__patchable_function_entries +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+[248] +00 +WAL +.*
#...
 +\[ *[0-9]+\] +__patchable_function_entries +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+[248] +00 +WAL +.*
#pass
