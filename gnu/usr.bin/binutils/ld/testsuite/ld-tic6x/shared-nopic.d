#name: C6X shared library without PIC code
#as: -mlittle-endian -mdsbt -mpid=near
#ld: -melf32_tic6x_le -Tdsbt-inrange.ld --dsbt-index 4 -shared
#source: got-reloc-global.s
#warning: non-PIC code
