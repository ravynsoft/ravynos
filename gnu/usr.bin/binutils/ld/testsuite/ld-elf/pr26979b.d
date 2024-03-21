#source: pr26979b.s
#source: pr26979c.s
#target: [check_shared_lib_support]
#as:
#ld: -shared --version-script=pr26979.ver
#readelf: -sW

#...
.* GLOBAL PROTECTED .*foo@@v1
#...
.* GLOBAL PROTECTED .*foo@@v1
#pass
