#name: PRU R_PRU_U8_PCREL illegal
#source: pcrel_u8-illegal.s
#source: pcrel_u8_label.s
#ld:
#error: [^\n]*: relocation truncated to fit: R_PRU_U8_PCREL against `.init0'

# Check that LOOP cannot reference "prior" labels.
