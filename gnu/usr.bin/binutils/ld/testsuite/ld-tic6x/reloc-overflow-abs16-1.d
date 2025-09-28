#name: C6X relocation overflow, ABS16
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x10000
#source: reloc-overflow-abs16.s
#error: .*relocation truncated to fit: R_C6000_ABS16.*
