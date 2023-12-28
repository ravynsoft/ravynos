#name: C6X GOT relocations, addend
#as: -mlittle-endian -mdsbt
#ld: -melf32_tic6x_le -Tdsbt.ld --dsbt-index 4 -shared
#source: got-reloc-global-addend-1.s
#error: .*relocation R_C6000_SBR_GOT_U15_W with non-zero addend 4.*
