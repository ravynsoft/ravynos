#name: MIPS JAL to global symbol overflow 1
#source: jal-global-overflow.s
#ld: -Ttext 0x1fffd000 -e 0x1fffd000
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x2000\): relocation truncated to fit: R_MIPS_26 against `abar'\n
#error:   [^\n]*: in function `bar':\n
#error:   \(\.text\+0x4000\): relocation truncated to fit: R_MIPS_26 against `afoo'\Z
