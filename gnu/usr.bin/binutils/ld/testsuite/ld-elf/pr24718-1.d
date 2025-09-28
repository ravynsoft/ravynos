#ld: -shared -version-script pr24718-1.t
#readelf: -V
#target: [check_shared_lib_support]
#xfail: tic6x-*-*
# tic6x requires a non-default emulation.

#...
Version definition section '\.gnu\.version_d' contains 3 entries:
 +Addr: 0x[0-9a-f]+ +Offset: 0x[0-9a-f]+ +Link: [0-9]+ \(\.dynstr\)
 +000000: Rev: 1 +Flags: BASE +Index: 1 +Cnt: 1 +Name: dump
 +0x001c: Rev: 1 +Flags: WEAK +Index: 2 +Cnt: 1 +Name: FOO_DEP
 +0x0038: Rev: 1 +Flags: none +Index: 3 +Cnt: 1 +Name: FOO
#pass
