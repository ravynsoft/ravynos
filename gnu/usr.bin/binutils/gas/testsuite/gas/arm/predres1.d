#name: Execution and Data Prediction Restriction instructions
#source: predres.s
#as: -march=armv8.5-a
#objdump: -dr --prefix-addresses --show-raw-insn

.*: *file format .*arm.*

Disassembly of section .text:
.*> ee071f93 	mcr	15, 0, r1, cr7, cr3, \{4\}
.*> ee072fb3 	mcr	15, 0, r2, cr7, cr3, \{5\}
.*> ee073ff3 	mcr	15, 0, r3, cr7, cr3, \{7\}
