#name: C6X GOT relocations, overflow
#as: -mlittle-endian -mdsbt
#ld: -melf32_tic6x_le -Tdsbt-overflow.ld --dsbt-index 4 -shared
#source: got-reloc-global.s
#error: .*relocation truncated to fit: R_C6000_SBR_GOT_U15_W.*
