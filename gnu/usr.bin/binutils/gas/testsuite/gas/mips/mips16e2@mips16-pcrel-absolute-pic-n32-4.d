#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative reference to absolute expression 4 (PIC, n32)
#as: -n32 -call_shared
#source: mips16-pcrel-absolute-4.s
#dump: mips16e2@mips16-pcrel-absolute-4.d
