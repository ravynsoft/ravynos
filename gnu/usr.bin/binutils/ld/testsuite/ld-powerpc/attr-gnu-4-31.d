#source: attr-gnu-4-3.s
#source: attr-gnu-4-1.s
#as: -a32
#ld: -r -melf32ppc
#error: .* uses double-precision hard float, .* uses single-precision hard float
#target: powerpc*-*-*
