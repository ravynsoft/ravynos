#source: nps-1.s
#as: -mcpu=arc700 -mnps400
#ld: -defsym=foo=0x56f03000 -T sda-relocs.ld
#error_output: nps-1b.err
