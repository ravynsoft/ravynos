#name: C6X relocation overflow, PCR_S21
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x10400000
#source: reloc-overflow-pcr-s21.s
#error: .*relocation truncated to fit: R_C6000_PCR_S21.*
