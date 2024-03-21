#name: GOT page test 1
#source: got-page-1.s
#ld: -T got-page-1.ld -shared
#readelf: -d
#
# There should be 10 page entries and 2 reserved entries
#
#...
.* \(MIPS_LOCAL_GOTNO\) *12
#pass
