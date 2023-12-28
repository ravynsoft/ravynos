#name: C6X relocation overflow, PCR_S10
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x10000800
#source: reloc-overflow-pcr-s10.s
#error: .*relocation truncated to fit: R_C6000_PCR_S10.*
