#source: attr-gnu-4-2.s
#source: attr-gnu-4-3.s
#as: -a32
#ld: -r -melf32ppc
#error: .* uses hard float, .* uses soft float
#target: powerpc*-*-*
