#name: C6X array attribute merging, 4 16
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-array-4.s
#source: attr-array-16.s
#error: .*requires more array alignment than .* preserves
