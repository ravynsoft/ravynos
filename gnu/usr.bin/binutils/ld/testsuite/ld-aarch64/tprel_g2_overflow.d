#name: TLS offset out of range - TPREL_G2
#source: tprel_g2_overflow.s
#as:
#ld: -e0
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLE_MOVW_TPREL_G2 against symbol `i' .*

