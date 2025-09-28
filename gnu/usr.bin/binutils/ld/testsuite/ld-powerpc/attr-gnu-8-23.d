#source: attr-gnu-8-2.s
#source: attr-gnu-8-3.s
#as: -a32
#ld: -r -melf32ppc
#error: .* uses AltiVec vector ABI, .* uses SPE vector ABI
#target: powerpc*-*-*
