#name: PRU R_PRU_U8_PCREL illegal offset 0
#source: pcrel_u8-illegal2.s
#source: pcrel_u8_label.s
#ld:
#error: [^\n]*: relocation out of range

# Check that LOOP cannot reference "prior" labels.
