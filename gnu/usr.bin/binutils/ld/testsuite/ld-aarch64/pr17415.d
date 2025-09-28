#name: TLS offset out of range
#source: pr17415.s
#as:
#ld: -e0
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLE_ADD_TPREL_HI12 against symbol `i' .*

