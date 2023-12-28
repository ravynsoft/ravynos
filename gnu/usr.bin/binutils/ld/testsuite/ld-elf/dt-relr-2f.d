#source: dt-relr-2.s
#ld: -r $DT_RELR_LDFLAGS
#readelf: -rW
#target: [supports_dt_relr]

#...
Relocation section '\.rel(a|)\.data' at offset 0x[0-9a-f]+ contains 5 entries:
#pass
