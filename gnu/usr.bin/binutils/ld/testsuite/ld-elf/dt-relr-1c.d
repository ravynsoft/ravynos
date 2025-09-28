#source: dt-relr-1.s
#ld: -shared $DT_RELR_LDFLAGS
#readelf: -rW -d
#target: [supports_dt_relr]

#failif
#...
Relocation section '\.relr\.dyn' at offset .*
#pass
