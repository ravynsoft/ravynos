#name: MIPS LWPC from unaligned symbol 1
#source: unaligned-lwpc-1.s
#source: unaligned-data.s
#as: -mips32r6
#ld: -Ttext 0x1c000000 -Tdata 0x1c080000 -e 0x1c000000
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x[0-9a-f]+\): PC-relative load from unaligned address\n
#error:   \(\.text\+0x[0-9a-f]+\): PC-relative load from unaligned address\Z
