#source: dt-relr-3.s
#ld: -e _start -pie $DT_RELR_LDFLAGS
#readelf: -rW -d
#target: [supports_dt_relr]

#failif
#...
Relocation section '\.relr\.dyn' at offset .*
#pass
