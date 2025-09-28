#source: ifunc-3.s
#target: [check_shared_lib_support]
#ld: -shared
#readelf: -r --wide

#...
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_[_0-9A-Z]+_IRELATIVE[ ]*[0-9a-f]*
#pass
