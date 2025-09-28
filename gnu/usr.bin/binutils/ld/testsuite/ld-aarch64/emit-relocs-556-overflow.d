#source: emit-relocs-556-overflow.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLE_LDST32_TPREL_LO12 against symbol `v2' .*
