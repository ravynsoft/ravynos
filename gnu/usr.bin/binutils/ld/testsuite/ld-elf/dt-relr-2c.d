#source: dt-relr-2.s
#ld: -shared $DT_RELR_LDFLAGS
#readelf: -rW -d
#target: [supports_dt_relr]

#...
 0x[0-9a-f]+ \(RELR\)    +0x[0-9a-f]+
 0x[0-9a-f]+ \(RELRSZ\)  +(8|16) \(bytes\)
 0x[0-9a-f]+ \(RELRENT\) +(4|8) \(bytes\)
#...
Relocation section '\.rel(a|)\.dyn' at offset 0x[0-9a-f]+ contains 2 entries:
#...
[0-9a-f]+ +[0-9a-f]+ +R_.*_(RELATIVE|UADDR.*) .*
#...
Relocation section '\.relr\.dyn' at offset 0x[0-9a-f]+ contains 2 entries:
  3 offsets
#pass
