#source: emit-relocs-552-overflow.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_P32_TLSLE_LDST8_TPREL_LO12 against symbol `v2' .*
