#name: C6X relocation overflow, SBR_U15_H
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tsbr.ld --defsym s=0x10080
#source: reloc-overflow-sbr-u15-h.s
#error: .*relocation truncated to fit: R_C6000_SBR_U15_H.*
