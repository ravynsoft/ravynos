#source: emit-relocs-88-overflow.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_P32_TLSLD_MOVW_DTPREL_G0 against symbol `v2' .*
