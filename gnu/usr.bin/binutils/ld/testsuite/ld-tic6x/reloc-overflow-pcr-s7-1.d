#name: C6X relocation overflow, PCR_S7
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x10000100
#source: reloc-overflow-pcr-s7.s
#error: .*relocation truncated to fit: R_C6000_PCR_S7.*
