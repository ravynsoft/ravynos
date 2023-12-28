#name: C6X relocation overflow, SBR_U15_W
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tsbr.ld --defsym s=0x20080
#source: reloc-overflow-sbr-u15-w.s
#error: .*relocation truncated to fit: R_C6000_SBR_U15_W.*
