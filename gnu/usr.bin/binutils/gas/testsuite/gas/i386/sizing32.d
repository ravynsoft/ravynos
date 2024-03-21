#name: ix86 insn sizing
#nm: -B
#source: sizing.s

#...
0+03 a adc
0+02 a add
0+03 a and
0+10 a bextr
0+05 a call
0+01 a inc
0+02 a jecxz
0+07 a lcall
0+05 a ljmp
0+05 a pextrw
0+06 a pextrw_store
0+06 a sbb
0+05 a sub
0+05 a vpextrw
0+07 a vpextrw_evex
