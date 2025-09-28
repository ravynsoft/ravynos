#name: MIPS JAL to local symbol overflow 1
#source: jal-local-overflow.s
#ld: -Ttext 0x1fffd000 -e 0x1fffd000
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x2000\): relocation truncated to fit: R_MIPS_26 against `.text'\n
#error:   [^\n]*: in function `bar':\n
#error:   \(\.text\+0x4000\): relocation truncated to fit: R_MIPS_26 against `.text'\Z
