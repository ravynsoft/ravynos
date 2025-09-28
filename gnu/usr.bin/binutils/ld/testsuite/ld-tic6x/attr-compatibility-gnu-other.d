#name: C6X compatibility attribute merging, gnu other
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-compatibility-gnu.s
#source: attr-compatibility-other.s
#error: .*object has vendor-specific contents that must be processed by the 'other' toolchain
