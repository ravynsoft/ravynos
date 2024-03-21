#name: C6X invalid DSBT_INDEX
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tsbr.ld --dsbt-index 5 --dsbt-size 3
#source: dsbt-index.s
#error: .*invalid --dsbt-index 5, outside DSBT size.*
