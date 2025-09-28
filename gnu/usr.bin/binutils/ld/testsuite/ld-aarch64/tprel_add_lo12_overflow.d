#name: TLS offset out of range - TPREL_ADD_LO12
#source: tprel_add_lo12_overflow.s
#as:
#ld: -e0
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLE_ADD_TPREL_LO12 against symbol `i' .*

