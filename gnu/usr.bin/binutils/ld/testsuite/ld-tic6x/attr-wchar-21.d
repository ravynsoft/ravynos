#name: C6X wchar_t attribute merging, 2 1
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-wchar-2.s
#source: attr-wchar-1.s
#warning: .*differ in wchar_t size
