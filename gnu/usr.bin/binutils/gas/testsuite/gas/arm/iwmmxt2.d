#objdump: -dr --prefix-addresses --show-raw-insn -miwmmxt
#name: Intel(r) Wireless MMX(tm) technology instructions version 2
#as: -mcpu=xscale+iwmmxt+iwmmxt2 -EL

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <iwmmxt2> ee654186[ 	]+waddhc[ 	]+wr4, wr5, wr6
0+004 <[^>]*> eea87189[ 	]+waddwc[ 	]+wr7, wr8, wr9
0+008 <[^>]*> ce954106[ 	]+wmadduxgt[ 	]+wr4, wr5, wr6
0+00c <[^>]*> 0ec87109[ 	]+wmadduneq[ 	]+wr7, wr8, wr9
0+010 <[^>]*> 1eb54106[ 	]+wmaddsxne[ 	]+wr4, wr5, wr6
0+014 <[^>]*> aee87109[ 	]+wmaddsnge[ 	]+wr7, wr8, wr9
0+018 <[^>]*> eed21103[ 	]+wmulumr[ 	]+wr1, wr2, wr3
0+01c <[^>]*> eef21103[ 	]+wmulsmr[ 	]+wr1, wr2, wr3
0+020 <[^>]*> ce12f190[ 	]+torvscbgt[ 	]+pc
0+024 <[^>]*> 1e52f190[ 	]+torvschne[ 	]+pc
0+028 <[^>]*> 0e92f190[ 	]+torvscweq[ 	]+pc
0+02c <[^>]*> ee2211c0[ 	]+wabsb[ 	]+wr1, wr2
0+030 <[^>]*> ee6431c0[ 	]+wabsh[ 	]+wr3, wr4
0+034 <[^>]*> eea651c0[ 	]+wabsw[ 	]+wr5, wr6
0+038 <[^>]*> ce2211c0[ 	]+wabsbgt[ 	]+wr1, wr2
0+03c <[^>]*> ee1211c3[ 	]+wabsdiffb[ 	]+wr1, wr2, wr3
0+040 <[^>]*> ee5541c6[ 	]+wabsdiffh[ 	]+wr4, wr5, wr6
0+044 <[^>]*> ee9871c9[ 	]+wabsdiffw[ 	]+wr7, wr8, wr9
0+048 <[^>]*> ce1211c3[ 	]+wabsdiffbgt[ 	]+wr1, wr2, wr3
0+04c <[^>]*> ee6211a3[ 	]+waddbhusm[ 	]+wr1, wr2, wr3
0+050 <[^>]*> ee2541a6[ 	]+waddbhusl[ 	]+wr4, wr5, wr6
0+054 <[^>]*> ce6211a3[ 	]+waddbhusmgt[ 	]+wr1, wr2, wr3
0+058 <[^>]*> ce2541a6[ 	]+waddbhuslgt[ 	]+wr4, wr5, wr6
0+05c <[^>]*> eea211a3[ 	]+waddsubhx[ 	]+wr1, wr2, wr3
0+060 <[^>]*> bea541a6[ 	]+waddsubhxlt[ 	]+wr4, wr5, wr6
0+064 <[^>]*> 0ea211a3[ 	]+waddsubhxeq[ 	]+wr1, wr2, wr3
0+068 <[^>]*> cea541a6[ 	]+waddsubhxgt[ 	]+wr4, wr5, wr6
0+06c <[^>]*> ee421003[ 	]+wavg4[ 	]+wr1, wr2, wr3
0+070 <[^>]*> ce454006[ 	]+wavg4gt[ 	]+wr4, wr5, wr6
0+074 <[^>]*> ee521003[ 	]+wavg4r[ 	]+wr1, wr2, wr3
0+078 <[^>]*> ce554006[ 	]+wavg4rgt[ 	]+wr4, wr5, wr6
0+07c <[^>]*> fc711102[ 	]+wldrd[ 	]+wr1, \[r1\], -r2
0+080 <[^>]*> fc712132[ 	]+wldrd[ 	]+wr2, \[r1\], -r2, lsl #3
0+084 <[^>]*> fcf13102[ 	]+wldrd[ 	]+wr3, \[r1\], \+r2
0+088 <[^>]*> fcf14142[ 	]+wldrd[ 	]+wr4, \[r1\], \+r2, lsl #4
0+08c <[^>]*> fd515102[ 	]+wldrd[ 	]+wr5, \[r1, -r2\]
0+090 <[^>]*> fd516132[ 	]+wldrd[ 	]+wr6, \[r1, -r2, lsl #3\]
0+094 <[^>]*> fdd17102[ 	]+wldrd[ 	]+wr7, \[r1, \+r2\]
0+098 <[^>]*> fdd18142[ 	]+wldrd[ 	]+wr8, \[r1, \+r2, lsl #4\]
0+09c <[^>]*> fd719102[ 	]+wldrd[ 	]+wr9, \[r1, -r2\]!
0+0a0 <[^>]*> fd71a132[ 	]+wldrd[ 	]+wr10, \[r1, -r2, lsl #3\]!
0+0a4 <[^>]*> fdf1b102[ 	]+wldrd[ 	]+wr11, \[r1, \+r2\]!
0+0a8 <[^>]*> fdf1c142[ 	]+wldrd[ 	]+wr12, \[r1, \+r2, lsl #4\]!
0+0ac <[^>]*> ee821083[ 	]+wmerge[ 	]+wr1, wr2, wr3, #4
0+0b0 <[^>]*> ce821083[ 	]+wmergegt[ 	]+wr1, wr2, wr3, #4
0+0b4 <[^>]*> 0e3210a3[ 	]+wmiatteq[ 	]+wr1, wr2, wr3
0+0b8 <[^>]*> ce2210a3[ 	]+wmiatbgt[ 	]+wr1, wr2, wr3
0+0bc <[^>]*> 1e1210a3[ 	]+wmiabtne[ 	]+wr1, wr2, wr3
0+0c0 <[^>]*> ce0210a3[ 	]+wmiabbgt[ 	]+wr1, wr2, wr3
0+0c4 <[^>]*> 0e7210a3[ 	]+wmiattneq[ 	]+wr1, wr2, wr3
0+0c8 <[^>]*> 1e6210a3[ 	]+wmiatbnne[ 	]+wr1, wr2, wr3
0+0cc <[^>]*> ce5210a3[ 	]+wmiabtngt[ 	]+wr1, wr2, wr3
0+0d0 <[^>]*> 0e4210a3[ 	]+wmiabbneq[ 	]+wr1, wr2, wr3
0+0d4 <[^>]*> 0eb21123[ 	]+wmiawtteq[ 	]+wr1, wr2, wr3
0+0d8 <[^>]*> cea21123[ 	]+wmiawtbgt[ 	]+wr1, wr2, wr3
0+0dc <[^>]*> 1e921123[ 	]+wmiawbtne[ 	]+wr1, wr2, wr3
0+0e0 <[^>]*> ce821123[ 	]+wmiawbbgt[ 	]+wr1, wr2, wr3
0+0e4 <[^>]*> 1ef21123[ 	]+wmiawttnne[ 	]+wr1, wr2, wr3
0+0e8 <[^>]*> cee21123[ 	]+wmiawtbngt[ 	]+wr1, wr2, wr3
0+0ec <[^>]*> 0ed21123[ 	]+wmiawbtneq[ 	]+wr1, wr2, wr3
0+0f0 <[^>]*> 1ec21123[ 	]+wmiawbbnne[ 	]+wr1, wr2, wr3
0+0f4 <[^>]*> 0ed210c3[ 	]+wmulwumeq[ 	]+wr1, wr2, wr3
0+0f8 <[^>]*> cec210c3[ 	]+wmulwumrgt[ 	]+wr1, wr2, wr3
0+0fc <[^>]*> 1ef210c3[ 	]+wmulwsmne[ 	]+wr1, wr2, wr3
0+100 <[^>]*> 0ee210c3[ 	]+wmulwsmreq[ 	]+wr1, wr2, wr3
0+104 <[^>]*> ceb210c3[ 	]+wmulwlgt[ 	]+wr1, wr2, wr3
0+108 <[^>]*> aeb210c3[ 	]+wmulwlge[ 	]+wr1, wr2, wr3
0+10c <[^>]*> 1eb210a3[ 	]+wqmiattne[ 	]+wr1, wr2, wr3
0+110 <[^>]*> 0ef210a3[ 	]+wqmiattneq[ 	]+wr1, wr2, wr3
0+114 <[^>]*> cea210a3[ 	]+wqmiatbgt[ 	]+wr1, wr2, wr3
0+118 <[^>]*> aee210a3[ 	]+wqmiatbnge[ 	]+wr1, wr2, wr3
0+11c <[^>]*> 1e9210a3[ 	]+wqmiabtne[ 	]+wr1, wr2, wr3
0+120 <[^>]*> 0ed210a3[ 	]+wqmiabtneq[ 	]+wr1, wr2, wr3
0+124 <[^>]*> ce8210a3[ 	]+wqmiabbgt[ 	]+wr1, wr2, wr3
0+128 <[^>]*> 1ec210a3[ 	]+wqmiabbnne[ 	]+wr1, wr2, wr3
0+12c <[^>]*> ce121083[ 	]+wqmulmgt[ 	]+wr1, wr2, wr3
0+130 <[^>]*> 0e321083[ 	]+wqmulmreq[ 	]+wr1, wr2, wr3
0+134 <[^>]*> cec210e3[ 	]+wqmulwmgt[ 	]+wr1, wr2, wr3
0+138 <[^>]*> 0ee210e3[ 	]+wqmulwmreq[ 	]+wr1, wr2, wr3
0+13c <[^>]*> fc611102[ 	]+wstrd[ 	]+wr1, \[r1\], -r2
0+140 <[^>]*> fc612132[ 	]+wstrd[ 	]+wr2, \[r1\], -r2, lsl #3
0+144 <[^>]*> fce13102[ 	]+wstrd[ 	]+wr3, \[r1\], \+r2
0+148 <[^>]*> fce14142[ 	]+wstrd[ 	]+wr4, \[r1\], \+r2, lsl #4
0+14c <[^>]*> fd415102[ 	]+wstrd[ 	]+wr5, \[r1, -r2\]
0+150 <[^>]*> fd416132[ 	]+wstrd[ 	]+wr6, \[r1, -r2, lsl #3\]
0+154 <[^>]*> fdc17102[ 	]+wstrd[ 	]+wr7, \[r1, \+r2\]
0+158 <[^>]*> fdc18142[ 	]+wstrd[ 	]+wr8, \[r1, \+r2, lsl #4\]
0+15c <[^>]*> fd619102[ 	]+wstrd[ 	]+wr9, \[r1, -r2\]!
0+160 <[^>]*> fd61a132[ 	]+wstrd[ 	]+wr10, \[r1, -r2, lsl #3\]!
0+164 <[^>]*> fde1b102[ 	]+wstrd[ 	]+wr11, \[r1, \+r2\]!
0+168 <[^>]*> fde1c142[ 	]+wstrd[ 	]+wr12, \[r1, \+r2, lsl #4\]!
0+16c <[^>]*> ced211c3[ 	]+wsubaddhxgt[ 	]+wr1, wr2, wr3
0+170 <[^>]*> fe721140[ 	]+wrorh[ 	]+wr1, wr2, #16
0+174 <[^>]*> feb21040[ 	]+wrorw[ 	]+wr1, wr2, #32
0+178 <[^>]*> ee021002[ 	]+wor[ 	]+wr1, wr2, wr2
0+17c <[^>]*> fe721145[ 	]+wrorh[ 	]+wr1, wr2, #21
0+180 <[^>]*> feb2104d[ 	]+wrorw[ 	]+wr1, wr2, #13
0+184 <[^>]*> fef2104e[ 	]+wrord[ 	]+wr1, wr2, #14
0+188 <[^>]*> fe721140[ 	]+wrorh[ 	]+wr1, wr2, #16
0+18c <[^>]*> feb21040[ 	]+wrorw[ 	]+wr1, wr2, #32
0+190 <[^>]*> ee021002[ 	]+wor[ 	]+wr1, wr2, wr2
0+194 <[^>]*> fe59204b[ 	]+wsllh[ 	]+wr2, wr9, #11
0+198 <[^>]*> fe95304d[ 	]+wsllw[ 	]+wr3, wr5, #13
0+19c <[^>]*> fed8304f[ 	]+wslld[ 	]+wr3, wr8, #15
0+1a0 <[^>]*> fe721140[ 	]+wrorh[ 	]+wr1, wr2, #16
0+1a4 <[^>]*> feb21040[ 	]+wrorw[ 	]+wr1, wr2, #32
0+1a8 <[^>]*> ee021002[ 	]+wor[ 	]+wr1, wr2, wr2
0+1ac <[^>]*> fe49204c[ 	]+wsrah[ 	]+wr2, wr9, #12
0+1b0 <[^>]*> fe85304e[ 	]+wsraw[ 	]+wr3, wr5, #14
0+1b4 <[^>]*> fec83140[ 	]+wsrad[ 	]+wr3, wr8, #16
0+1b8 <[^>]*> fe721140[ 	]+wrorh[ 	]+wr1, wr2, #16
0+1bc <[^>]*> feb21040[ 	]+wrorw[ 	]+wr1, wr2, #32
0+1c0 <[^>]*> ee021002[ 	]+wor[ 	]+wr1, wr2, wr2
0+1c4 <[^>]*> fe69204c[ 	]+wsrlh[ 	]+wr2, wr9, #12
0+1c8 <[^>]*> fea5304e[ 	]+wsrlw[ 	]+wr3, wr5, #14
0+1cc <[^>]*> fee83140[ 	]+wsrld[ 	]+wr3, wr8, #16
