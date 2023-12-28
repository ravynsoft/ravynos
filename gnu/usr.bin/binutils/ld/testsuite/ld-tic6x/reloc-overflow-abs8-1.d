#name: C6X relocation overflow, ABS8
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x100
#source: reloc-overflow-abs8.s
#error: .*relocation truncated to fit: R_C6000_ABS8.*
