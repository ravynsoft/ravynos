# source: tls-2.s
# ld: --entry foo -q --force-dynamic
# readelf: -d

#...
 0x60000010 \(Operating System specific: 60000010\)        0x[0-9a-f]+
 0x60000011 \(Operating System specific: 60000011\)        0x4
 0x60000015 \(Operating System specific: 60000015\)        0x4
 0x60000012 \(Operating System specific: 60000012\)        0x[0-9a-f]+
 0x60000013 \(Operating System specific: 60000013\)        0xc
#...
