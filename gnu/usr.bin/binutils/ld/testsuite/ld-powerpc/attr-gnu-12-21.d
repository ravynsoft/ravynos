#source: attr-gnu-12-2.s
#source: attr-gnu-12-1.s
#as: -a32
#ld: -r -melf32ppc
#error: .* uses r3/r4 for small structure returns, .* uses memory
#target: powerpc*-*-*
