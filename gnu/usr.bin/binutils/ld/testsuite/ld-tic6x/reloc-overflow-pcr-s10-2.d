#name: C6X relocation underflow, PCR_S10
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x0ffff7fc
#source: reloc-overflow-pcr-s10.s
#error: .*relocation truncated to fit: R_C6000_PCR_S10.*
