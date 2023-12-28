#source: emit-relocs-531-overflow.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLD_LDST8_DTPREL_LO12 against symbol `v2' .*
