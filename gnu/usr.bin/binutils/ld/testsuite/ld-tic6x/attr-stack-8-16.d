#name: C6X stack attribute merging, 8 16
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-stack-8.s
#source: attr-stack-16.s
#error: .*requires more stack alignment than .* preserves
