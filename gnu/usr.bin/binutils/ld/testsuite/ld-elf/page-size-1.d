#source: dummy.s
#ld: -z common-page-size=0x4000 -z max-page-size=0x1000
#error: common page size \(0x4000\) > maximum page size \(0x1000\)
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi
