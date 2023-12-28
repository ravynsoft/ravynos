#name: C6X compatibility attribute merging, other gnu
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-compatibility-other.s
#source: attr-compatibility-gnu.s
#error: .*object tag '1, gnu' is incompatible with tag '1, other'
