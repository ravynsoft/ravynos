#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 link PC-relative operations 0 (n64, sym32)
#source: ../../../gas/testsuite/gas/mips/mips16-pcrel-0.s
#as: -msym32 -mips64r2 -mmips16e2
#ld: -Ttext 0 -e 0
#dump: mips16e2-pcrel-0.d
