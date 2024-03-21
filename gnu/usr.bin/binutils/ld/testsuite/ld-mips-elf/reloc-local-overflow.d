#name: MIPS reloc against local symbol overflow
#source: reloc-local-overflow.s
#ld: -Tdata 0x10000000 -e 0
#error: \A[^\n]*:\(\.text\+0x0\): relocation truncated to fit: R_MIPS_26 against `\.data'\Z

# Verify that the section name (`.data') is printed rather than `no symbol'.
