send(8)         null<1>F        g123<8,8,1>F    0x8a080017
                            urb MsgDesc: 1 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         null<1>F        g13<8,8,1>F     0x12080007
                            urb MsgDesc: 0 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080027
                            urb MsgDesc: 2 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(16)        g9<1>UD         g2<8,8,1>UD     0x02280300
                            const MsgDesc: (0, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         null<1>F        g119<8,8,1>F    0x92080017
                            urb MsgDesc: 1 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         null<1>UW       g3<8,8,1>UD     0x0e0b5001
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0x0) mlen 7 rlen 0 { align1 1Q };
send(8)         null<1>UW       g1<8,8,1>UD     0x0e0b6001
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0x0) mlen 7 rlen 0 { align1 2Q };
send(16)        null<1>UW       g127<8,8,1>UW   0x82000010
                            thread_spawner MsgDesc: mlen 1 rlen 0           { align1 WE_all 1H EOT };
send(8)         g124<1>UW       g13<8,8,1>UD    0x08427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g10<8,8,1>UD    0x10847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g10<1>UD        g2<8,8,1>UD     0x02480028
                            urb MsgDesc: 2 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         null<1>F        g8<8,8,1>F      0x140a0017
                            urb MsgDesc: 1 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g118<8,8,1>F    0x940a0017
                            urb MsgDesc: 1 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q EOT };
send(8)         null<1>F        g11<8,8,1>UD    0x0c0a0037
                            urb MsgDesc: 3 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0a080027
                            urb MsgDesc: 2 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0c088017
                            urb MsgDesc: 1 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0a088017
                            urb MsgDesc: 1 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x08088017
                            urb MsgDesc: 1 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g2<8,8,1>UD     0x06088017
                            urb MsgDesc: 1 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0c088007
                            urb MsgDesc: 0 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0a088007
                            urb MsgDesc: 0 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x02480008
                            urb MsgDesc: 0 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         g6<1>UD         g6<8,8,1>UD     0x02480018
                            urb MsgDesc: 1 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         null<1>F        g125<8,8,1>UD   0x86088007
                            urb MsgDesc: 0 SIMD8 write masked mlen 3 rlen 0 { align1 1Q EOT };
send(8)         g7<1>UW         g7<8,8,1>UD     0x04427000
                            sampler MsgDesc: ld SIMD8 Surface = 0 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>F     0x12080017
                            urb MsgDesc: 1 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g20<8,8,1>F     0x12080037
                            urb MsgDesc: 3 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080057
                            urb MsgDesc: 5 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x0643d001
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g8<1>UW         g16<8,8,1>UD    0x0c85d001
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g8<1>UW         g17<8,8,1>UD    0x0a43e001
                            sampler MsgDesc: ld2dms SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g34<1>UW        g16<8,8,1>UD    0x1485e001
                            sampler MsgDesc: ld2dms SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g5<1>UW         g7<8,8,1>UD     0x0242a000
                            sampler MsgDesc: resinfo SIMD8 Surface = 0 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g15<8,8,1>UD    0x064a8000
                            sampler MsgDesc: gather4 SIMD8 Surface = 0 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g11<8,8,1>UD    0x06420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g19<8,8,1>UD    0x0c840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g12<1>UW        g5<8,8,1>UD     0x02427000
                            sampler MsgDesc: ld SIMD8 Surface = 0 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080037
                            urb MsgDesc: 3 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         g124<1>UW       g7<8,8,1>UD     0x144a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 10 rlen 4 { align1 1Q };
(+f1.0) send(16) g122<1>UW      g9<8,8,1>UD     0x0420a501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, inc) mlen 2 rlen 2 { align1 1H };
send(8)         g12<1>UW        g7<8,8,1>UD     0x0443d001
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g13<1>UW        g5<8,8,1>UD     0x0843e001
                            sampler MsgDesc: ld2dms SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g15<1>UW        g11<8,8,1>UD    0x0885d001
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g27<1>UW        g7<8,8,1>UD     0x1085e001
                            sampler MsgDesc: ld2dms SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
(+f1.0) send(8) g125<1>UW       g3<8,8,1>UD     0x0210b501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, inc) mlen 1 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     a0<0,1,0>UD     0x00000200
                            sampler MsgDesc: indirect                       { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x084a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x064a8001
                            sampler MsgDesc: gather4 SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g12<8,8,1>UD    0x0a8c8001
                            sampler MsgDesc: gather4 SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0a4a6001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g12<8,8,1>UD    0x0a4a6102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x128c6001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(16)        g10<1>UW        g20<8,8,1>UD    0x128c6102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(8)         g124<1>UW       g3<8,8,1>UD     0x0a43e000
                            sampler MsgDesc: ld2dms SIMD8 Surface = 0 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080027
                            urb MsgDesc: 2 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         g2<1>UW         g3<8,8,1>UD     0x0643d000
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 0 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         null<1>F        g7<8,8,1>UD     0x0a080037
                            urb MsgDesc: 3 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g8<8,8,1>UD     0x0a080047
                            urb MsgDesc: 4 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g29<8,8,1>F     0x0c0a0017
                            urb MsgDesc: 1 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g122<8,8,1>F    0x8c0a0017
                            urb MsgDesc: 1 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         g5<1>UW         g21<8,8,1>UD    0x02420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g26<8,8,1>UD    0x04840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g8<1>UW         g10<8,8,1>UD    0x0242a001
                            sampler MsgDesc: resinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0c4b1001
                            sampler MsgDesc: gather4_po SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g20<8,8,1>UD    0x0484a001
                            sampler MsgDesc: resinfo SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(16)        g18<1>UW        g7<8,8,1>UD     0x168d1001
                            sampler MsgDesc: gather4_po SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         null<1>F        g6<8,8,1>UD     0x0a088027
                            urb MsgDesc: 2 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g7<8,8,1>UD     0x0a088037
                            urb MsgDesc: 3 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g8<8,8,1>UD     0x0a088047
                            urb MsgDesc: 4 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g9<8,8,1>UD     0x0a088057
                            urb MsgDesc: 5 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x084a8001
                            sampler MsgDesc: gather4 SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g14<1>UW        g7<8,8,1>UD     0x0e8c8001
                            sampler MsgDesc: gather4 SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x0c424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0c4b1000
                            sampler MsgDesc: gather4_po SIMD8 Surface = 0 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x0242a101
                            sampler MsgDesc: resinfo SIMD8 Surface = 1 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x0242a202
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g14<8,8,1>UD    0x0242a303
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x0242a404
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g22<8,8,1>UD    0x0242a505
                            sampler MsgDesc: resinfo SIMD8 Surface = 5 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x0242a606
                            sampler MsgDesc: resinfo SIMD8 Surface = 6 Sampler = 6 mlen 1 rlen 4 { align1 1Q };
send(8)         g6<1>UD         g15<8,8,1>UD    0x042a0318
                            urb MsgDesc: 49 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g8<1>UD         g15<8,8,1>UD    0x042a0518
                            urb MsgDesc: 81 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g10<1>UD        g15<8,8,1>UD    0x042a0718
                            urb MsgDesc: 113 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g12<1>UD        g15<8,8,1>UD    0x042a0918
                            urb MsgDesc: 145 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g14<1>UD        g15<8,8,1>UD    0x042a0128
                            urb MsgDesc: 18 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g16<1>UD        g14<8,8,1>UD    0x042a0218
                            urb MsgDesc: 33 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g18<1>UD        g14<8,8,1>UD    0x042a0418
                            urb MsgDesc: 65 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g20<1>UD        g14<8,8,1>UD    0x042a0618
                            urb MsgDesc: 97 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g22<1>UD        g14<8,8,1>UD    0x042a0818
                            urb MsgDesc: 129 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g13<1>UD        g14<8,8,1>UD    0x042a0028
                            urb MsgDesc: 2 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g30<8,8,1>UD    0x02480208
                            urb MsgDesc: 32 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g14<1>UD        g30<8,8,1>UD    0x02480408
                            urb MsgDesc: 64 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g18<1>UD        g30<8,8,1>UD    0x02480608
                            urb MsgDesc: 96 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g30<8,8,1>UD    0x02480808
                            urb MsgDesc: 128 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0a0a8217
                            urb MsgDesc: 33 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x0a0a8227
                            urb MsgDesc: 34 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0a0a8237
                            urb MsgDesc: 35 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x0a0a8247
                            urb MsgDesc: 36 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x0a0a8257
                            urb MsgDesc: 37 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x0a0a8267
                            urb MsgDesc: 38 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0a0a8277
                            urb MsgDesc: 39 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0a0a8287
                            urb MsgDesc: 40 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0a0a8297
                            urb MsgDesc: 41 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0a0a82a7
                            urb MsgDesc: 42 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0a0a82b7
                            urb MsgDesc: 43 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0a0a82c7
                            urb MsgDesc: 44 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0a0a82d7
                            urb MsgDesc: 45 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0a0a82e7
                            urb MsgDesc: 46 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0a0a82f7
                            urb MsgDesc: 47 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0a0a8307
                            urb MsgDesc: 48 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0a0a8317
                            urb MsgDesc: 49 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0a0a8327
                            urb MsgDesc: 50 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0a0a8337
                            urb MsgDesc: 51 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0a0a8347
                            urb MsgDesc: 52 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0a0a8357
                            urb MsgDesc: 53 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0a0a8367
                            urb MsgDesc: 54 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0a0a8377
                            urb MsgDesc: 55 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0a0a8387
                            urb MsgDesc: 56 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0a0a8397
                            urb MsgDesc: 57 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0a0a83a7
                            urb MsgDesc: 58 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0a0a83b7
                            urb MsgDesc: 59 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0a0a83c7
                            urb MsgDesc: 60 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0a0a83d7
                            urb MsgDesc: 61 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0a0a83e7
                            urb MsgDesc: 62 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x0a0a83f7
                            urb MsgDesc: 63 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x08088027
                            urb MsgDesc: 2 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x08088037
                            urb MsgDesc: 3 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x08088047
                            urb MsgDesc: 4 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x08088057
                            urb MsgDesc: 5 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x08088067
                            urb MsgDesc: 6 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x08088077
                            urb MsgDesc: 7 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x08088087
                            urb MsgDesc: 8 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x08088097
                            urb MsgDesc: 9 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x080880a7
                            urb MsgDesc: 10 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x080880b7
                            urb MsgDesc: 11 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x080880c7
                            urb MsgDesc: 12 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x080880d7
                            urb MsgDesc: 13 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x080880e7
                            urb MsgDesc: 14 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x080880f7
                            urb MsgDesc: 15 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x08088107
                            urb MsgDesc: 16 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x08088117
                            urb MsgDesc: 17 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x08088127
                            urb MsgDesc: 18 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x08088137
                            urb MsgDesc: 19 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x08088147
                            urb MsgDesc: 20 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x08088157
                            urb MsgDesc: 21 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x08088167
                            urb MsgDesc: 22 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x08088177
                            urb MsgDesc: 23 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x08088187
                            urb MsgDesc: 24 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x08088197
                            urb MsgDesc: 25 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x080881a7
                            urb MsgDesc: 26 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x080881b7
                            urb MsgDesc: 27 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x080881c7
                            urb MsgDesc: 28 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x080881d7
                            urb MsgDesc: 29 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x080881e7
                            urb MsgDesc: 30 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x080881f7
                            urb MsgDesc: 31 SIMD8 write masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x0c0a0207
                            urb MsgDesc: 32 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080057
                            urb MsgDesc: 5 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         g10<1>UW        g18<8,8,1>UD    0x084a8000
                            sampler MsgDesc: gather4 SIMD8 Surface = 0 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g1<8,8,1>UD     0x0c847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        null<1>UW       g57<8,8,1>UD    0x04008502
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x08842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g3<8,8,1>UD     0x06427000
                            sampler MsgDesc: ld SIMD8 Surface = 0 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g5<1>UW         g3<8,8,1>UD     0x02427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g8<1>UW         g5<8,8,1>UD     0x04847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         null<1>F        g119<8,8,1>F    0x92080007
                            urb MsgDesc: 0 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         null<1>F        g126<8,8,1>UD   0x84080017
                            urb MsgDesc: 1 SIMD8 write mlen 2 rlen 0        { align1 1Q EOT };
send(8)         g124<1>UW       g9<8,8,1>UD     0x08421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g14<8,8,1>UD    0x10841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g38<1>UD        g1<8,8,1>UD     0x02180028
                            urb MsgDesc: 2 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g40<1>UD        g1<8,8,1>UD     0x02180038
                            urb MsgDesc: 3 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g42<1>UD        g1<8,8,1>UD     0x02180048
                            urb MsgDesc: 4 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g44<1>UD        g1<8,8,1>UD     0x02180058
                            urb MsgDesc: 5 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g46<1>UD        g1<8,8,1>UD     0x02180068
                            urb MsgDesc: 6 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g48<1>UD        g1<8,8,1>UD     0x02180078
                            urb MsgDesc: 7 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g50<1>UD        g1<8,8,1>UD     0x02180088
                            urb MsgDesc: 8 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g52<1>UD        g1<8,8,1>UD     0x02180098
                            urb MsgDesc: 9 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g54<1>UD        g1<8,8,1>UD     0x021800a8
                            urb MsgDesc: 10 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g56<1>UD        g1<8,8,1>UD     0x021800b8
                            urb MsgDesc: 11 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g58<1>UD        g1<8,8,1>UD     0x021800c8
                            urb MsgDesc: 12 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g60<1>UD        g1<8,8,1>UD     0x021800d8
                            urb MsgDesc: 13 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g62<1>UD        g1<8,8,1>UD     0x021800e8
                            urb MsgDesc: 14 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g64<1>UD        g1<8,8,1>UD     0x021800f8
                            urb MsgDesc: 15 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g66<1>UD        g1<8,8,1>UD     0x02180108
                            urb MsgDesc: 16 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g68<1>UD        g1<8,8,1>UD     0x02180118
                            urb MsgDesc: 17 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g70<1>UD        g1<8,8,1>UD     0x02180128
                            urb MsgDesc: 18 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g72<1>UD        g1<8,8,1>UD     0x02180138
                            urb MsgDesc: 19 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g74<1>UD        g1<8,8,1>UD     0x02180148
                            urb MsgDesc: 20 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g76<1>UD        g1<8,8,1>UD     0x02180158
                            urb MsgDesc: 21 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g78<1>UD        g1<8,8,1>UD     0x02180168
                            urb MsgDesc: 22 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g80<1>UD        g1<8,8,1>UD     0x02180178
                            urb MsgDesc: 23 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g82<1>UD        g1<8,8,1>UD     0x02180188
                            urb MsgDesc: 24 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g84<1>UD        g1<8,8,1>UD     0x02180198
                            urb MsgDesc: 25 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g86<1>UD        g1<8,8,1>UD     0x021801a8
                            urb MsgDesc: 26 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g88<1>UD        g1<8,8,1>UD     0x021801b8
                            urb MsgDesc: 27 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g90<1>UD        g1<8,8,1>UD     0x021801c8
                            urb MsgDesc: 28 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g92<1>UD        g1<8,8,1>UD     0x021801d8
                            urb MsgDesc: 29 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g94<1>UD        g1<8,8,1>UD     0x021801e8
                            urb MsgDesc: 30 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g96<1>UD        g1<8,8,1>UD     0x021801f8
                            urb MsgDesc: 31 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g98<1>UD        g1<8,8,1>UD     0x02180208
                            urb MsgDesc: 32 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0c0a0027
                            urb MsgDesc: 2 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>UW       g126<8,8,1>UD   0x040a02fd
                            data MsgDesc: ( DC OWORD block write, 253, 2) mlen 2 rlen 0 { align1 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c001b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 27, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c001c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 28, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c001d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 29, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c001e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 30, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c001f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 31, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0020
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 32, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0021
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 33, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g25<8,8,1>F     0x12080057
                            urb MsgDesc: 5 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g34<8,8,1>F     0x12080077
                            urb MsgDesc: 7 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g43<8,8,1>F     0x12080097
                            urb MsgDesc: 9 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g52<8,8,1>F     0x120800b7
                            urb MsgDesc: 11 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g61<8,8,1>F     0x120800d7
                            urb MsgDesc: 13 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g70<8,8,1>F     0x120800f7
                            urb MsgDesc: 15 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         g2<1>UW         g0<8,8,1>F      0x021c0000
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 0, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g3<1>UW         g0<8,8,1>F      0x021c0001
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 1, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c0002
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 2, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g5<1>UW         g0<8,8,1>F      0x021c0003
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 3, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c0004
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 4, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g7<1>UW         g0<8,8,1>F      0x021c0005
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 5, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g8<1>UW         g0<8,8,1>F      0x021c0006
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 6, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g9<1>UW         g0<8,8,1>F      0x021c0007
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 7, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g10<1>UW        g0<8,8,1>F      0x021c0008
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 8, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         null<1>F        g2<8,8,1>F      0x12080117
                            urb MsgDesc: 17 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         g2<1>UW         g0<8,8,1>F      0x021c0009
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 9, 0) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g3<1>UW         g0<8,8,1>F      0x021c000a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 10, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c000b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 11, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g5<1>UW         g0<8,8,1>F      0x021c000c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 12, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c000d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 13, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g7<1>UW         g0<8,8,1>F      0x021c000e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 14, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g8<1>UW         g0<8,8,1>F      0x021c000f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 15, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g9<1>UW         g0<8,8,1>F      0x021c0010
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 16, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g10<1>UW        g0<8,8,1>F      0x021c0011
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 17, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g2<8,8,1>F      0x12080137
                            urb MsgDesc: 19 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         g2<1>UW         g0<8,8,1>F      0x021c0012
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 18, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g3<1>UW         g0<8,8,1>F      0x021c0013
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 19, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c0014
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 20, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g5<1>UW         g0<8,8,1>F      0x021c0015
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 21, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c0016
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 22, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g7<1>UW         g0<8,8,1>F      0x021c0017
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 23, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g8<1>UW         g0<8,8,1>F      0x021c0018
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 24, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g9<1>UW         g0<8,8,1>F      0x021c0019
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 25, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g10<1>UW        g0<8,8,1>F      0x021c001a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 26, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g2<8,8,1>F      0x12080157
                            urb MsgDesc: 21 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g79<8,8,1>F     0x12080177
                            urb MsgDesc: 23 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g88<8,8,1>F     0x12080197
                            urb MsgDesc: 25 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g97<8,8,1>F     0x120801b7
                            urb MsgDesc: 27 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g106<8,8,1>F    0x120801d7
                            urb MsgDesc: 29 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g117<8,8,1>F    0x920801f7
                            urb MsgDesc: 31 SIMD8 write mlen 9 rlen 0       { align1 1Q EOT };
send(8)         g124<1>UW       g12<8,8,1>UD    0x0a4b1001
                            sampler MsgDesc: gather4_po SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x128d1001
                            sampler MsgDesc: gather4_po SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x02429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x04849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g124<1>UW       g3<8,8,1>UD     0x08427000
                            sampler MsgDesc: ld SIMD8 Surface = 0 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        null<1>UW       g40<8,8,1>UD    0x04008501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(8)         null<1>F        g127<8,8,1>UD   0x82080007
                            urb MsgDesc: 0 SIMD8 write mlen 1 rlen 0        { align1 1Q EOT };
send(8)         g124<1>UW       g9<8,8,1>UD     0x0a4a8000
                            sampler MsgDesc: gather4 SIMD8 Surface = 0 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g10<8,8,1>UD    0x0e434001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
(+f1.0) send(8) g12<1>UW        g2<8,8,1>UD     0x0410b201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, or) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) null<1>UW       g2<8,8,1>UD     0x02009501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, inc) mlen 1 rlen 0 { align1 1Q };
(+f1.0) send(16) g14<1>UW       g16<8,8,1>UD    0x0820a201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, or) mlen 4 rlen 2 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x08434001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g119<1>UW       g0<8,8,1>F      0x021c0022
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 34, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g120<1>UW       g0<8,8,1>F      0x021c0023
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 35, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g102<8,8,1>F    0x120801f7
                            urb MsgDesc: 31 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g121<8,8,1>F    0x8a080217
                            urb MsgDesc: 33 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(16)        null<1>UW       g3<8,8,1>UD     0x08025efe
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 254, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g3<8,8,1>UD     0x02008004
                            gateway MsgDesc: (barrier msg) mlen 1 rlen 0    { align1 WE_all 1H };
send(16)        null<1>UW       g7<8,8,1>UD     0x080087fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, add) mlen 4 rlen 0 { align1 1H };
send(16)        g3<1>UW         g5<8,8,1>UD     0x04205efe
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 254, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
send(8)         g124<1>UW       g3<8,8,1>UD     0x06423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g10<8,8,1>UD    0x0c843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g30<8,8,1>F     0x140a0027
                            urb MsgDesc: 2 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>F     0x0c0a0047
                            urb MsgDesc: 4 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g126<8,8,1>UD   0x84080007
                            urb MsgDesc: 0 SIMD8 write mlen 2 rlen 0        { align1 1Q EOT };
send(8)         g5<1>UW         g11<8,8,1>UD    0x06495001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 3 rlen 4 { align1 1Q };
send(8)         null<1>UW       g14<8,8,1>UD    0x0e0b5002
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0x0) mlen 7 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g11<8,8,1>UD    0x06496001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 3 rlen 4 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x0e0b6002
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0x0) mlen 7 rlen 0 { align1 2Q };
send(8)         g13<1>UD        g3<8,8,1>UD     0x02480038
                            urb MsgDesc: 3 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         null<1>F        g7<8,8,1>F      0x140a0037
                            urb MsgDesc: 3 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         g15<1>UD        g2<8,8,1>UD     0x02280038
                            urb MsgDesc: 3 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080037
                            urb MsgDesc: 3 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         null<1>F        g8<8,8,1>F      0x140a0007
                            urb MsgDesc: 0 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g118<8,8,1>F    0x940a0007
                            urb MsgDesc: 0 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q EOT };
send(8)         g116<1>UW       g0<8,8,1>F      0x021c0024
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 36, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g117<1>UW       g0<8,8,1>F      0x021c0025
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 37, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g118<1>UW       g0<8,8,1>F      0x021c0026
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 38, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g119<1>UW       g0<8,8,1>F      0x021c0027
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 39, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g120<1>UW       g0<8,8,1>F      0x021c0028
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 40, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g10<1>UD        g2<8,8,1>UD     0x02480048
                            urb MsgDesc: 4 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         g6<1>UD         g2<8,8,1>UD     0x02480088
                            urb MsgDesc: 8 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         g14<1>UD        g2<8,8,1>UD     0x02480058
                            urb MsgDesc: 5 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         g11<1>UD        g2<8,8,1>UD     0x024800a8
                            urb MsgDesc: 10 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g18<1>UD        g2<8,8,1>UD     0x02480068
                            urb MsgDesc: 6 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         g16<1>UD        g2<8,8,1>UD     0x023800c8
                            urb MsgDesc: 12 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g2<8,8,1>UD     0x02480078
                            urb MsgDesc: 7 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x024800b8
                            urb MsgDesc: 11 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g7<1>UD         g2<8,8,1>UD     0x02480098
                            urb MsgDesc: 9 SIMD8 read mlen 1 rlen 4         { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x920800b7
                            urb MsgDesc: 11 SIMD8 write mlen 9 rlen 0       { align1 1Q EOT };
send(8)         g4<1>UW         g10<8,8,1>UD    0x084b0000
                            sampler MsgDesc: gather4_c SIMD8 Surface = 0 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g7<1>UW         g0<8,8,1>UD     0x02200008
                            pixel interp MsgDesc: (persp, per_message_offset, 0x08) mlen 1 rlen 2 { align1 1Q };
send(16)        g9<1>UW         g0<8,8,1>UD     0x02410008
                            pixel interp MsgDesc: (persp, per_message_offset, 0x08) mlen 1 rlen 4 { align1 1H };
send(8)         g2<1>UW         g6<8,8,1>UD     0x0a4b1000
                            sampler MsgDesc: gather4_po SIMD8 Surface = 0 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g74<1>UD        g2<8,8,1>UD     0x02280028
                            urb MsgDesc: 2 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         g7<1>UD         g2<8,8,1>UD     0x02380028
                            urb MsgDesc: 2 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         g15<1>UD        g2<8,8,1>UD     0x02380038
                            urb MsgDesc: 3 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         g124<1>UW       g3<8,8,1>UD     0x0843e000
                            sampler MsgDesc: ld2dms SIMD8 Surface = 0 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g3<8,8,1>UD     0x0443d000
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 0 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g11<8,8,1>UD    0x0a4a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g19<8,8,1>UD    0x128c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         null<1>F        g2<8,8,1>F      0x0c0a0057
                            urb MsgDesc: 5 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g9<8,8,1>UD     0x04080027
                            urb MsgDesc: 2 SIMD8 write mlen 2 rlen 0        { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x04429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g6<1>UW         g11<8,8,1>UD    0x08434102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g7<1>UW         g7<8,8,1>UD     0x024ab000
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 0 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         null<1>F        g50<8,8,1>F     0x140a0057
                            urb MsgDesc: 5 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g60<8,8,1>F     0x140a0077
                            urb MsgDesc: 7 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g70<8,8,1>F     0x0c0a0097
                            urb MsgDesc: 9 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g122<8,8,1>F    0x8c0a0097
                            urb MsgDesc: 9 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         g2<1>UW         g6<8,8,1>UD     0x08427005
                            sampler MsgDesc: ld SIMD8 Surface = 5 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g10<8,8,1>UD    0x08427006
                            sampler MsgDesc: ld SIMD8 Surface = 6 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g14<8,8,1>UD    0x08427007
                            sampler MsgDesc: ld SIMD8 Surface = 7 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g18<8,8,1>UD    0x08427008
                            sampler MsgDesc: ld SIMD8 Surface = 8 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g65<1>UW        g73<8,8,1>UD    0x10847005
                            sampler MsgDesc: ld SIMD16 Surface = 5 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(16)        g32<1>UW        g81<8,8,1>UD    0x10847006
                            sampler MsgDesc: ld SIMD16 Surface = 6 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(16)        g40<1>UW        g49<8,8,1>UD    0x10847007
                            sampler MsgDesc: ld SIMD16 Surface = 7 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(16)        g48<1>UW        g57<8,8,1>UD    0x10847008
                            sampler MsgDesc: ld SIMD16 Surface = 8 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x0a4b0000
                            sampler MsgDesc: gather4_c SIMD8 Surface = 0 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g6<8,8,1>UD     0x064a3001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g9<8,8,1>UD     0x064a3102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0a8c3001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0a8c3102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 5 rlen 8 { align1 1H };
send(8)         null<1>F        g123<8,8,1>F    0x8a080077
                            urb MsgDesc: 7 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         null<1>F        g30<8,8,1>UD    0x0c0a0067
                            urb MsgDesc: 6 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0c0a0077
                            urb MsgDesc: 7 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g42<8,8,1>UD    0x0c0a0087
                            urb MsgDesc: 8 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a4a8001
                            sampler MsgDesc: gather4 SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g7<8,8,1>UD     0x128c8001
                            sampler MsgDesc: gather4 SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0c840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x04420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06420304
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0c840304
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 3 mlen 6 rlen 8 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x04420304
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 3 mlen 2 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06420708
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 7 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840304
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 3 mlen 4 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0c840708
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 7 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x06421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x0c841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        g4<1>UD         g13<8,8,1>UD    0x02280301
                            const MsgDesc: (1, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x06427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g11<8,8,1>UD    0x06425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g14<8,8,1>UD    0x06425102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g21<8,8,1>UD    0x0c845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        g10<1>UW        g27<8,8,1>UD    0x0c845102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g10<8,8,1>UD    0x06422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g17<8,8,1>UD    0x0c842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g38<1>UD        g2<8,8,1>UD     0x024800c8
                            urb MsgDesc: 12 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g39<1>UD        g2<8,8,1>UD     0x024800d8
                            urb MsgDesc: 13 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g40<1>UD        g2<8,8,1>UD     0x024800e8
                            urb MsgDesc: 14 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g41<1>UD        g2<8,8,1>UD     0x024800f8
                            urb MsgDesc: 15 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g42<1>UD        g2<8,8,1>UD     0x02480108
                            urb MsgDesc: 16 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g43<1>UD        g2<8,8,1>UD     0x02480118
                            urb MsgDesc: 17 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g44<1>UD        g2<8,8,1>UD     0x02480128
                            urb MsgDesc: 18 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g45<1>UD        g2<8,8,1>UD     0x02480138
                            urb MsgDesc: 19 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g46<1>UD        g2<8,8,1>UD     0x02480148
                            urb MsgDesc: 20 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g47<1>UD        g2<8,8,1>UD     0x02480158
                            urb MsgDesc: 21 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g48<1>UD        g2<8,8,1>UD     0x02480168
                            urb MsgDesc: 22 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g49<1>UD        g2<8,8,1>UD     0x02480178
                            urb MsgDesc: 23 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g50<1>UD        g2<8,8,1>UD     0x02480188
                            urb MsgDesc: 24 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g51<1>UD        g2<8,8,1>UD     0x02480198
                            urb MsgDesc: 25 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g52<1>UD        g2<8,8,1>UD     0x024801a8
                            urb MsgDesc: 26 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g53<1>UD        g2<8,8,1>UD     0x024801b8
                            urb MsgDesc: 27 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g54<1>UD        g2<8,8,1>UD     0x024801c8
                            urb MsgDesc: 28 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g55<1>UD        g2<8,8,1>UD     0x024801d8
                            urb MsgDesc: 29 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g56<1>UD        g2<8,8,1>UD     0x024801e8
                            urb MsgDesc: 30 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g57<1>UD        g2<8,8,1>UD     0x024801f8
                            urb MsgDesc: 31 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c003e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 62, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c003d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 61, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0029
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 41, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c002a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 42, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c002b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 43, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c002c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 44, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c002d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 45, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c002e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 46, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c002f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 47, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0030
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 48, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0031
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 49, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0032
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 50, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0033
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 51, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0034
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 52, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0035
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 53, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0036
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 54, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0037
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 55, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0038
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 56, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c0039
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 57, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g2<1>UW         g0<8,8,1>F      0x021c003a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 58, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c003b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 59, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g2<1>UW         g0<8,8,1>F      0x021c003c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 60, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x080a8027
                            urb MsgDesc: 2 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g8<8,8,1>UD     0x0a0a8027
                            urb MsgDesc: 2 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x0e424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g8<1>UD         g14<8,8,1>UD    0x044a0128
                            urb MsgDesc: 18 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g22<1>UD        g16<8,8,1>UD    0x044a0028
                            urb MsgDesc: 2 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x0a080017
                            urb MsgDesc: 1 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g7<8,8,1>F      0x0a080057
                            urb MsgDesc: 5 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         g4<1>UW         g2<8,8,1>UD     0x02406001
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 1 rlen 4 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x0a026001
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
send(16)        g6<1>UW         g2<8,8,1>UD     0x04805001
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 2 rlen 8 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x14025001
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD16, Mask = 0x0) mlen 10 rlen 0 { align1 1H };
send(8)         g3<1>UW         g7<8,8,1>UD     0x08427002
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g3<1>UW         g11<8,8,1>UD    0x10847002
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g12<8,8,1>UD    0x084b0001
                            sampler MsgDesc: gather4_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x0e8d0001
                            sampler MsgDesc: gather4_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0e434102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 7 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g10<8,8,1>UD    0x064a8202
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x084a8101
                            sampler MsgDesc: gather4 SIMD8 Surface = 1 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g10<8,8,1>UD    0x08422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g15<8,8,1>UD    0x10842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         null<1>F        g122<8,8,1>F    0x8c0a0037
                            urb MsgDesc: 3 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         null<1>F        g10<8,8,1>F     0x12080027
                            urb MsgDesc: 2 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080047
                            urb MsgDesc: 4 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         g14<1>UW        g2<8,8,1>UD     0x06422000
                            sampler MsgDesc: sample_l SIMD8 Surface = 0 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g61<1>UD        g107<8,8,1>UD   0x02380048
                            urb MsgDesc: 4 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         g64<1>UD        g113<8,8,1>UD   0x02380058
                            urb MsgDesc: 5 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080047
                            urb MsgDesc: 4 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         null<1>UW       g2<8,8,1>UD     0x04009b00
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 0, SIMD8, imin) mlen 2 rlen 0 { align1 1Q };
send(8)         g5<1>UW         g4<8,8,1>UD     0x08495001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x08496001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 4 rlen 4 { align1 2Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080077
                            urb MsgDesc: 7 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         g12<1>UD        g8<4,4,1>UD     0x044a0038
                            urb MsgDesc: 3 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g21<1>UD        g8<4,4,1>UD     0x044a0048
                            urb MsgDesc: 4 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0c0a00a7
                            urb MsgDesc: 10 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g56<8,8,1>F     0x140a0097
                            urb MsgDesc: 9 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g76<8,8,1>F     0x0c0a00b7
                            urb MsgDesc: 11 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g122<8,8,1>F    0x8c0a00b7
                            urb MsgDesc: 11 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         null<1>F        g6<8,8,1>UD     0x0a080007
                            urb MsgDesc: 0 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         g2<1>UW         g9<8,8,1>UD     0x08423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g8<8,8,1>UD     0x10843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g2<1>UW         g11<8,8,1>UD    0x06426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g14<8,8,1>UD    0x06426102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g21<8,8,1>UD    0x0c846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        g10<1>UW        g27<8,8,1>UD    0x0c846102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g10<8,8,1>UD    0x0a421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g9<8,8,1>UD     0x14841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g4<1>UW         g0<8,8,1>UD     0x02201000
                            pixel interp MsgDesc: (persp, sample_position, 0x00) mlen 1 rlen 2 { align1 1Q };
send(16)        g6<1>UW         g0<8,8,1>UD     0x02411000
                            pixel interp MsgDesc: (persp, sample_position, 0x00) mlen 1 rlen 4 { align1 1H };
send(8)         g124<1>UW       g14<8,8,1>UD    0x0a4b0001
                            sampler MsgDesc: gather4_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x128d0001
                            sampler MsgDesc: gather4_c SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         null<1>F        g118<8,8,1>F    0x940a0037
                            urb MsgDesc: 3 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q EOT };
send(8)         g8<1>UD         g15<8,8,1>UD    0x042a0138
                            urb MsgDesc: 19 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g10<1>UD        g15<8,8,1>UD    0x042a0338
                            urb MsgDesc: 51 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g12<1>UD        g15<8,8,1>UD    0x042a0538
                            urb MsgDesc: 83 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g14<1>UD        g15<8,8,1>UD    0x042a0738
                            urb MsgDesc: 115 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g8<1>UD         g15<8,8,1>UD    0x042a0038
                            urb MsgDesc: 3 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g10<1>UD        g15<8,8,1>UD    0x042a0238
                            urb MsgDesc: 35 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g12<1>UD        g15<8,8,1>UD    0x042a0438
                            urb MsgDesc: 67 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g14<1>UD        g15<8,8,1>UD    0x042a0638
                            urb MsgDesc: 99 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g35<8,8,1>UD    0x02480228
                            urb MsgDesc: 34 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g8<1>UD         g35<8,8,1>UD    0x02480428
                            urb MsgDesc: 66 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g35<8,8,1>UD    0x02480628
                            urb MsgDesc: 98 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0a0a8037
                            urb MsgDesc: 3 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x0a0a8047
                            urb MsgDesc: 4 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0a0a8057
                            urb MsgDesc: 5 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x0a0a8067
                            urb MsgDesc: 6 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x0a0a8077
                            urb MsgDesc: 7 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x0a0a8087
                            urb MsgDesc: 8 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0a0a8097
                            urb MsgDesc: 9 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0a0a80a7
                            urb MsgDesc: 10 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0a0a80b7
                            urb MsgDesc: 11 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0a0a80c7
                            urb MsgDesc: 12 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0a0a80d7
                            urb MsgDesc: 13 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0a0a80e7
                            urb MsgDesc: 14 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0a0a80f7
                            urb MsgDesc: 15 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0a0a8107
                            urb MsgDesc: 16 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0a0a8117
                            urb MsgDesc: 17 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0a0a8127
                            urb MsgDesc: 18 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0a0a8137
                            urb MsgDesc: 19 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0a0a8147
                            urb MsgDesc: 20 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0a0a8157
                            urb MsgDesc: 21 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0a0a8167
                            urb MsgDesc: 22 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0a0a8177
                            urb MsgDesc: 23 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0a0a8187
                            urb MsgDesc: 24 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0a0a8197
                            urb MsgDesc: 25 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0a0a81a7
                            urb MsgDesc: 26 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0a0a81b7
                            urb MsgDesc: 27 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0a0a81c7
                            urb MsgDesc: 28 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0a0a81d7
                            urb MsgDesc: 29 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0a0a81e7
                            urb MsgDesc: 30 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0a0a81f7
                            urb MsgDesc: 31 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0a0a8207
                            urb MsgDesc: 32 SIMD8 write per-slot masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g122<8,8,1>F    0x8c0a0027
                            urb MsgDesc: 2 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         g2<1>UW         g2<8,8,1>UD     0x06429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g12<8,8,1>UD    0x0c849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g5<1>UW         g17<8,8,1>UD    0x06427102
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g21<1>UW        g7<8,8,1>UD     0x0c847102
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g118<8,8,1>F    0x940a0027
                            urb MsgDesc: 2 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q EOT };
send(8)         null<1>F        g2<8,8,1>F      0x12080067
                            urb MsgDesc: 6 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080087
                            urb MsgDesc: 8 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         g21<1>UD        g2<8,8,1>UD     0x02380068
                            urb MsgDesc: 6 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         g35<1>UD        g2<8,8,1>UD     0x02380088
                            urb MsgDesc: 8 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         null<1>F        g5<8,8,1>F      0x140a0067
                            urb MsgDesc: 6 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g118<8,8,1>F    0x940a0067
                            urb MsgDesc: 6 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q EOT };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g5<8,8,1>UD     0x08425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x10845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         null<1>F        g123<8,8,1>F    0x8a0800d7
                            urb MsgDesc: 13 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(8)         g22<1>UW        g14<8,8,1>UD    0x064a8405
                            sampler MsgDesc: gather4 SIMD8 Surface = 5 Sampler = 4 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x084a8102
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g10<8,8,1>UD    0x084a8203
                            sampler MsgDesc: gather4 SIMD8 Surface = 3 Sampler = 2 mlen 4 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g26<8,8,1>UD    0x0a4a8304
                            sampler MsgDesc: gather4 SIMD8 Surface = 4 Sampler = 3 mlen 5 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g43<8,8,1>UD    0x0a8c8405
                            sampler MsgDesc: gather4 SIMD16 Surface = 5 Sampler = 4 mlen 5 rlen 8 { align1 1H };
send(16)        g43<1>UW        g7<8,8,1>UD     0x0e8c8102
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(16)        g2<1>UW         g51<8,8,1>UD    0x0e8c8203
                            sampler MsgDesc: gather4 SIMD16 Surface = 3 Sampler = 2 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g26<8,8,1>UD    0x128c8304
                            sampler MsgDesc: gather4 SIMD16 Surface = 4 Sampler = 3 mlen 9 rlen 8 { align1 1H };
send(8)         g124<1>UW       g10<8,8,1>UD    0x0e4a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g10<8,8,1>UD    0x084a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g17<8,8,1>UD    0x0e8c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g124<1>UW       g9<8,8,1>UD     0x0a422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x14842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(16)        null<1>UW       g2<8,8,1>UD     0x04008601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, dec) mlen 2 rlen 0 { align1 1H };
send(8)         g2<1>UW         g7<8,8,1>UD     0x08426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x08426102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x10846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(16)        g10<1>UW        g19<8,8,1>UD    0x10846102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         null<1>F        g18<8,8,1>UD    0x0e0a8047
                            urb MsgDesc: 4 SIMD8 write per-slot masked mlen 7 rlen 0 { align1 1Q };
send(8)         g9<1>UD         g34<8,8,1>UD    0x02480218
                            urb MsgDesc: 33 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g17<1>UD        g34<8,8,1>UD    0x02480238
                            urb MsgDesc: 35 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g2<1>UD         g6<8,8,1>UD     0x041a0128
                            urb MsgDesc: 18 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g22<1>UD        g8<8,8,1>UD     0x041a0028
                            urb MsgDesc: 2 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         null<1>F        g2<8,8,1>UD     0x06088027
                            urb MsgDesc: 2 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x06088037
                            urb MsgDesc: 3 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x06088047
                            urb MsgDesc: 4 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x06088057
                            urb MsgDesc: 5 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x06088067
                            urb MsgDesc: 6 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x06088077
                            urb MsgDesc: 7 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x06088087
                            urb MsgDesc: 8 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x06088097
                            urb MsgDesc: 9 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x060880a7
                            urb MsgDesc: 10 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x060880b7
                            urb MsgDesc: 11 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x060880c7
                            urb MsgDesc: 12 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x060880d7
                            urb MsgDesc: 13 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x060880e7
                            urb MsgDesc: 14 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x060880f7
                            urb MsgDesc: 15 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x06088107
                            urb MsgDesc: 16 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x06088117
                            urb MsgDesc: 17 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x06088127
                            urb MsgDesc: 18 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x06088137
                            urb MsgDesc: 19 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x06088147
                            urb MsgDesc: 20 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x06088157
                            urb MsgDesc: 21 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x06088167
                            urb MsgDesc: 22 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x06088177
                            urb MsgDesc: 23 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x06088187
                            urb MsgDesc: 24 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x06088197
                            urb MsgDesc: 25 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x060881a7
                            urb MsgDesc: 26 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x060881b7
                            urb MsgDesc: 27 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x060881c7
                            urb MsgDesc: 28 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x060881d7
                            urb MsgDesc: 29 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x060881e7
                            urb MsgDesc: 30 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x060881f7
                            urb MsgDesc: 31 SIMD8 write masked mlen 3 rlen 0 { align1 1Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x02406000
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 0, SIMD8, Mask = 0x0) mlen 1 rlen 4 { align1 1Q };
send(8)         g23<1>UW        g11<8,8,1>UD    0x06195e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xe) mlen 3 rlen 1 { align1 1Q };
send(8)         null<1>UW       g24<8,8,1>UD    0x080b5e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1Q };
send(8)         g39<1>UW        g17<8,8,1>UD    0x06196e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xe) mlen 3 rlen 1 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x080b6e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0xe) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g11<8,8,1>UD    0x06098501
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, inc) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099501
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, inc) mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g24<8,8,1>UD    0x08098c01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, umax) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099c01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, umax) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g24<8,8,1>UD    0x08098401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, mov) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, mov) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g26<8,8,1>UD    0x0a098e01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, cmpwr) mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>UW       g14<8,8,1>UD    0x0a099e01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, cmpwr) mlen 5 rlen 0 { align1 2Q };
send(8)         g6<1>UW         g8<8,8,1>UD     0x04423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g18<8,8,1>UD    0x08843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(8)         g6<1>UD         g22<8,8,1>UD    0x044a0318
                            urb MsgDesc: 49 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g10<1>UD        g22<8,8,1>UD    0x044a0518
                            urb MsgDesc: 81 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g14<1>UD        g22<8,8,1>UD    0x044a0718
                            urb MsgDesc: 113 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g18<1>UD        g22<8,8,1>UD    0x044a0918
                            urb MsgDesc: 145 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g13<1>UD        g29<8,8,1>UD    0x044a0218
                            urb MsgDesc: 33 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g17<1>UD        g29<8,8,1>UD    0x044a0418
                            urb MsgDesc: 65 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g21<1>UD        g29<8,8,1>UD    0x044a0618
                            urb MsgDesc: 97 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g25<1>UD        g29<8,8,1>UD    0x044a0818
                            urb MsgDesc: 129 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0c0a0217
                            urb MsgDesc: 33 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0c0a0227
                            urb MsgDesc: 34 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x0c0a0237
                            urb MsgDesc: 35 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x0c0a0247
                            urb MsgDesc: 36 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x0c0a0257
                            urb MsgDesc: 37 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0c0a0267
                            urb MsgDesc: 38 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0c0a0277
                            urb MsgDesc: 39 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0c0a0287
                            urb MsgDesc: 40 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0c0a0297
                            urb MsgDesc: 41 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0c0a02a7
                            urb MsgDesc: 42 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0c0a02b7
                            urb MsgDesc: 43 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0c0a02c7
                            urb MsgDesc: 44 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0c0a02d7
                            urb MsgDesc: 45 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0c0a02e7
                            urb MsgDesc: 46 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0c0a02f7
                            urb MsgDesc: 47 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0c0a0307
                            urb MsgDesc: 48 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0c0a0317
                            urb MsgDesc: 49 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0c0a0327
                            urb MsgDesc: 50 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0c0a0337
                            urb MsgDesc: 51 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0c0a0347
                            urb MsgDesc: 52 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0c0a0357
                            urb MsgDesc: 53 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0c0a0367
                            urb MsgDesc: 54 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0c0a0377
                            urb MsgDesc: 55 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0c0a0387
                            urb MsgDesc: 56 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0c0a0397
                            urb MsgDesc: 57 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0c0a03a7
                            urb MsgDesc: 58 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0c0a03b7
                            urb MsgDesc: 59 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0c0a03c7
                            urb MsgDesc: 60 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0c0a03d7
                            urb MsgDesc: 61 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x0c0a03e7
                            urb MsgDesc: 62 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g41<8,8,1>UD    0x0c0a03f7
                            urb MsgDesc: 63 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0a080067
                            urb MsgDesc: 6 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0a080077
                            urb MsgDesc: 7 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0a080087
                            urb MsgDesc: 8 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0a080097
                            urb MsgDesc: 9 SIMD8 write mlen 5 rlen 0        { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0a0800a7
                            urb MsgDesc: 10 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0a0800b7
                            urb MsgDesc: 11 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0a0800c7
                            urb MsgDesc: 12 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0a0800d7
                            urb MsgDesc: 13 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0a0800e7
                            urb MsgDesc: 14 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0a0800f7
                            urb MsgDesc: 15 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0a080107
                            urb MsgDesc: 16 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0a080117
                            urb MsgDesc: 17 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0a080127
                            urb MsgDesc: 18 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0a080137
                            urb MsgDesc: 19 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0a080147
                            urb MsgDesc: 20 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0a080157
                            urb MsgDesc: 21 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0a080167
                            urb MsgDesc: 22 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0a080177
                            urb MsgDesc: 23 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0a080187
                            urb MsgDesc: 24 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0a080197
                            urb MsgDesc: 25 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0a0801a7
                            urb MsgDesc: 26 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0a0801b7
                            urb MsgDesc: 27 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0a0801c7
                            urb MsgDesc: 28 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0a0801d7
                            urb MsgDesc: 29 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x0a0801e7
                            urb MsgDesc: 30 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         null<1>F        g41<8,8,1>UD    0x0a0801f7
                            urb MsgDesc: 31 SIMD8 write mlen 5 rlen 0       { align1 1Q };
send(8)         g124<1>UW       g5<8,8,1>UD     0x064a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x0a8c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g26<8,8,1>UD    0x0c843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g9<8,8,1>UD     0x08420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x10840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g5<1>UW         g15<8,8,1>UD    0x04420203
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 2 mlen 2 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g27<8,8,1>UD    0x08840203
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 2 mlen 4 rlen 8 { align1 1H };
send(16)        g4<1>UW         g17<8,8,1>UD    0x0420a503
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 3, SIMD16, inc) mlen 2 rlen 2 { align1 1H };
send(16)        null<1>UW       g18<8,8,1>UD    0x04008504
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 4, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(16)        g11<1>UW        g19<8,8,1>UD    0x0420a602
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, dec) mlen 2 rlen 2 { align1 1H };
send(16)        null<1>UW       g20<8,8,1>UD    0x04008505
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 5, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(16)        g16<1>UW        g21<8,8,1>UD    0x04205e01
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
send(16)        null<1>UW       g22<8,8,1>UD    0x04008506
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 6, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(8)         g26<1>UW        g26<8,8,1>UD    0x0242a203
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g30<1>UW        g30<8,8,1>UD    0x0242a304
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g34<1>UW        g34<8,8,1>UD    0x0242a405
                            sampler MsgDesc: resinfo SIMD8 Surface = 5 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g38<8,8,1>UD    0x0242a506
                            sampler MsgDesc: resinfo SIMD8 Surface = 6 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g25<8,8,1>UD    0x0242a102
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g42<1>UW        g42<8,8,1>UD    0x0242a607
                            sampler MsgDesc: resinfo SIMD8 Surface = 7 Sampler = 6 mlen 1 rlen 4 { align1 1Q };
send(8)         g46<1>UW        g46<8,8,1>UD    0x0242a708
                            sampler MsgDesc: resinfo SIMD8 Surface = 8 Sampler = 7 mlen 1 rlen 4 { align1 1Q };
send(8)         g50<1>UW        g50<8,8,1>UD    0x0242a809
                            sampler MsgDesc: resinfo SIMD8 Surface = 9 Sampler = 8 mlen 1 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g54<8,8,1>UD    0x0242a90a
                            sampler MsgDesc: resinfo SIMD8 Surface = 10 Sampler = 9 mlen 1 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g55<8,8,1>UD    0x0242aa0b
                            sampler MsgDesc: resinfo SIMD8 Surface = 11 Sampler = 10 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g56<8,8,1>UD    0x0242ab0c
                            sampler MsgDesc: resinfo SIMD8 Surface = 12 Sampler = 11 mlen 1 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g57<8,8,1>UD    0x0242ac0d
                            sampler MsgDesc: resinfo SIMD8 Surface = 13 Sampler = 12 mlen 1 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0484a102
                            sampler MsgDesc: resinfo SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 1H };
send(16)        g82<1>UW        g110<8,8,1>UD   0x0484aa0b
                            sampler MsgDesc: resinfo SIMD16 Surface = 11 Sampler = 10 mlen 2 rlen 8 { align1 1H };
send(16)        g18<1>UW        g26<8,8,1>UD    0x0484a203
                            sampler MsgDesc: resinfo SIMD16 Surface = 3 Sampler = 2 mlen 2 rlen 8 { align1 1H };
send(16)        g90<1>UW        g112<8,8,1>UD   0x0484ab0c
                            sampler MsgDesc: resinfo SIMD16 Surface = 12 Sampler = 11 mlen 2 rlen 8 { align1 1H };
send(16)        g98<1>UW        g106<8,8,1>UD   0x0484ac0d
                            sampler MsgDesc: resinfo SIMD16 Surface = 13 Sampler = 12 mlen 2 rlen 8 { align1 1H };
send(16)        g26<1>UW        g34<8,8,1>UD    0x0484a304
                            sampler MsgDesc: resinfo SIMD16 Surface = 4 Sampler = 3 mlen 2 rlen 8 { align1 1H };
send(16)        g34<1>UW        g42<8,8,1>UD    0x0484a405
                            sampler MsgDesc: resinfo SIMD16 Surface = 5 Sampler = 4 mlen 2 rlen 8 { align1 1H };
send(16)        g42<1>UW        g50<8,8,1>UD    0x0484a506
                            sampler MsgDesc: resinfo SIMD16 Surface = 6 Sampler = 5 mlen 2 rlen 8 { align1 1H };
send(16)        g50<1>UW        g58<8,8,1>UD    0x0484a607
                            sampler MsgDesc: resinfo SIMD16 Surface = 7 Sampler = 6 mlen 2 rlen 8 { align1 1H };
send(16)        g58<1>UW        g66<8,8,1>UD    0x0484a708
                            sampler MsgDesc: resinfo SIMD16 Surface = 8 Sampler = 7 mlen 2 rlen 8 { align1 1H };
send(16)        g66<1>UW        g74<8,8,1>UD    0x0484a809
                            sampler MsgDesc: resinfo SIMD16 Surface = 9 Sampler = 8 mlen 2 rlen 8 { align1 1H };
send(16)        g74<1>UW        g108<8,8,1>UD   0x0484a90a
                            sampler MsgDesc: resinfo SIMD16 Surface = 10 Sampler = 9 mlen 2 rlen 8 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x040085fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x08008dfe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, umin) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g5<8,8,1>UD     0x08008bfe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, imin) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x08008cfe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, umax) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g5<8,8,1>UD     0x08008afe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, imax) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x080081fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, and) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x080082fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, or) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x080083fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, xor) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x080084fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, mov) mlen 4 rlen 0 { align1 1H };
send(16)        null<1>UW       g1<8,8,1>UD     0x0c008efe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, cmpwr) mlen 6 rlen 0 { align1 1H };
send(8)         null<1>F        g119<8,8,1>F    0x92080067
                            urb MsgDesc: 6 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         g124<1>UW       g11<8,8,1>UD    0x12424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 9 rlen 4 { align1 1Q };
send(16)        g1<1>UW         g14<8,8,1>UD    0x0820a4fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, mov) mlen 4 rlen 2 { align1 1H };
send(16)        g13<1>UW        g17<8,8,1>UD    0x0820a2fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, or) mlen 4 rlen 2 { align1 1H };
send(16)        null<1>UW       g123<8,8,1>UD   0x060a03fd
                            data MsgDesc: ( DC OWORD block write, 253, 3) mlen 3 rlen 0 { align1 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1000
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 0, 16) mlen 1 rlen 2           { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1002
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 2, 16) mlen 1 rlen 2           { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1012
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 18, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1014
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 20, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1004
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 4, 16) mlen 1 rlen 2           { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1006
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 6, 16) mlen 1 rlen 2           { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1016
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 22, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1018
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 24, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1008
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 8, 16) mlen 1 rlen 2           { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c100a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 10, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c101a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 26, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c101c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 28, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c100c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 12, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c100e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 14, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c101e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 30, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1022
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 34, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1010
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 16, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1020
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 32, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1024
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 36, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c102a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 42, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1026
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 38, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1028
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 40, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c102c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 44, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1032
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 50, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c102e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 46, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1030
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 48, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1034
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 52, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c103a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 58, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1036
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 54, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1038
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 56, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c103c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 60, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1042
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 66, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c103e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 62, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1040
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 64, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1044
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 68, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c104a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 74, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1046
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 70, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1048
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 72, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c104c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 76, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1052
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 82, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c104e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 78, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1050
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 80, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1054
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 84, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c105a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 90, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1056
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 86, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1058
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 88, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c105c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 92, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1062
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 98, 16) mlen 1 rlen 2          { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c105e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 94, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1060
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 96, 16) mlen 1 rlen 2          { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1064
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 100, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c106a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 106, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1066
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 102, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1068
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 104, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c106c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 108, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1072
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 114, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c106e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 110, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1070
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 112, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1074
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 116, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c107a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 122, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1076
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 118, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1078
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 120, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c107c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 124, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1082
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 130, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c107e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 126, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1080
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 128, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1084
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 132, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c108a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 138, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c1086
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 134, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c1088
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 136, 16) mlen 1 rlen 2         { align1 WE_all 2H };
send(16)        g4<1>UW         g0<8,8,1>F      0x022c108c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 140, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(8)         null<1>F        g12<8,8,1>UD    0x0c0a0127
                            urb MsgDesc: 18 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g11<8,8,1>UD    0x04420405
                            sampler MsgDesc: sample SIMD8 Surface = 5 Sampler = 4 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x04420506
                            sampler MsgDesc: sample SIMD8 Surface = 6 Sampler = 5 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x04420607
                            sampler MsgDesc: sample SIMD8 Surface = 7 Sampler = 6 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g14<8,8,1>UD    0x04420708
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 7 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g15<8,8,1>UD    0x04420809
                            sampler MsgDesc: sample SIMD8 Surface = 9 Sampler = 8 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g16<8,8,1>UD    0x0442090a
                            sampler MsgDesc: sample SIMD8 Surface = 10 Sampler = 9 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g17<8,8,1>UD    0x04420a0b
                            sampler MsgDesc: sample SIMD8 Surface = 11 Sampler = 10 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g18<8,8,1>UD    0x04420b0c
                            sampler MsgDesc: sample SIMD8 Surface = 12 Sampler = 11 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g19<8,8,1>UD    0x04420c0d
                            sampler MsgDesc: sample SIMD8 Surface = 13 Sampler = 12 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g20<8,8,1>UD    0x04420d0e
                            sampler MsgDesc: sample SIMD8 Surface = 14 Sampler = 13 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g21<8,8,1>UD    0x04420e0f
                            sampler MsgDesc: sample SIMD8 Surface = 15 Sampler = 14 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g22<8,8,1>UD    0x04420f10
                            sampler MsgDesc: sample SIMD8 Surface = 16 Sampler = 15 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0011
                            sampler MsgDesc: sample SIMD8 Surface = 17 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0112
                            sampler MsgDesc: sample SIMD8 Surface = 18 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0213
                            sampler MsgDesc: sample SIMD8 Surface = 19 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0314
                            sampler MsgDesc: sample SIMD8 Surface = 20 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0415
                            sampler MsgDesc: sample SIMD8 Surface = 21 Sampler = 4 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0516
                            sampler MsgDesc: sample SIMD8 Surface = 22 Sampler = 5 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0617
                            sampler MsgDesc: sample SIMD8 Surface = 23 Sampler = 6 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0718
                            sampler MsgDesc: sample SIMD8 Surface = 24 Sampler = 7 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0819
                            sampler MsgDesc: sample SIMD8 Surface = 25 Sampler = 8 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a091a
                            sampler MsgDesc: sample SIMD8 Surface = 26 Sampler = 9 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0a1b
                            sampler MsgDesc: sample SIMD8 Surface = 27 Sampler = 10 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0b1c
                            sampler MsgDesc: sample SIMD8 Surface = 28 Sampler = 11 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0c1d
                            sampler MsgDesc: sample SIMD8 Surface = 29 Sampler = 12 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0d1e
                            sampler MsgDesc: sample SIMD8 Surface = 30 Sampler = 13 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x064a0e1f
                            sampler MsgDesc: sample SIMD8 Surface = 31 Sampler = 14 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g13<8,8,1>UD    0x064a0f20
                            sampler MsgDesc: sample SIMD8 Surface = 32 Sampler = 15 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g28<8,8,1>UD    0x08840405
                            sampler MsgDesc: sample SIMD16 Surface = 5 Sampler = 4 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g29<8,8,1>UD    0x08840506
                            sampler MsgDesc: sample SIMD16 Surface = 6 Sampler = 5 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g30<8,8,1>UD    0x08840607
                            sampler MsgDesc: sample SIMD16 Surface = 7 Sampler = 6 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g31<8,8,1>UD    0x08840708
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 7 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g32<8,8,1>UD    0x08840809
                            sampler MsgDesc: sample SIMD16 Surface = 9 Sampler = 8 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g33<8,8,1>UD    0x0884090a
                            sampler MsgDesc: sample SIMD16 Surface = 10 Sampler = 9 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g34<8,8,1>UD    0x08840a0b
                            sampler MsgDesc: sample SIMD16 Surface = 11 Sampler = 10 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g35<8,8,1>UD    0x08840b0c
                            sampler MsgDesc: sample SIMD16 Surface = 12 Sampler = 11 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g36<8,8,1>UD    0x08840c0d
                            sampler MsgDesc: sample SIMD16 Surface = 13 Sampler = 12 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g37<8,8,1>UD    0x08840d0e
                            sampler MsgDesc: sample SIMD16 Surface = 14 Sampler = 13 mlen 4 rlen 8 { align1 1H };
send(16)        g7<1>UW         g38<8,8,1>UD    0x08840e0f
                            sampler MsgDesc: sample SIMD16 Surface = 15 Sampler = 14 mlen 4 rlen 8 { align1 1H };
send(16)        g23<1>UW        g39<8,8,1>UD    0x08840f10
                            sampler MsgDesc: sample SIMD16 Surface = 16 Sampler = 15 mlen 4 rlen 8 { align1 1H };
send(16)        g17<1>UW        g2<8,8,1>UD     0x0a8c0011
                            sampler MsgDesc: sample SIMD16 Surface = 17 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g29<1>UW        g7<8,8,1>UD     0x0a8c0112
                            sampler MsgDesc: sample SIMD16 Surface = 18 Sampler = 1 mlen 5 rlen 8 { align1 1H };
send(16)        g27<1>UW        g12<8,8,1>UD    0x0a8c0213
                            sampler MsgDesc: sample SIMD16 Surface = 19 Sampler = 2 mlen 5 rlen 8 { align1 1H };
send(16)        g32<1>UW        g17<8,8,1>UD    0x0a8c0314
                            sampler MsgDesc: sample SIMD16 Surface = 20 Sampler = 3 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g22<8,8,1>UD    0x0a8c0415
                            sampler MsgDesc: sample SIMD16 Surface = 21 Sampler = 4 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g27<8,8,1>UD    0x0a8c0516
                            sampler MsgDesc: sample SIMD16 Surface = 22 Sampler = 5 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g32<8,8,1>UD    0x0a8c0617
                            sampler MsgDesc: sample SIMD16 Surface = 23 Sampler = 6 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g37<8,8,1>UD    0x0a8c0718
                            sampler MsgDesc: sample SIMD16 Surface = 24 Sampler = 7 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g42<8,8,1>UD    0x0a8c0819
                            sampler MsgDesc: sample SIMD16 Surface = 25 Sampler = 8 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g47<8,8,1>UD    0x0a8c091a
                            sampler MsgDesc: sample SIMD16 Surface = 26 Sampler = 9 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g52<8,8,1>UD    0x0a8c0a1b
                            sampler MsgDesc: sample SIMD16 Surface = 27 Sampler = 10 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g57<8,8,1>UD    0x0a8c0b1c
                            sampler MsgDesc: sample SIMD16 Surface = 28 Sampler = 11 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g62<8,8,1>UD    0x0a8c0c1d
                            sampler MsgDesc: sample SIMD16 Surface = 29 Sampler = 12 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g67<8,8,1>UD    0x0a8c0d1e
                            sampler MsgDesc: sample SIMD16 Surface = 30 Sampler = 13 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g72<8,8,1>UD    0x0a8c0e1f
                            sampler MsgDesc: sample SIMD16 Surface = 31 Sampler = 14 mlen 5 rlen 8 { align1 1H };
send(16)        g2<1>UW         g77<8,8,1>UD    0x0a8c0f20
                            sampler MsgDesc: sample SIMD16 Surface = 32 Sampler = 15 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g2<8,8,1>UD     0x02420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g2<8,8,1>UD     0x04840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 1H };
send(8)         null<1>UW       g10<8,8,1>UD    0x0a026000
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 0, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g8<8,8,1>UD     0x0c0a8027
                            urb MsgDesc: 2 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>F     0x12080047
                            urb MsgDesc: 4 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080087
                            urb MsgDesc: 8 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         g2<1>UW         g74<8,8,1>UD    0x02106e02
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(8)         null<1>UW       g1<8,8,1>UD     0x04026efe
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 254, SIMD8, Mask = 0xe) mlen 2 rlen 0 { align1 1Q };
send(8)         g67<1>UW        g2<8,8,1>UD     0x0410bdfe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD8, umin) mlen 2 rlen 1 { align1 1Q };
send(8)         g68<1>UW        g2<8,8,1>UD     0x02106efe
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 254, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(8)         g73<1>UW        g4<8,8,1>UD     0x0410b4fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD8, mov) mlen 2 rlen 1 { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080097
                            urb MsgDesc: 9 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         g29<1>UW        g5<8,8,1>UD     0x0e4b2001
                            sampler MsgDesc: gather4_po_c SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x084a2001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x0e8c2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g2<1>UW         g5<8,8,1>UD     0x0a426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g10<8,8,1>UD    0x0a426102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g15<8,8,1>UD    0x14846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(16)        g10<1>UW        g25<8,8,1>UD    0x14846102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
send(8)         null<1>F        g14<8,8,1>UD    0x0c0a8037
                            urb MsgDesc: 3 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x0c0a8047
                            urb MsgDesc: 4 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0c0a8057
                            urb MsgDesc: 5 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x084a5001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x084a5102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x0e8c5001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c5102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(16)        g1<1>UW         g7<8,8,1>UD     0x0820a7fe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD16, add) mlen 4 rlen 2 { align1 1H };
send(8)         g2<1>UW         g6<8,8,1>UD     0x084a3001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g10<8,8,1>UD    0x084a3102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0e8c3001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c3102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(8)         g124<1>UW       g3<8,8,1>UD     0x044a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g4<8,8,1>UD     0x068c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 3 rlen 8 { align1 1H };
send(8)         g17<1>UW        g12<8,8,1>UD    0x04420003
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g39<8,8,1>UD    0x08840003
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g11<1>UW        g39<8,8,1>UD    0x06427008
                            sampler MsgDesc: ld SIMD8 Surface = 8 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g15<1>UW        g39<8,8,1>UD    0x06427109
                            sampler MsgDesc: ld SIMD8 Surface = 9 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(8)         g19<1>UW        g39<8,8,1>UD    0x0642720a
                            sampler MsgDesc: ld SIMD8 Surface = 10 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(8)         g23<1>UW        g39<8,8,1>UD    0x0642730b
                            sampler MsgDesc: ld SIMD8 Surface = 11 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
send(8)         g27<1>UW        g39<8,8,1>UD    0x0642740c
                            sampler MsgDesc: ld SIMD8 Surface = 12 Sampler = 4 mlen 3 rlen 4 { align1 1Q };
send(8)         g31<1>UW        g39<8,8,1>UD    0x0642750d
                            sampler MsgDesc: ld SIMD8 Surface = 13 Sampler = 5 mlen 3 rlen 4 { align1 1Q };
send(8)         g35<1>UW        g39<8,8,1>UD    0x0642760e
                            sampler MsgDesc: ld SIMD8 Surface = 14 Sampler = 6 mlen 3 rlen 4 { align1 1Q };
send(8)         g39<1>UW        g39<8,8,1>UD    0x0642770f
                            sampler MsgDesc: ld SIMD8 Surface = 15 Sampler = 7 mlen 3 rlen 4 { align1 1Q };
send(16)        g67<1>UW        g93<8,8,1>UD    0x0c847008
                            sampler MsgDesc: ld SIMD16 Surface = 8 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        g27<1>UW        g93<8,8,1>UD    0x0c847109
                            sampler MsgDesc: ld SIMD16 Surface = 9 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(16)        g37<1>UW        g93<8,8,1>UD    0x0c84720a
                            sampler MsgDesc: ld SIMD16 Surface = 10 Sampler = 2 mlen 6 rlen 8 { align1 1H };
send(16)        g47<1>UW        g93<8,8,1>UD    0x0c84730b
                            sampler MsgDesc: ld SIMD16 Surface = 11 Sampler = 3 mlen 6 rlen 8 { align1 1H };
send(16)        g57<1>UW        g93<8,8,1>UD    0x0c84740c
                            sampler MsgDesc: ld SIMD16 Surface = 12 Sampler = 4 mlen 6 rlen 8 { align1 1H };
send(16)        g17<1>UW        g93<8,8,1>UD    0x0c84750d
                            sampler MsgDesc: ld SIMD16 Surface = 13 Sampler = 5 mlen 6 rlen 8 { align1 1H };
send(16)        g85<1>UW        g93<8,8,1>UD    0x0c84760e
                            sampler MsgDesc: ld SIMD16 Surface = 14 Sampler = 6 mlen 6 rlen 8 { align1 1H };
send(16)        g77<1>UW        g93<8,8,1>UD    0x0c84770f
                            sampler MsgDesc: ld SIMD16 Surface = 15 Sampler = 7 mlen 6 rlen 8 { align1 1H };
send(16)        g81<1>UW        g84<8,8,1>UD    0x04205e00
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 0, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
send(8)         null<1>F        g122<8,8,1>F    0x8c0a0047
                            urb MsgDesc: 4 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         g124<1>UW       g8<8,8,1>UD     0x064a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g15<8,8,1>UD    0x0a8c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g14<1>UW        g11<8,8,1>UD    0x084b0202
                            sampler MsgDesc: gather4_c SIMD8 Surface = 2 Sampler = 2 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x0a4b0101
                            sampler MsgDesc: gather4_c SIMD8 Surface = 1 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(8)         null<1>F        g3<8,8,1>F      0x12080087
                            urb MsgDesc: 8 SIMD8 write mlen 9 rlen 0        { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a0800a7
                            urb MsgDesc: 10 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(8)         g2<1>UW         g7<8,8,1>UD     0x084a6001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x084a6102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x0e8c6001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c6102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(8)         g31<1>UD        g28<8,8,1>UD    0x02380238
                            urb MsgDesc: 35 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g34<1>UD        g28<8,8,1>UD    0x02380438
                            urb MsgDesc: 67 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g37<1>UD        g28<8,8,1>UD    0x02380638
                            urb MsgDesc: 99 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g28<8,8,1>UD    0x02380248
                            urb MsgDesc: 36 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g28<8,8,1>UD    0x02380448
                            urb MsgDesc: 68 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g28<1>UD        g28<8,8,1>UD    0x02380648
                            urb MsgDesc: 100 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g22<1>UD        g29<8,8,1>UD    0x02380258
                            urb MsgDesc: 37 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g29<8,8,1>UD    0x02380458
                            urb MsgDesc: 69 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g28<1>UD        g29<8,8,1>UD    0x02380658
                            urb MsgDesc: 101 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g22<1>UD        g30<8,8,1>UD    0x02380268
                            urb MsgDesc: 38 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g30<8,8,1>UD    0x02380468
                            urb MsgDesc: 70 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g28<1>UD        g30<8,8,1>UD    0x02380668
                            urb MsgDesc: 102 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g22<1>UD        g31<8,8,1>UD    0x02380278
                            urb MsgDesc: 39 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g31<8,8,1>UD    0x02380478
                            urb MsgDesc: 71 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g28<1>UD        g31<8,8,1>UD    0x02380678
                            urb MsgDesc: 103 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g25<1>UD        g32<8,8,1>UD    0x02380488
                            urb MsgDesc: 72 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g32<8,8,1>UD    0x02380288
                            urb MsgDesc: 40 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g28<1>UD        g32<8,8,1>UD    0x02380688
                            urb MsgDesc: 104 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g25<1>UD        g33<8,8,1>UD    0x02380498
                            urb MsgDesc: 73 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g33<8,8,1>UD    0x02380298
                            urb MsgDesc: 41 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g28<1>UD        g33<8,8,1>UD    0x02380698
                            urb MsgDesc: 105 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g25<1>UD        g34<8,8,1>UD    0x023806a8
                            urb MsgDesc: 106 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g34<8,8,1>UD    0x023802a8
                            urb MsgDesc: 42 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g34<8,8,1>UD    0x023804a8
                            urb MsgDesc: 74 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g8<1>UD         g35<8,8,1>UD    0x023802b8
                            urb MsgDesc: 43 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g35<8,8,1>UD    0x023804b8
                            urb MsgDesc: 75 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g35<8,8,1>UD    0x023806b8
                            urb MsgDesc: 107 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g36<8,8,1>UD    0x023802c8
                            urb MsgDesc: 44 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g36<8,8,1>UD    0x023804c8
                            urb MsgDesc: 76 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g36<8,8,1>UD    0x023806c8
                            urb MsgDesc: 108 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g37<8,8,1>UD    0x023802d8
                            urb MsgDesc: 45 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g37<8,8,1>UD    0x023804d8
                            urb MsgDesc: 77 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g37<8,8,1>UD    0x023806d8
                            urb MsgDesc: 109 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g38<8,8,1>UD    0x023802e8
                            urb MsgDesc: 46 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g38<8,8,1>UD    0x023804e8
                            urb MsgDesc: 78 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g38<8,8,1>UD    0x023806e8
                            urb MsgDesc: 110 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g39<8,8,1>UD    0x023802f8
                            urb MsgDesc: 47 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g39<8,8,1>UD    0x023804f8
                            urb MsgDesc: 79 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g25<1>UD        g39<8,8,1>UD    0x023806f8
                            urb MsgDesc: 111 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g40<8,8,1>UD    0x02380308
                            urb MsgDesc: 48 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g40<8,8,1>UD    0x02380508
                            urb MsgDesc: 80 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g40<8,8,1>UD    0x02380708
                            urb MsgDesc: 112 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g41<8,8,1>UD    0x02380318
                            urb MsgDesc: 49 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g41<8,8,1>UD    0x02380518
                            urb MsgDesc: 81 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g41<8,8,1>UD    0x02380718
                            urb MsgDesc: 113 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g3<8,8,1>UD     0x02380328
                            urb MsgDesc: 50 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g3<8,8,1>UD     0x02380528
                            urb MsgDesc: 82 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g3<8,8,1>UD     0x02380728
                            urb MsgDesc: 114 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g43<8,8,1>UD    0x02380338
                            urb MsgDesc: 51 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g43<8,8,1>UD    0x02380538
                            urb MsgDesc: 83 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g43<8,8,1>UD    0x02380738
                            urb MsgDesc: 115 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g44<8,8,1>UD    0x02380348
                            urb MsgDesc: 52 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g44<8,8,1>UD    0x02380548
                            urb MsgDesc: 84 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g44<8,8,1>UD    0x02380748
                            urb MsgDesc: 116 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g45<8,8,1>UD    0x02380358
                            urb MsgDesc: 53 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g45<8,8,1>UD    0x02380558
                            urb MsgDesc: 85 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g22<1>UD        g45<8,8,1>UD    0x02380758
                            urb MsgDesc: 117 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g46<8,8,1>UD    0x02380368
                            urb MsgDesc: 54 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g46<8,8,1>UD    0x02380568
                            urb MsgDesc: 86 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g46<8,8,1>UD    0x02380768
                            urb MsgDesc: 118 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g47<8,8,1>UD    0x02380378
                            urb MsgDesc: 55 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g47<8,8,1>UD    0x02380578
                            urb MsgDesc: 87 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g47<8,8,1>UD    0x02380778
                            urb MsgDesc: 119 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g48<8,8,1>UD    0x02380388
                            urb MsgDesc: 56 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g48<8,8,1>UD    0x02380588
                            urb MsgDesc: 88 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g48<8,8,1>UD    0x02380788
                            urb MsgDesc: 120 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g49<8,8,1>UD    0x02380398
                            urb MsgDesc: 57 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g49<8,8,1>UD    0x02380598
                            urb MsgDesc: 89 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g49<8,8,1>UD    0x02380798
                            urb MsgDesc: 121 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g50<8,8,1>UD    0x023803a8
                            urb MsgDesc: 58 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g50<8,8,1>UD    0x023805a8
                            urb MsgDesc: 90 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g50<8,8,1>UD    0x023807a8
                            urb MsgDesc: 122 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g54<8,8,1>UD    0x023803b8
                            urb MsgDesc: 59 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g54<8,8,1>UD    0x023805b8
                            urb MsgDesc: 91 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g54<8,8,1>UD    0x023807b8
                            urb MsgDesc: 123 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g55<8,8,1>UD    0x023803c8
                            urb MsgDesc: 60 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g55<8,8,1>UD    0x023805c8
                            urb MsgDesc: 92 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g55<8,8,1>UD    0x023807c8
                            urb MsgDesc: 124 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g56<8,8,1>UD    0x023803d8
                            urb MsgDesc: 61 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g56<8,8,1>UD    0x023805d8
                            urb MsgDesc: 93 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g56<8,8,1>UD    0x023807d8
                            urb MsgDesc: 125 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g57<8,8,1>UD    0x023803e8
                            urb MsgDesc: 62 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g57<8,8,1>UD    0x023805e8
                            urb MsgDesc: 94 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g57<8,8,1>UD    0x023807e8
                            urb MsgDesc: 126 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g58<8,8,1>UD    0x023803f8
                            urb MsgDesc: 63 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g58<8,8,1>UD    0x023805f8
                            urb MsgDesc: 95 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g58<8,8,1>UD    0x023807f8
                            urb MsgDesc: 127 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g59<8,8,1>UD    0x02380208
                            urb MsgDesc: 32 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g59<8,8,1>UD    0x02380408
                            urb MsgDesc: 64 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g59<8,8,1>UD    0x02380608
                            urb MsgDesc: 96 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g17<1>UD        g59<8,8,1>UD    0x02380808
                            urb MsgDesc: 128 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         g8<1>UD         g60<8,8,1>UD    0x02380218
                            urb MsgDesc: 33 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g60<8,8,1>UD    0x02380418
                            urb MsgDesc: 65 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g14<1>UD        g60<8,8,1>UD    0x02380618
                            urb MsgDesc: 97 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g17<1>UD        g60<8,8,1>UD    0x02380818
                            urb MsgDesc: 129 SIMD8 read mlen 1 rlen 3       { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x0c0a8067
                            urb MsgDesc: 6 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x0c0a8077
                            urb MsgDesc: 7 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0c0a8087
                            urb MsgDesc: 8 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0c0a8097
                            urb MsgDesc: 9 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0c0a80a7
                            urb MsgDesc: 10 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0c0a80b7
                            urb MsgDesc: 11 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0c0a80c7
                            urb MsgDesc: 12 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0c0a80d7
                            urb MsgDesc: 13 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0c0a80e7
                            urb MsgDesc: 14 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0c0a80f7
                            urb MsgDesc: 15 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0c0a8107
                            urb MsgDesc: 16 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0c0a8117
                            urb MsgDesc: 17 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0c0a8127
                            urb MsgDesc: 18 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0c0a8137
                            urb MsgDesc: 19 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0c0a8147
                            urb MsgDesc: 20 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0c0a8157
                            urb MsgDesc: 21 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0c0a8167
                            urb MsgDesc: 22 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0c0a8177
                            urb MsgDesc: 23 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0c0a8187
                            urb MsgDesc: 24 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0c0a8197
                            urb MsgDesc: 25 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0c0a81a7
                            urb MsgDesc: 26 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0c0a81b7
                            urb MsgDesc: 27 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0c0a81c7
                            urb MsgDesc: 28 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0c0a81d7
                            urb MsgDesc: 29 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0c0a81e7
                            urb MsgDesc: 30 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0c0a81f7
                            urb MsgDesc: 31 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x0c0a8207
                            urb MsgDesc: 32 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g41<8,8,1>UD    0x0c0a8217
                            urb MsgDesc: 33 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(1)         g1<1>UW         g1<0,1,0>UW     0x0209c000
                            data MsgDesc: ( DC mfence, 0, 0) mlen 1 rlen 0  { align1 WE_all 1N };
send(8)         g124<1>UW       g2<8,8,1>UD     0x02106e01
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(16)        g11<1>UW        g19<8,8,1>UD    0x0420a601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, dec) mlen 2 rlen 2 { align1 1H };
send(16)        null<1>UW       g20<8,8,1>UD    0x04008503
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 3, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(8)         null<1>F        g122<8,8,1>UD   0x8c088007
                            urb MsgDesc: 0 SIMD8 write masked mlen 6 rlen 0 { align1 1Q EOT };
(+f1.0) send(8) null<1>UW       g6<8,8,1>UD     0x04026e01
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD8, Mask = 0xe) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g6<8,8,1>UD     0x04026e02
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD8, Mask = 0xe) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g8<8,8,1>UD     0x08025e01
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g8<8,8,1>UD     0x08025e02
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1H };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a4a5001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0a4a5102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g25<1>UW        g7<8,8,1>UD     0x128c5001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(16)        g33<1>UW        g16<8,8,1>UD    0x128c5102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0c4b2001
                            sampler MsgDesc: gather4_po_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g15<8,8,1>UD    0x168d2001
                            sampler MsgDesc: gather4_po_c SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         g54<1>UD        g7<8,8,1>UD     0x02280048
                            urb MsgDesc: 4 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         g7<1>UW         g53<8,8,1>UD    0x02106e00
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 0, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(8)         null<1>UW       g53<8,8,1>UD    0x02009500
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 0, SIMD8, inc) mlen 1 rlen 0 { align1 1Q };
send(8)         g7<1>UD         g37<8,8,1>UD    0x02480438
                            urb MsgDesc: 67 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g11<1>UD        g37<8,8,1>UD    0x02480638
                            urb MsgDesc: 99 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g13<1>UD        g14<8,8,1>UD    0x042a0148
                            urb MsgDesc: 20 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g14<8,8,1>UD    0x042a0048
                            urb MsgDesc: 4 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x124b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 9 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g16<8,8,1>UD    0x124b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 9 rlen 4 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x02380078
                            urb MsgDesc: 7 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x0a4a2001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g9<8,8,1>UD     0x128c2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         g50<1>UD        g51<8,8,1>UD    0x02180018
                            urb MsgDesc: 1 SIMD8 read mlen 1 rlen 1         { align1 1Q };
send(8)         g59<1>UW        g64<8,8,1>UD    0x02427002
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g64<8,8,1>UD    0x02427003
                            sampler MsgDesc: ld SIMD8 Surface = 3 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g64<8,8,1>UD    0x02427004
                            sampler MsgDesc: ld SIMD8 Surface = 4 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g64<8,8,1>UD    0x02427005
                            sampler MsgDesc: ld SIMD8 Surface = 5 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g64<8,8,1>UD    0x02427006
                            sampler MsgDesc: ld SIMD8 Surface = 6 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g64<8,8,1>UD    0x02427007
                            sampler MsgDesc: ld SIMD8 Surface = 7 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g64<8,8,1>UD    0x02427008
                            sampler MsgDesc: ld SIMD8 Surface = 8 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g64<8,8,1>UD    0x02427009
                            sampler MsgDesc: ld SIMD8 Surface = 9 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g30<1>UW        g64<8,8,1>UD    0x0242700a
                            sampler MsgDesc: ld SIMD8 Surface = 10 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g34<1>UW        g64<8,8,1>UD    0x0242700b
                            sampler MsgDesc: ld SIMD8 Surface = 11 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g64<8,8,1>UD    0x0242700c
                            sampler MsgDesc: ld SIMD8 Surface = 12 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g42<1>UW        g64<8,8,1>UD    0x0242700d
                            sampler MsgDesc: ld SIMD8 Surface = 13 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x06422505
                            sampler MsgDesc: sample_l SIMD8 Surface = 5 Sampler = 5 mlen 3 rlen 4 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0a088067
                            urb MsgDesc: 6 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0a088077
                            urb MsgDesc: 7 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0a088087
                            urb MsgDesc: 8 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0a088097
                            urb MsgDesc: 9 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0a0880a7
                            urb MsgDesc: 10 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0a0880b7
                            urb MsgDesc: 11 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0a0880c7
                            urb MsgDesc: 12 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0a0880d7
                            urb MsgDesc: 13 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0a0880e7
                            urb MsgDesc: 14 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0a0880f7
                            urb MsgDesc: 15 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0a088107
                            urb MsgDesc: 16 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0a088117
                            urb MsgDesc: 17 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0a088127
                            urb MsgDesc: 18 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0a088137
                            urb MsgDesc: 19 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0a088147
                            urb MsgDesc: 20 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0a088157
                            urb MsgDesc: 21 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0a088167
                            urb MsgDesc: 22 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0a088177
                            urb MsgDesc: 23 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0a088187
                            urb MsgDesc: 24 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0a088197
                            urb MsgDesc: 25 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0a0881a7
                            urb MsgDesc: 26 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0a0881b7
                            urb MsgDesc: 27 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0a0881c7
                            urb MsgDesc: 28 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0a0881d7
                            urb MsgDesc: 29 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x0a0881e7
                            urb MsgDesc: 30 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g41<8,8,1>UD    0x0a0881f7
                            urb MsgDesc: 31 SIMD8 write masked mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>F        g4<8,8,1>UD     0x0e0a8027
                            urb MsgDesc: 2 SIMD8 write per-slot masked mlen 7 rlen 0 { align1 1Q };
(+f1.0) send(8) g3<1>UW         g3<8,8,1>UD     0x0410b702
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, add) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(16) g4<1>UW        g6<8,8,1>UD     0x0820a702
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, add) mlen 4 rlen 2 { align1 1H };
send(8)         g2<1>UW         g10<8,8,1>UD    0x0a423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g15<8,8,1>UD    0x0a423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g29<1>UW        g9<8,8,1>UD     0x14843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(16)        g37<1>UW        g19<8,8,1>UD    0x14843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x06424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
(+f1.0) send(8) g4<1>UW         g12<8,8,1>UD    0x0210b502
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, inc) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(16) g5<1>UW        g17<8,8,1>UD    0x0420a502
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, inc) mlen 2 rlen 2 { align1 1H };
send(8)         g12<1>UD        g1<8,8,1>UD     0x02280058
                            urb MsgDesc: 5 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0e0a8067
                            urb MsgDesc: 6 SIMD8 write per-slot masked mlen 7 rlen 0 { align1 1Q };
send(8)         g12<1>UD        g1<8,8,1>UD     0x02280078
                            urb MsgDesc: 7 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0e0a8087
                            urb MsgDesc: 8 SIMD8 write per-slot masked mlen 7 rlen 0 { align1 1Q };
send(8)         g12<1>UD        g1<8,8,1>UD     0x02280098
                            urb MsgDesc: 9 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0e0a80a7
                            urb MsgDesc: 10 SIMD8 write per-slot masked mlen 7 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g9<8,8,1>UD     0x024ab001
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x028cb001
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 1 Sampler = 0 mlen 1 rlen 8 { align1 1H };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x06026c01
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD8, Mask = 0xc) mlen 3 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x0c025c01
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD16, Mask = 0xc) mlen 6 rlen 0 { align1 1H };
send(8)         null<1>UW       g9<8,8,1>UD     0x04026e00
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 0, SIMD8, Mask = 0xe) mlen 2 rlen 0 { align1 1Q };
send(8)         null<1>UW       g9<8,8,1>UD     0x06026c00
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 0, SIMD8, Mask = 0xc) mlen 3 rlen 0 { align1 1Q };
send(16)        g9<1>UW         g17<8,8,1>UD    0x04847002
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 0 mlen 2 rlen 8 { align1 1H };
(+f1.0) send(8) g14<1>UW        g13<8,8,1>UD    0x0410bb02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, imin) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g18<1>UW        g4<8,8,1>UD     0x0410b402
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, mov) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(16) g22<1>UW       g24<8,8,1>UD    0x0820ab02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, imin) mlen 4 rlen 2 { align1 1H };
send(16)        g23<1>UW        g3<8,8,1>UD     0x04205e02
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
(+f1.0) send(16) g26<1>UW       g6<8,8,1>UD     0x0820a402
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, mov) mlen 4 rlen 2 { align1 1H };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280068
                            urb MsgDesc: 6 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280088
                            urb MsgDesc: 8 SIMD8 read mlen 1 rlen 2         { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022800a8
                            urb MsgDesc: 10 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022800b8
                            urb MsgDesc: 11 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022800c8
                            urb MsgDesc: 12 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022800d8
                            urb MsgDesc: 13 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022800e8
                            urb MsgDesc: 14 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022800f8
                            urb MsgDesc: 15 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280108
                            urb MsgDesc: 16 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280118
                            urb MsgDesc: 17 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280128
                            urb MsgDesc: 18 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280138
                            urb MsgDesc: 19 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280148
                            urb MsgDesc: 20 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280158
                            urb MsgDesc: 21 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280168
                            urb MsgDesc: 22 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280178
                            urb MsgDesc: 23 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280188
                            urb MsgDesc: 24 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280198
                            urb MsgDesc: 25 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022801a8
                            urb MsgDesc: 26 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022801b8
                            urb MsgDesc: 27 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022801c8
                            urb MsgDesc: 28 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022801d8
                            urb MsgDesc: 29 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022801e8
                            urb MsgDesc: 30 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x022801f8
                            urb MsgDesc: 31 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g1<8,8,1>UD     0x02280208
                            urb MsgDesc: 32 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g2<1>UW         g3<8,8,1>UD     0x04203000
                            pixel interp MsgDesc: (persp, per_slot_offset, 0x00) mlen 2 rlen 2 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x08413000
                            pixel interp MsgDesc: (persp, per_slot_offset, 0x00) mlen 4 rlen 4 { align1 1H };
send(8)         g2<1>UW         g0<8,8,1>UD     0x02201010
                            pixel interp MsgDesc: (persp, sample_position, 0x10) mlen 1 rlen 2 { align1 1Q };
send(16)        g2<1>UW         g0<8,8,1>UD     0x02411010
                            pixel interp MsgDesc: (persp, sample_position, 0x10) mlen 1 rlen 4 { align1 1H };
send(8)         g2<1>UW         g0<8,8,1>UD     0x02201020
                            pixel interp MsgDesc: (persp, sample_position, 0x20) mlen 1 rlen 2 { align1 1Q };
send(16)        g2<1>UW         g0<8,8,1>UD     0x02411020
                            pixel interp MsgDesc: (persp, sample_position, 0x20) mlen 1 rlen 4 { align1 1H };
send(8)         g2<1>UW         g0<8,8,1>UD     0x02201030
                            pixel interp MsgDesc: (persp, sample_position, 0x30) mlen 1 rlen 2 { align1 1Q };
send(16)        g2<1>UW         g0<8,8,1>UD     0x02411030
                            pixel interp MsgDesc: (persp, sample_position, 0x30) mlen 1 rlen 4 { align1 1H };
(+f1.0) send(8) null<1>UW       g118<8,8,1>UD   0x02009601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, dec) mlen 1 rlen 0 { align1 1Q };
(+f1.0) send(8) g47<1>UW        g118<8,8,1>UD   0x0210b601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, dec) mlen 1 rlen 1 { align1 1Q };
send(16)        null<1>UW       g8<8,8,1>UD     0x0c025c02
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD16, Mask = 0xc) mlen 6 rlen 0 { align1 1H };
send(16)        g4<1>UW         g1<8,8,1>UD     0x04405c02
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD16, Mask = 0xc) mlen 2 rlen 4 { align1 1H };
send(8)         null<1>UW       g124<8,8,1>UD   0x02009600
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 0, SIMD8, dec) mlen 1 rlen 0 { align1 1Q };
send(8)         g51<1>UW        g124<8,8,1>UD   0x0210b600
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 0, SIMD8, dec) mlen 1 rlen 1 { align1 1Q };
send(8)         g127<1>UW       g9<8,8,1>UD     0x0819a401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, mov) mlen 4 rlen 1 { align1 1Q };
send(8)         g127<1>UW       g2<8,8,1>UD     0x0819b401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, mov) mlen 4 rlen 1 { align1 2Q };
send(16)        g13<1>UW        g21<8,8,1>UD    0x0c85d002
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 2 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g13<1>UW        g31<8,8,1>UD    0x0a43e002
                            sampler MsgDesc: ld2dms SIMD8 Surface = 2 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x04421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x08841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g36<1>UW        g0<8,8,1>F      0x021c003f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 63, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g37<1>UW        g0<8,8,1>F      0x021c0040
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 64, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g38<1>UW        g0<8,8,1>F      0x021c0041
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 65, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g39<1>UW        g0<8,8,1>F      0x021c0042
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 66, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g41<1>UW        g0<8,8,1>F      0x021c0044
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 68, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g40<1>UW        g0<8,8,1>F      0x021c0043
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 67, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g42<1>UW        g0<8,8,1>F      0x021c0045
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 69, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g43<1>UW        g0<8,8,1>F      0x021c0046
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 70, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g45<1>UW        g0<8,8,1>F      0x021c0048
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 72, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g44<1>UW        g0<8,8,1>F      0x021c0047
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 71, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g46<1>UW        g0<8,8,1>F      0x021c0049
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 73, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g47<1>UW        g0<8,8,1>F      0x021c004a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 74, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g61<1>UW        g0<8,8,1>F      0x021c0057
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 87, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g48<1>UW        g0<8,8,1>F      0x021c004b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 75, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g62<1>UW        g0<8,8,1>F      0x021c0058
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 88, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g57<1>UW        g0<8,8,1>F      0x021c0053
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 83, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g63<1>UW        g0<8,8,1>F      0x021c0059
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 89, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g58<1>UW        g0<8,8,1>F      0x021c0054
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 84, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g64<1>UW        g0<8,8,1>F      0x021c005a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 90, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g53<1>UW        g0<8,8,1>F      0x021c004f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 79, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g59<1>UW        g0<8,8,1>F      0x021c0055
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 85, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g54<1>UW        g0<8,8,1>F      0x021c0050
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 80, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g60<1>UW        g0<8,8,1>F      0x021c0056
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 86, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g51<1>UW        g0<8,8,1>F      0x021c004d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 77, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g55<1>UW        g0<8,8,1>F      0x021c0051
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 81, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g52<1>UW        g0<8,8,1>F      0x021c004e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 78, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g56<1>UW        g0<8,8,1>F      0x021c0052
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 82, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g50<1>UW        g0<8,8,1>F      0x021c004c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 76, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080117
                            urb MsgDesc: 17 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(8)         g6<1>UW         g9<8,8,1>UD     0x02406002
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD8, Mask = 0x0) mlen 1 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g14<8,8,1>UD    0x04805002
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD16, Mask = 0x0) mlen 2 rlen 8 { align1 1H };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0210b500
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 0, SIMD8, inc) mlen 1 rlen 1 { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080097
                            urb MsgDesc: 9 SIMD8 write mlen 9 rlen 0        { align1 1Q EOT };
send(8)         null<1>F        g4<8,8,1>F      0x120800c7
                            urb MsgDesc: 12 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g5<8,8,1>F      0x120800e7
                            urb MsgDesc: 14 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080107
                            urb MsgDesc: 16 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(8)         g6<1>UW         g6<8,8,1>UD     0x024ab101
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 1 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x024ab202
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 2 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g14<8,8,1>UD    0x024ab303
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 3 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x024ab404
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 4 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g22<8,8,1>UD    0x024ab505
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 5 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x064a2001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g12<8,8,1>UD    0x0a8c2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g15<8,8,1>UD    0x08423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g2<8,8,1>UD     0x10843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g2<1>UD         g9<8,8,1>UD     0x043a0028
                            urb MsgDesc: 2 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x02380098
                            urb MsgDesc: 9 SIMD8 read mlen 1 rlen 3         { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x023800a8
                            urb MsgDesc: 10 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x023800b8
                            urb MsgDesc: 11 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x023800d8
                            urb MsgDesc: 13 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x023800e8
                            urb MsgDesc: 14 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x023800f8
                            urb MsgDesc: 15 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x02380108
                            urb MsgDesc: 16 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g13<1>UD        g1<8,8,1>UD     0x02380118
                            urb MsgDesc: 17 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         null<1>F        g60<8,8,1>F     0x120800a7
                            urb MsgDesc: 10 SIMD8 write mlen 9 rlen 0       { align1 1Q };
send(8)         null<1>F        g119<8,8,1>F    0x92080107
                            urb MsgDesc: 16 SIMD8 write mlen 9 rlen 0       { align1 1Q EOT };
send(8)         g3<1>UW         g3<8,8,1>UD     0x04195e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xe) mlen 2 rlen 1 { align1 1Q };
send(8)         null<1>UW       g5<8,8,1>UD     0x060b5e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0xe) mlen 3 rlen 0 { align1 1Q };
send(8)         g5<1>UW         g8<8,8,1>UD     0x04196e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xe) mlen 2 rlen 1 { align1 2Q };
send(8)         null<1>UW       g10<8,8,1>UD    0x060b6e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0xe) mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080067
                            urb MsgDesc: 6 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c0077
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 119, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g30<1>UW        g0<8,8,1>F      0x021c0075
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 117, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c0074
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 116, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g14<1>UW        g0<8,8,1>F      0x021c0073
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 115, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g118<1>UW       g0<8,8,1>F      0x021c0070
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 112, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g122<1>UW       g0<8,8,1>F      0x021c0071
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 113, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c0072
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 114, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c006a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 106, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g70<1>UW        g0<8,8,1>F      0x021c006b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 107, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g102<1>UW       g0<8,8,1>F      0x021c006c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 108, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g106<1>UW       g0<8,8,1>F      0x021c006d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 109, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g110<1>UW       g0<8,8,1>F      0x021c006e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 110, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g114<1>UW       g0<8,8,1>F      0x021c006f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 111, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c005e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 94, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g14<1>UW        g0<8,8,1>F      0x021c005f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 95, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c0060
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 96, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g30<1>UW        g0<8,8,1>F      0x021c0061
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 97, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g34<1>UW        g0<8,8,1>F      0x021c0062
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 98, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g38<1>UW        g0<8,8,1>F      0x021c0063
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 99, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g42<1>UW        g0<8,8,1>F      0x021c0064
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 100, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g46<1>UW        g0<8,8,1>F      0x021c0065
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 101, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g50<1>UW        g0<8,8,1>F      0x021c0066
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 102, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g54<1>UW        g0<8,8,1>F      0x021c0067
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 103, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g58<1>UW        g0<8,8,1>F      0x021c0068
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 104, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g62<1>UW        g0<8,8,1>F      0x021c0069
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 105, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c005b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 91, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c005c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 92, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c005d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 93, 0) mlen 1 rlen 1           { align1 WE_all 1Q };
send(8)         null<1>F        g80<8,8,1>F     0x140a00b7
                            urb MsgDesc: 11 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a00d7
                            urb MsgDesc: 13 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a00f7
                            urb MsgDesc: 15 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a0117
                            urb MsgDesc: 17 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a0137
                            urb MsgDesc: 19 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g90<8,8,1>F     0x140a0157
                            urb MsgDesc: 21 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g100<8,8,1>F    0x140a0177
                            urb MsgDesc: 23 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g110<8,8,1>F    0x0c0a0197
                            urb MsgDesc: 25 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         g16<1>UW        g0<8,8,1>F      0x021c0076
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 118, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c0079
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 121, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c0078
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 120, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         null<1>F        g120<8,8,1>F    0x8c0a0197
                            urb MsgDesc: 25 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         null<1>F        g123<8,8,1>F    0x8a0800b7
                            urb MsgDesc: 11 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(8)         g22<1>UD        g53<8,8,1>UD    0x02180238
                            urb MsgDesc: 35 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g54<1>UD        g53<8,8,1>UD    0x02180438
                            urb MsgDesc: 67 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g67<1>UD        g53<8,8,1>UD    0x02180638
                            urb MsgDesc: 99 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g61<1>UD        g53<8,8,1>UD    0x02180248
                            urb MsgDesc: 36 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g66<1>UD        g53<8,8,1>UD    0x02180448
                            urb MsgDesc: 68 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g63<1>UD        g53<8,8,1>UD    0x02180648
                            urb MsgDesc: 100 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g68<1>UD        g65<8,8,1>UD    0x02180258
                            urb MsgDesc: 37 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g69<1>UD        g65<8,8,1>UD    0x02180458
                            urb MsgDesc: 69 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g70<1>UD        g65<8,8,1>UD    0x02180658
                            urb MsgDesc: 101 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g75<1>UD        g24<8,8,1>UD    0x02180268
                            urb MsgDesc: 38 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g76<1>UD        g24<8,8,1>UD    0x02180468
                            urb MsgDesc: 70 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g77<1>UD        g24<8,8,1>UD    0x02180668
                            urb MsgDesc: 102 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g82<1>UD        g25<8,8,1>UD    0x02180278
                            urb MsgDesc: 39 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g83<1>UD        g25<8,8,1>UD    0x02180478
                            urb MsgDesc: 71 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g84<1>UD        g25<8,8,1>UD    0x02180678
                            urb MsgDesc: 103 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g89<1>UD        g26<8,8,1>UD    0x02180288
                            urb MsgDesc: 40 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g90<1>UD        g26<8,8,1>UD    0x02180488
                            urb MsgDesc: 72 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g91<1>UD        g26<8,8,1>UD    0x02180688
                            urb MsgDesc: 104 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g96<1>UD        g27<8,8,1>UD    0x02180298
                            urb MsgDesc: 41 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g97<1>UD        g27<8,8,1>UD    0x02180498
                            urb MsgDesc: 73 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g98<1>UD        g27<8,8,1>UD    0x02180698
                            urb MsgDesc: 105 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g103<1>UD       g28<8,8,1>UD    0x021802a8
                            urb MsgDesc: 42 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g104<1>UD       g28<8,8,1>UD    0x021804a8
                            urb MsgDesc: 74 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g105<1>UD       g28<8,8,1>UD    0x021806a8
                            urb MsgDesc: 106 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g110<1>UD       g29<8,8,1>UD    0x021802b8
                            urb MsgDesc: 43 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g111<1>UD       g29<8,8,1>UD    0x021804b8
                            urb MsgDesc: 75 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g112<1>UD       g29<8,8,1>UD    0x021806b8
                            urb MsgDesc: 107 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g117<1>UD       g30<8,8,1>UD    0x021802c8
                            urb MsgDesc: 44 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g118<1>UD       g30<8,8,1>UD    0x021804c8
                            urb MsgDesc: 76 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g119<1>UD       g30<8,8,1>UD    0x021806c8
                            urb MsgDesc: 108 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g124<1>UD       g31<8,8,1>UD    0x021802d8
                            urb MsgDesc: 45 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g125<1>UD       g31<8,8,1>UD    0x021804d8
                            urb MsgDesc: 77 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g126<1>UD       g31<8,8,1>UD    0x021806d8
                            urb MsgDesc: 109 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g10<1>UD        g32<8,8,1>UD    0x021802e8
                            urb MsgDesc: 46 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g11<1>UD        g32<8,8,1>UD    0x021804e8
                            urb MsgDesc: 78 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g12<1>UD        g32<8,8,1>UD    0x021806e8
                            urb MsgDesc: 110 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g26<1>UD        g33<8,8,1>UD    0x021802f8
                            urb MsgDesc: 47 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g27<1>UD        g33<8,8,1>UD    0x021804f8
                            urb MsgDesc: 79 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g28<1>UD        g33<8,8,1>UD    0x021806f8
                            urb MsgDesc: 111 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g33<1>UD        g35<8,8,1>UD    0x02180308
                            urb MsgDesc: 48 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g34<1>UD        g35<8,8,1>UD    0x02180508
                            urb MsgDesc: 80 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g35<1>UD        g35<8,8,1>UD    0x02180708
                            urb MsgDesc: 112 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g64<1>UD        g36<8,8,1>UD    0x02180318
                            urb MsgDesc: 49 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g41<1>UD        g36<8,8,1>UD    0x02180518
                            urb MsgDesc: 81 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g42<1>UD        g36<8,8,1>UD    0x02180718
                            urb MsgDesc: 113 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g6<1>UD         g37<8,8,1>UD    0x02180328
                            urb MsgDesc: 50 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g48<1>UD        g37<8,8,1>UD    0x02180528
                            urb MsgDesc: 82 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g49<1>UD        g37<8,8,1>UD    0x02180728
                            urb MsgDesc: 114 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g67<1>UD        g38<8,8,1>UD    0x02180338
                            urb MsgDesc: 51 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g56<1>UD        g38<8,8,1>UD    0x02180538
                            urb MsgDesc: 83 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g57<1>UD        g38<8,8,1>UD    0x02180738
                            urb MsgDesc: 115 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g66<1>UD        g39<8,8,1>UD    0x02180348
                            urb MsgDesc: 52 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g63<1>UD        g39<8,8,1>UD    0x02180548
                            urb MsgDesc: 84 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g40<1>UD        g39<8,8,1>UD    0x02180748
                            urb MsgDesc: 116 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g69<1>UD        g64<8,8,1>UD    0x02180358
                            urb MsgDesc: 53 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g70<1>UD        g64<8,8,1>UD    0x02180558
                            urb MsgDesc: 85 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g71<1>UD        g64<8,8,1>UD    0x02180758
                            urb MsgDesc: 117 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g76<1>UD        g41<8,8,1>UD    0x02180368
                            urb MsgDesc: 54 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g77<1>UD        g41<8,8,1>UD    0x02180568
                            urb MsgDesc: 86 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g78<1>UD        g41<8,8,1>UD    0x02180768
                            urb MsgDesc: 118 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g83<1>UD        g42<8,8,1>UD    0x02180378
                            urb MsgDesc: 55 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g84<1>UD        g42<8,8,1>UD    0x02180578
                            urb MsgDesc: 87 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g85<1>UD        g42<8,8,1>UD    0x02180778
                            urb MsgDesc: 119 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g90<1>UD        g43<8,8,1>UD    0x02180388
                            urb MsgDesc: 56 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g91<1>UD        g43<8,8,1>UD    0x02180588
                            urb MsgDesc: 88 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g92<1>UD        g43<8,8,1>UD    0x02180788
                            urb MsgDesc: 120 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g97<1>UD        g44<8,8,1>UD    0x02180398
                            urb MsgDesc: 57 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g98<1>UD        g44<8,8,1>UD    0x02180598
                            urb MsgDesc: 89 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g99<1>UD        g44<8,8,1>UD    0x02180798
                            urb MsgDesc: 121 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g104<1>UD       g45<8,8,1>UD    0x021803a8
                            urb MsgDesc: 58 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g105<1>UD       g45<8,8,1>UD    0x021805a8
                            urb MsgDesc: 90 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g106<1>UD       g45<8,8,1>UD    0x021807a8
                            urb MsgDesc: 122 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g111<1>UD       g46<8,8,1>UD    0x021803b8
                            urb MsgDesc: 59 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g112<1>UD       g46<8,8,1>UD    0x021805b8
                            urb MsgDesc: 91 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g113<1>UD       g46<8,8,1>UD    0x021807b8
                            urb MsgDesc: 123 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g118<1>UD       g6<8,8,1>UD     0x021803c8
                            urb MsgDesc: 60 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g119<1>UD       g6<8,8,1>UD     0x021805c8
                            urb MsgDesc: 92 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g120<1>UD       g6<8,8,1>UD     0x021807c8
                            urb MsgDesc: 124 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g125<1>UD       g48<8,8,1>UD    0x021803d8
                            urb MsgDesc: 61 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g126<1>UD       g48<8,8,1>UD    0x021805d8
                            urb MsgDesc: 93 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g2<1>UD         g48<8,8,1>UD    0x021807d8
                            urb MsgDesc: 125 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g12<1>UD        g49<8,8,1>UD    0x021803e8
                            urb MsgDesc: 62 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g13<1>UD        g49<8,8,1>UD    0x021805e8
                            urb MsgDesc: 94 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g14<1>UD        g49<8,8,1>UD    0x021807e8
                            urb MsgDesc: 126 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g19<1>UD        g50<8,8,1>UD    0x021803f8
                            urb MsgDesc: 63 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g20<1>UD        g50<8,8,1>UD    0x021805f8
                            urb MsgDesc: 95 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g53<1>UD        g50<8,8,1>UD    0x021807f8
                            urb MsgDesc: 127 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g28<1>UD        g51<8,8,1>UD    0x02180408
                            urb MsgDesc: 64 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g29<1>UD        g51<8,8,1>UD    0x02180608
                            urb MsgDesc: 96 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g30<1>UD        g51<8,8,1>UD    0x02180808
                            urb MsgDesc: 128 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         g35<1>UD        g22<8,8,1>UD    0x02180218
                            urb MsgDesc: 33 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g36<1>UD        g22<8,8,1>UD    0x02180418
                            urb MsgDesc: 65 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g37<1>UD        g22<8,8,1>UD    0x02180618
                            urb MsgDesc: 97 SIMD8 read mlen 1 rlen 1        { align1 1Q };
send(8)         g38<1>UD        g22<8,8,1>UD    0x02180818
                            urb MsgDesc: 129 SIMD8 read mlen 1 rlen 1       { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x080a8037
                            urb MsgDesc: 3 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g10<8,8,1>UD    0x080a8047
                            urb MsgDesc: 4 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x080a8057
                            urb MsgDesc: 5 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x080a8067
                            urb MsgDesc: 6 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x080a8077
                            urb MsgDesc: 7 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x080a8087
                            urb MsgDesc: 8 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x080a8097
                            urb MsgDesc: 9 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x080a80a7
                            urb MsgDesc: 10 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x080a80b7
                            urb MsgDesc: 11 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x080a80c7
                            urb MsgDesc: 12 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x080a80d7
                            urb MsgDesc: 13 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x080a80e7
                            urb MsgDesc: 14 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x080a80f7
                            urb MsgDesc: 15 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x080a8107
                            urb MsgDesc: 16 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x080a8117
                            urb MsgDesc: 17 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x080a8127
                            urb MsgDesc: 18 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x080a8137
                            urb MsgDesc: 19 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x080a8147
                            urb MsgDesc: 20 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x080a8157
                            urb MsgDesc: 21 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x080a8167
                            urb MsgDesc: 22 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x080a8177
                            urb MsgDesc: 23 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x080a8187
                            urb MsgDesc: 24 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x080a8197
                            urb MsgDesc: 25 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x080a81a7
                            urb MsgDesc: 26 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x080a81b7
                            urb MsgDesc: 27 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x080a81c7
                            urb MsgDesc: 28 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x080a81d7
                            urb MsgDesc: 29 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x080a81e7
                            urb MsgDesc: 30 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x080a81f7
                            urb MsgDesc: 31 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x080a8207
                            urb MsgDesc: 32 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x080a8217
                            urb MsgDesc: 33 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         g19<1>UW        g20<8,8,1>UD    0x06195e00
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 0, SIMD16, Mask = 0xe) mlen 3 rlen 1 { align1 1Q };
send(8)         null<1>UW       g23<8,8,1>UD    0x080b5e00
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 0, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g3<8,8,1>UD     0x080b5e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g1<8,8,1>UD     0x080b6e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0xe) mlen 4 rlen 0 { align1 2Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0a4b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g12<8,8,1>UD    0x0a4b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(8)         g34<1>UD        g42<8,8,1>UD    0x02480248
                            urb MsgDesc: 36 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g38<1>UD        g42<8,8,1>UD    0x02480448
                            urb MsgDesc: 68 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g42<1>UD        g42<8,8,1>UD    0x02480648
                            urb MsgDesc: 100 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g22<1>UD        g43<8,8,1>UD    0x02480258
                            urb MsgDesc: 37 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g43<8,8,1>UD    0x02480458
                            urb MsgDesc: 69 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g43<8,8,1>UD    0x02480658
                            urb MsgDesc: 101 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g22<1>UD        g44<8,8,1>UD    0x02480268
                            urb MsgDesc: 38 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g44<8,8,1>UD    0x02480468
                            urb MsgDesc: 70 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g44<8,8,1>UD    0x02480668
                            urb MsgDesc: 102 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g22<1>UD        g45<8,8,1>UD    0x02480278
                            urb MsgDesc: 39 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g45<8,8,1>UD    0x02480478
                            urb MsgDesc: 71 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g45<8,8,1>UD    0x02480678
                            urb MsgDesc: 103 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g22<1>UD        g55<8,8,1>UD    0x02480288
                            urb MsgDesc: 40 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g55<8,8,1>UD    0x02480488
                            urb MsgDesc: 72 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g55<8,8,1>UD    0x02480688
                            urb MsgDesc: 104 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g26<1>UD        g56<8,8,1>UD    0x02480498
                            urb MsgDesc: 73 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g56<8,8,1>UD    0x02480298
                            urb MsgDesc: 41 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g56<8,8,1>UD    0x02480698
                            urb MsgDesc: 105 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g26<1>UD        g82<8,8,1>UD    0x024804a8
                            urb MsgDesc: 74 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g82<8,8,1>UD    0x024802a8
                            urb MsgDesc: 42 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g82<8,8,1>UD    0x024806a8
                            urb MsgDesc: 106 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g26<1>UD        g83<8,8,1>UD    0x024804b8
                            urb MsgDesc: 75 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g83<8,8,1>UD    0x024802b8
                            urb MsgDesc: 43 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g30<1>UD        g83<8,8,1>UD    0x024806b8
                            urb MsgDesc: 107 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g26<1>UD        g84<8,8,1>UD    0x024806c8
                            urb MsgDesc: 108 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g84<8,8,1>UD    0x024802c8
                            urb MsgDesc: 44 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g84<8,8,1>UD    0x024804c8
                            urb MsgDesc: 76 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g8<1>UD         g85<8,8,1>UD    0x024802d8
                            urb MsgDesc: 45 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g85<8,8,1>UD    0x024804d8
                            urb MsgDesc: 77 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g85<8,8,1>UD    0x024806d8
                            urb MsgDesc: 109 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g6<8,8,1>UD     0x024802e8
                            urb MsgDesc: 46 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g6<8,8,1>UD     0x024804e8
                            urb MsgDesc: 78 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g6<8,8,1>UD     0x024806e8
                            urb MsgDesc: 110 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g3<8,8,1>UD     0x024802f8
                            urb MsgDesc: 47 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g3<8,8,1>UD     0x024804f8
                            urb MsgDesc: 79 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g3<8,8,1>UD     0x024806f8
                            urb MsgDesc: 111 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g46<8,8,1>UD    0x02480308
                            urb MsgDesc: 48 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g46<8,8,1>UD    0x02480508
                            urb MsgDesc: 80 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g46<8,8,1>UD    0x02480708
                            urb MsgDesc: 112 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g47<8,8,1>UD    0x02480318
                            urb MsgDesc: 49 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g47<8,8,1>UD    0x02480518
                            urb MsgDesc: 81 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g47<8,8,1>UD    0x02480718
                            urb MsgDesc: 113 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g57<8,8,1>UD    0x02480328
                            urb MsgDesc: 50 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g57<8,8,1>UD    0x02480528
                            urb MsgDesc: 82 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g57<8,8,1>UD    0x02480728
                            urb MsgDesc: 114 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g58<8,8,1>UD    0x02480338
                            urb MsgDesc: 51 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g58<8,8,1>UD    0x02480538
                            urb MsgDesc: 83 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g26<1>UD        g58<8,8,1>UD    0x02480738
                            urb MsgDesc: 115 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g59<8,8,1>UD    0x02480348
                            urb MsgDesc: 52 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g59<8,8,1>UD    0x02480548
                            urb MsgDesc: 84 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g59<8,8,1>UD    0x02480748
                            urb MsgDesc: 116 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g60<8,8,1>UD    0x02480358
                            urb MsgDesc: 53 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g60<8,8,1>UD    0x02480558
                            urb MsgDesc: 85 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g60<8,8,1>UD    0x02480758
                            urb MsgDesc: 117 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g61<8,8,1>UD    0x02480368
                            urb MsgDesc: 54 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g61<8,8,1>UD    0x02480568
                            urb MsgDesc: 86 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g61<8,8,1>UD    0x02480768
                            urb MsgDesc: 118 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g62<8,8,1>UD    0x02480378
                            urb MsgDesc: 55 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g62<8,8,1>UD    0x02480578
                            urb MsgDesc: 87 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g62<8,8,1>UD    0x02480778
                            urb MsgDesc: 119 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g63<8,8,1>UD    0x02480388
                            urb MsgDesc: 56 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g63<8,8,1>UD    0x02480588
                            urb MsgDesc: 88 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g63<8,8,1>UD    0x02480788
                            urb MsgDesc: 120 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g64<8,8,1>UD    0x02480398
                            urb MsgDesc: 57 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g64<8,8,1>UD    0x02480598
                            urb MsgDesc: 89 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g64<8,8,1>UD    0x02480798
                            urb MsgDesc: 121 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g68<8,8,1>UD    0x024803a8
                            urb MsgDesc: 58 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g68<8,8,1>UD    0x024805a8
                            urb MsgDesc: 90 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g68<8,8,1>UD    0x024807a8
                            urb MsgDesc: 122 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g69<8,8,1>UD    0x024803b8
                            urb MsgDesc: 59 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g69<8,8,1>UD    0x024805b8
                            urb MsgDesc: 91 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g22<1>UD        g69<8,8,1>UD    0x024807b8
                            urb MsgDesc: 123 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g70<8,8,1>UD    0x024803c8
                            urb MsgDesc: 60 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g70<8,8,1>UD    0x024805c8
                            urb MsgDesc: 92 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g16<1>UD        g70<8,8,1>UD    0x024807c8
                            urb MsgDesc: 124 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g71<8,8,1>UD    0x024803d8
                            urb MsgDesc: 61 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g71<8,8,1>UD    0x024805d8
                            urb MsgDesc: 93 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g16<1>UD        g71<8,8,1>UD    0x024807d8
                            urb MsgDesc: 125 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g72<8,8,1>UD    0x024803e8
                            urb MsgDesc: 62 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g72<8,8,1>UD    0x024805e8
                            urb MsgDesc: 94 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g16<1>UD        g72<8,8,1>UD    0x024807e8
                            urb MsgDesc: 126 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g8<1>UD         g73<8,8,1>UD    0x024803f8
                            urb MsgDesc: 63 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g12<1>UD        g73<8,8,1>UD    0x024805f8
                            urb MsgDesc: 95 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g16<1>UD        g73<8,8,1>UD    0x024807f8
                            urb MsgDesc: 127 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         g12<1>UD        g75<8,8,1>UD    0x02480418
                            urb MsgDesc: 65 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g16<1>UD        g75<8,8,1>UD    0x02480618
                            urb MsgDesc: 97 SIMD8 read mlen 1 rlen 4        { align1 1Q };
send(8)         g20<1>UD        g75<8,8,1>UD    0x02480818
                            urb MsgDesc: 129 SIMD8 read mlen 1 rlen 4       { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0c0a00c7
                            urb MsgDesc: 12 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0c0a00d7
                            urb MsgDesc: 13 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0c0a00e7
                            urb MsgDesc: 14 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0c0a00f7
                            urb MsgDesc: 15 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0c0a0107
                            urb MsgDesc: 16 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0c0a0117
                            urb MsgDesc: 17 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0c0a0137
                            urb MsgDesc: 19 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0c0a0147
                            urb MsgDesc: 20 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0c0a0157
                            urb MsgDesc: 21 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0c0a0167
                            urb MsgDesc: 22 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0c0a0177
                            urb MsgDesc: 23 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0c0a0187
                            urb MsgDesc: 24 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0c0a01a7
                            urb MsgDesc: 26 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0c0a01b7
                            urb MsgDesc: 27 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0c0a01c7
                            urb MsgDesc: 28 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0c0a01d7
                            urb MsgDesc: 29 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0c0a01e7
                            urb MsgDesc: 30 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0c0a01f7
                            urb MsgDesc: 31 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q };
send(16)        g46<1>UD        g12<8,8,1>UD    0x02280302
                            const MsgDesc: (2, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g50<1>UD        g15<8,8,1>UD    0x02280304
                            const MsgDesc: (4, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g34<1>UD        g20<8,8,1>UD    0x02280303
                            const MsgDesc: (3, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g16<1>UD        g21<8,8,1>UD    0x02280306
                            const MsgDesc: (6, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(32)        g87<1>UW        g0<8,8,1>F      0x024c200c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 12, 32) mlen 1 rlen 4          { align1 WE_all };
send(32)        g91<1>UW        g0<8,8,1>F      0x024c2014
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 20, 32) mlen 1 rlen 4          { align1 WE_all };
send(32)        g95<1>UW        g0<8,8,1>F      0x024c201c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 28, 32) mlen 1 rlen 4          { align1 WE_all };
send(8)         g8<1>UW         g24<8,8,1>UD    0x02106e04
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 4, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(8)         g5<1>UW         g21<8,8,1>UD    0x02106e03
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 3, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(16)        g14<1>UW        g40<8,8,1>UD    0x04205e04
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 4, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
send(16)        g8<1>UW         g36<8,8,1>UD    0x04205e03
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 3, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
(+f1.0) send(8) null<1>UW       g11<8,8,1>UD    0x0a026002
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g11<8,8,1>UD    0x14025002
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD16, Mask = 0x0) mlen 10 rlen 0 { align1 1H };
send(8)         g15<1>UD        g12<8,8,1>UD    0x041a0038
                            urb MsgDesc: 3 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g22<1>UW        g14<8,8,1>UD    0x064a8404
                            sampler MsgDesc: gather4 SIMD8 Surface = 4 Sampler = 4 mlen 3 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g10<8,8,1>UD    0x084a8202
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 2 mlen 4 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g26<8,8,1>UD    0x0a4a8303
                            sampler MsgDesc: gather4 SIMD8 Surface = 3 Sampler = 3 mlen 5 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g54<8,8,1>UD    0x0242a707
                            sampler MsgDesc: resinfo SIMD8 Surface = 7 Sampler = 7 mlen 1 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g55<8,8,1>UD    0x0242a808
                            sampler MsgDesc: resinfo SIMD8 Surface = 8 Sampler = 8 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g56<8,8,1>UD    0x0242a909
                            sampler MsgDesc: resinfo SIMD8 Surface = 9 Sampler = 9 mlen 1 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g57<8,8,1>UD    0x0242aa0a
                            sampler MsgDesc: resinfo SIMD8 Surface = 10 Sampler = 10 mlen 1 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g58<8,8,1>UD    0x0242ab0b
                            sampler MsgDesc: resinfo SIMD8 Surface = 11 Sampler = 11 mlen 1 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g59<8,8,1>UD    0x0242ac0c
                            sampler MsgDesc: resinfo SIMD8 Surface = 12 Sampler = 12 mlen 1 rlen 4 { align1 1Q };
send(8)         null<1>F        g9<8,8,1>UD     0x0c088027
                            urb MsgDesc: 2 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g10<8,8,1>UD    0x0c088047
                            urb MsgDesc: 4 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x0c088067
                            urb MsgDesc: 6 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>UD     0x0c088037
                            urb MsgDesc: 3 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g7<8,8,1>UD     0x0c088057
                            urb MsgDesc: 5 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g8<8,8,1>UD     0x0c088077
                            urb MsgDesc: 7 SIMD8 write masked mlen 6 rlen 0 { align1 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c0100
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 0, 1) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00c1
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 193, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g79<1>UW        g0<8,8,1>F      0x021c00fe
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 254, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g75<1>UW        g0<8,8,1>F      0x021c00fd
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 253, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g71<1>UW        g0<8,8,1>F      0x021c00fc
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 252, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g67<1>UW        g0<8,8,1>F      0x021c00fb
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 251, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g63<1>UW        g0<8,8,1>F      0x021c00fa
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 250, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g47<1>UW        g0<8,8,1>F      0x021c00f6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 246, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g8<1>UW         g0<8,8,1>F      0x021c00ef
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 239, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g15<1>UW        g0<8,8,1>F      0x021c00f0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 240, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g23<1>UW        g0<8,8,1>F      0x021c00f1
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 241, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g31<1>UW        g0<8,8,1>F      0x021c00f2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 242, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g35<1>UW        g0<8,8,1>F      0x021c00f3
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 243, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g39<1>UW        g0<8,8,1>F      0x021c00f4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 244, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g43<1>UW        g0<8,8,1>F      0x021c00f5
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 245, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g51<1>UW        g0<8,8,1>F      0x021c00f7
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 247, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g55<1>UW        g0<8,8,1>F      0x021c00f8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 248, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g59<1>UW        g0<8,8,1>F      0x021c00f9
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 249, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g89<1>UW        g0<8,8,1>F      0x021c00ee
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 238, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g85<1>UW        g0<8,8,1>F      0x021c00ed
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 237, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g81<1>UW        g0<8,8,1>F      0x021c00ec
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 236, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g77<1>UW        g0<8,8,1>F      0x021c00eb
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 235, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00df
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 223, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g33<1>UW        g0<8,8,1>F      0x021c00e0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 224, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g69<1>UW        g0<8,8,1>F      0x021c00e9
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 233, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g73<1>UW        g0<8,8,1>F      0x021c00ea
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 234, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g37<1>UW        g0<8,8,1>F      0x021c00e1
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 225, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g41<1>UW        g0<8,8,1>F      0x021c00e2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 226, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g45<1>UW        g0<8,8,1>F      0x021c00e3
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 227, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g49<1>UW        g0<8,8,1>F      0x021c00e4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 228, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g53<1>UW        g0<8,8,1>F      0x021c00e5
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 229, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g57<1>UW        g0<8,8,1>F      0x021c00e6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 230, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g61<1>UW        g0<8,8,1>F      0x021c00e7
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 231, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g65<1>UW        g0<8,8,1>F      0x021c00e8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 232, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g23<1>UW        g0<8,8,1>F      0x021c00be
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 190, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00c0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 192, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g116<1>UW       g0<8,8,1>F      0x021c00bf
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 191, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g7<1>UW         g0<8,8,1>F      0x021c00c3
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 195, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00c4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 196, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00c5
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 197, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00c6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 198, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00c7
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 199, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00c8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 200, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00c9
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 201, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00ca
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 202, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00cb
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 203, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00cc
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 204, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00cd
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 205, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00ce
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 206, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00cf
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 207, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00d0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 208, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00d1
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 209, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00d2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 210, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00d3
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 211, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00d4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 212, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00d5
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 213, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00d6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 214, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c00db
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 219, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00dc
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 220, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00dd
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 221, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00de
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 222, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00d7
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 215, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c00d8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 216, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c00d9
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 217, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c00da
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 218, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a0197
                            urb MsgDesc: 25 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a01b7
                            urb MsgDesc: 27 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g6<8,8,1>F      0x140a01d7
                            urb MsgDesc: 29 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(16)        g6<1>UW         g0<8,8,1>F      0x022c10a0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 160, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g8<1>UW         g0<8,8,1>F      0x022c10a2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 162, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g10<1>UW        g0<8,8,1>F      0x022c10a4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 164, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g12<1>UW        g0<8,8,1>F      0x022c10a6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 166, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g14<1>UW        g0<8,8,1>F      0x022c10a8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 168, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(8)         null<1>F        g6<8,8,1>F      0x140a01f7
                            urb MsgDesc: 31 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         g16<1>UW        g0<8,8,1>F      0x021c00ff
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 255, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(16)        g18<1>UW        g0<8,8,1>F      0x022c108e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 142, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g20<1>UW        g0<8,8,1>F      0x022c1090
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 144, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g22<1>UW        g0<8,8,1>F      0x022c1092
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 146, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g24<1>UW        g0<8,8,1>F      0x022c1094
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 148, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g16<1>UW        g0<8,8,1>F      0x022c10aa
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 170, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g18<1>UW        g0<8,8,1>F      0x022c10ac
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 172, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g20<1>UW        g0<8,8,1>F      0x022c10ae
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 174, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g22<1>UW        g0<8,8,1>F      0x022c10b0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 176, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g24<1>UW        g0<8,8,1>F      0x022c10b2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 178, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c0102
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 2, 1) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g4<1>UW         g0<8,8,1>F      0x021c0101
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 1, 1) mlen 1 rlen 1            { align1 WE_all 1Q };
send(8)         g5<1>UW         g0<8,8,1>F      0x021c00c2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 194, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(16)        g16<1>UW        g0<8,8,1>F      0x022c1096
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 150, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g18<1>UW        g0<8,8,1>F      0x022c1098
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 152, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g20<1>UW        g0<8,8,1>F      0x022c109a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 154, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g22<1>UW        g0<8,8,1>F      0x022c109c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 156, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g24<1>UW        g0<8,8,1>F      0x022c109e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 158, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g16<1>UW        g0<8,8,1>F      0x022c10b4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 180, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g18<1>UW        g0<8,8,1>F      0x022c10b6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 182, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g20<1>UW        g0<8,8,1>F      0x022c10b8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 184, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g22<1>UW        g0<8,8,1>F      0x022c10ba
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 186, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(16)        g24<1>UW        g0<8,8,1>F      0x022c10bc
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 188, 16) mlen 1 rlen 2         { align1 WE_all 1H };
send(8)         null<1>F        g120<8,8,1>F    0x8c0a0217
                            urb MsgDesc: 33 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         g8<1>UD         g6<8,8,1>UD     0x041a0318
                            urb MsgDesc: 49 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g9<1>UD         g6<8,8,1>UD     0x041a0518
                            urb MsgDesc: 81 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g10<1>UD        g6<8,8,1>UD     0x041a0718
                            urb MsgDesc: 113 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g11<1>UD        g6<8,8,1>UD     0x041a0918
                            urb MsgDesc: 145 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g7<1>UD         g11<8,8,1>UD    0x041a0218
                            urb MsgDesc: 33 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g8<1>UD         g11<8,8,1>UD    0x041a0418
                            urb MsgDesc: 65 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g9<1>UD         g11<8,8,1>UD    0x041a0618
                            urb MsgDesc: 97 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g10<1>UD        g11<8,8,1>UD    0x041a0818
                            urb MsgDesc: 129 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         null<1>F        g10<8,8,1>UD    0x080a8227
                            urb MsgDesc: 34 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>UD    0x080a8237
                            urb MsgDesc: 35 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x080a8247
                            urb MsgDesc: 36 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x080a8257
                            urb MsgDesc: 37 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x080a8267
                            urb MsgDesc: 38 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x080a8277
                            urb MsgDesc: 39 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x080a8287
                            urb MsgDesc: 40 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x080a8297
                            urb MsgDesc: 41 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x080a82a7
                            urb MsgDesc: 42 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x080a82b7
                            urb MsgDesc: 43 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x080a82c7
                            urb MsgDesc: 44 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x080a82d7
                            urb MsgDesc: 45 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x080a82e7
                            urb MsgDesc: 46 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x080a82f7
                            urb MsgDesc: 47 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x080a8307
                            urb MsgDesc: 48 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x080a8317
                            urb MsgDesc: 49 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x080a8327
                            urb MsgDesc: 50 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x080a8337
                            urb MsgDesc: 51 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x080a8347
                            urb MsgDesc: 52 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x080a8357
                            urb MsgDesc: 53 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x080a8367
                            urb MsgDesc: 54 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x080a8377
                            urb MsgDesc: 55 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x080a8387
                            urb MsgDesc: 56 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x080a8397
                            urb MsgDesc: 57 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x080a83a7
                            urb MsgDesc: 58 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x080a83b7
                            urb MsgDesc: 59 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x080a83c7
                            urb MsgDesc: 60 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x080a83d7
                            urb MsgDesc: 61 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x080a83e7
                            urb MsgDesc: 62 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x080a83f7
                            urb MsgDesc: 63 SIMD8 write per-slot masked mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>F        g123<8,8,1>F    0x8a080007
                            urb MsgDesc: 0 SIMD8 write mlen 5 rlen 0        { align1 1Q EOT };
(+f1.0) send(8) g14<1>UW        g13<8,8,1>UD    0x0410bd02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, umin) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(16) g22<1>UW       g24<8,8,1>UD    0x0820ad02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, umin) mlen 4 rlen 2 { align1 1H };
send(8)         g2<1>UW         g19<8,8,1>UD    0x06295c01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xc) mlen 3 rlen 2 { align1 1Q };
send(8)         null<1>UW       g22<8,8,1>UD    0x0a0b5c02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0xc) mlen 5 rlen 0 { align1 1Q };
send(8)         g49<1>UW        g43<8,8,1>UD    0x06296c01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xc) mlen 3 rlen 2 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x0a0b6c02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0xc) mlen 5 rlen 0 { align1 2Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x104a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x084a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x0e8c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g119<1>UW       g0<8,8,1>F      0x021c00b2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 178, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g39<1>UW        g0<8,8,1>F      0x021c00ad
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 173, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g40<1>UW        g0<8,8,1>F      0x021c00af
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 175, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g41<1>UW        g0<8,8,1>F      0x021c00ae
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 174, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g42<1>UW        g0<8,8,1>F      0x021c00b0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 176, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g33<1>UW        g0<8,8,1>F      0x021c00a9
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 169, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g34<1>UW        g0<8,8,1>F      0x021c00a8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 168, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g35<1>UW        g0<8,8,1>F      0x021c00aa
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 170, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g25<1>UW        g0<8,8,1>F      0x021c00a4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 164, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g36<1>UW        g0<8,8,1>F      0x021c00ab
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 171, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g30<1>UW        g0<8,8,1>F      0x021c00a5
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 165, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g31<1>UW        g0<8,8,1>F      0x021c00a7
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 167, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g32<1>UW        g0<8,8,1>F      0x021c00a6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 166, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g17<1>UW        g0<8,8,1>F      0x021c00a0
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 160, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g22<1>UW        g0<8,8,1>F      0x021c00a2
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 162, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g23<1>UW        g0<8,8,1>F      0x021c00a1
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 161, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g24<1>UW        g0<8,8,1>F      0x021c00a3
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 163, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g13<1>UW        g0<8,8,1>F      0x021c009d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 157, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g14<1>UW        g0<8,8,1>F      0x021c009c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 156, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g15<1>UW        g0<8,8,1>F      0x021c009e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 158, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g9<1>UW         g0<8,8,1>F      0x021c0098
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 152, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g16<1>UW        g0<8,8,1>F      0x021c009f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 159, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g10<1>UW        g0<8,8,1>F      0x021c0099
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 153, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g11<1>UW        g0<8,8,1>F      0x021c009b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 155, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g12<1>UW        g0<8,8,1>F      0x021c009a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 154, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g121<1>UW       g0<8,8,1>F      0x021c0094
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 148, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g6<1>UW         g0<8,8,1>F      0x021c0096
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 150, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g7<1>UW         g0<8,8,1>F      0x021c0095
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 149, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g8<1>UW         g0<8,8,1>F      0x021c0097
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 151, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g116<1>UW       g0<8,8,1>F      0x021c0091
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 145, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g117<1>UW       g0<8,8,1>F      0x021c0090
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 144, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g119<1>UW       g0<8,8,1>F      0x021c0092
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 146, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g112<1>UW       g0<8,8,1>F      0x021c008c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 140, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g120<1>UW       g0<8,8,1>F      0x021c0093
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 147, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g113<1>UW       g0<8,8,1>F      0x021c008d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 141, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g114<1>UW       g0<8,8,1>F      0x021c008f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 143, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g115<1>UW       g0<8,8,1>F      0x021c008e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 142, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g104<1>UW       g0<8,8,1>F      0x021c0084
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 132, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g105<1>UW       g0<8,8,1>F      0x021c0086
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 134, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g106<1>UW       g0<8,8,1>F      0x021c0085
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 133, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g107<1>UW       g0<8,8,1>F      0x021c0087
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 135, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g108<1>UW       g0<8,8,1>F      0x021c0089
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 137, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g109<1>UW       g0<8,8,1>F      0x021c0088
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 136, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g110<1>UW       g0<8,8,1>F      0x021c008a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 138, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g100<1>UW       g0<8,8,1>F      0x021c0080
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 128, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g111<1>UW       g0<8,8,1>F      0x021c008b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 139, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g101<1>UW       g0<8,8,1>F      0x021c0081
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 129, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g102<1>UW       g0<8,8,1>F      0x021c0083
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 131, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g103<1>UW       g0<8,8,1>F      0x021c0082
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 130, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g96<1>UW        g0<8,8,1>F      0x021c007c
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 124, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g97<1>UW        g0<8,8,1>F      0x021c007e
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 126, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g98<1>UW        g0<8,8,1>F      0x021c007d
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 125, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g99<1>UW        g0<8,8,1>F      0x021c007f
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 127, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g44<1>UW        g0<8,8,1>F      0x021c00b1
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 177, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g94<1>UW        g0<8,8,1>F      0x021c007a
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 122, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g95<1>UW        g0<8,8,1>F      0x021c007b
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 123, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g38<1>UW        g0<8,8,1>F      0x021c00ac
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 172, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         null<1>F        g121<8,8,1>F    0x8a080197
                            urb MsgDesc: 25 SIMD8 write mlen 5 rlen 0       { align1 1Q EOT };
send(8)         g2<1>UW         g17<8,8,1>UD    0x04495000
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 0, SIMD16, Mask = 0x0) mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g17<8,8,1>UD    0x04295c00
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 0, SIMD16, Mask = 0xc) mlen 2 rlen 2 { align1 1Q };
send(8)         g18<1>UW        g17<8,8,1>UD    0x04195e00
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 0, SIMD16, Mask = 0xe) mlen 2 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619a700
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, add) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619ad00
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, umin) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619ac00
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, umax) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619a100
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, and) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619a200
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, or) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619a300
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, xor) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0619a400
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, mov) mlen 3 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g19<8,8,1>UD    0x0819ae00
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 0, SIMD16, cmpwr) mlen 4 rlen 1 { align1 1Q };
send(8)         g34<1>UW        g0<8,8,1>F      0x021c00b3
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 179, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g35<1>UW        g0<8,8,1>F      0x021c00b4
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 180, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g36<1>UW        g0<8,8,1>F      0x021c00b5
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 181, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g119<1>UW       g0<8,8,1>F      0x021c00b8
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 184, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g117<1>UW       g0<8,8,1>F      0x021c00b7
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 183, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g40<1>UW        g0<8,8,1>F      0x021c00b6
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 182, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g17<1>UW        g0<8,8,1>F      0x021c00b9
                            data MsgDesc: (*** invalid DP DC0 message type value 16 , 185, 0) mlen 1 rlen 1          { align1 WE_all 1Q };
send(8)         g9<1>UW         g19<8,8,1>UD    0x0843e102
                            sampler MsgDesc: ld2dms SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g7<8,8,1>UD     0x1085e102
                            sampler MsgDesc: ld2dms SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g9<1>UW         g21<8,8,1>UD    0x0443d002
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g11<8,8,1>UD    0x0885d002
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g5<8,8,1>UD     0x0c4b0001
                            sampler MsgDesc: gather4_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x168d0001
                            sampler MsgDesc: gather4_c SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         null<1>UW       g2<8,8,1>UD     0x060b5e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0xe) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x060b6e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0xe) mlen 3 rlen 0 { align1 2Q };
(+f1.0) send(8) null<1>UW       g7<8,8,1>UD     0x0a026003
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 3, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g9<8,8,1>UD     0x14025003
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 3, SIMD16, Mask = 0x0) mlen 10 rlen 0 { align1 1H };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0a434001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g12<8,8,1>UD    0x0a434102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(8)         g22<1>UD        g10<8,8,1>UD    0x041a0138
                            urb MsgDesc: 19 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g21<1>UD        g10<8,8,1>UD    0x041a0338
                            urb MsgDesc: 51 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g65<1>UD        g10<8,8,1>UD    0x041a0538
                            urb MsgDesc: 83 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g10<1>UD        g10<8,8,1>UD    0x041a0738
                            urb MsgDesc: 115 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g65<1>UD        g11<8,8,1>UD    0x041a0238
                            urb MsgDesc: 35 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g10<1>UD        g11<8,8,1>UD    0x041a0438
                            urb MsgDesc: 67 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g11<1>UD        g11<8,8,1>UD    0x041a0638
                            urb MsgDesc: 99 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g8<1>UD         g7<8,8,1>UD     0x041a0048
                            urb MsgDesc: 4 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x0a4a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
(+f1.0) send(8) null<1>UW       g12<8,8,1>UD    0x04009701
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, add) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g20<8,8,1>UD    0x08008701
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, add) mlen 4 rlen 0 { align1 1H };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0c4b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g13<8,8,1>UD    0x0c4b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g10<8,8,1>UD    0x064a8203
                            sampler MsgDesc: gather4 SIMD8 Surface = 3 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(16)        g26<1>UW        g2<8,8,1>UD     0x0a8c8203
                            sampler MsgDesc: gather4 SIMD16 Surface = 3 Sampler = 2 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g11<8,8,1>UD    0x08425102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g19<8,8,1>UD    0x10845102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x02306801
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD8, Mask = 0x8) mlen 1 rlen 3 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x04605801
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD16, Mask = 0x8) mlen 2 rlen 6 { align1 1H };
send(8)         g8<1>UD         g7<8,8,1>UD     0x043a0128
                            urb MsgDesc: 18 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x104b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g20<8,8,1>UD    0x104b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 8 rlen 4 { align1 1Q };
send(8)         g8<1>UD         g20<8,8,1>UD    0x044a0138
                            urb MsgDesc: 19 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g20<8,8,1>UD    0x044a0338
                            urb MsgDesc: 51 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g16<1>UD        g20<8,8,1>UD    0x044a0538
                            urb MsgDesc: 83 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g20<1>UD        g20<8,8,1>UD    0x044a0738
                            urb MsgDesc: 115 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g14<1>UD        g22<8,8,1>UD    0x044a0238
                            urb MsgDesc: 35 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g18<1>UD        g22<8,8,1>UD    0x044a0438
                            urb MsgDesc: 67 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g22<1>UD        g22<8,8,1>UD    0x044a0638
                            urb MsgDesc: 99 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x04420004
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g26<8,8,1>UD    0x08840004
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         null<1>UW       g1<8,8,1>UD     0x100b5001
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0x0) mlen 8 rlen 0 { align1 1Q };
send(8)         null<1>UW       g1<8,8,1>UD     0x100b6001
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0x0) mlen 8 rlen 0 { align1 2Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x0c4b2000
                            sampler MsgDesc: gather4_po_c SIMD8 Surface = 0 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g13<1>UD        g39<8,8,1>UD    0x041a0058
                            urb MsgDesc: 5 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g10<8,8,1>UD    0x041a0068
                            urb MsgDesc: 6 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g3<8,8,1>UD     0x041a0078
                            urb MsgDesc: 7 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g3<8,8,1>UD     0x041a0088
                            urb MsgDesc: 8 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g3<8,8,1>UD     0x041a0098
                            urb MsgDesc: 9 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g3<8,8,1>UD     0x041a00a8
                            urb MsgDesc: 10 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g3<8,8,1>UD     0x041a00b8
                            urb MsgDesc: 11 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g4<1>UD         g3<8,8,1>UD     0x041a00c8
                            urb MsgDesc: 12 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a00d8
                            urb MsgDesc: 13 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a00e8
                            urb MsgDesc: 14 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a00f8
                            urb MsgDesc: 15 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0108
                            urb MsgDesc: 16 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0118
                            urb MsgDesc: 17 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0148
                            urb MsgDesc: 20 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0158
                            urb MsgDesc: 21 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0168
                            urb MsgDesc: 22 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0178
                            urb MsgDesc: 23 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0188
                            urb MsgDesc: 24 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0198
                            urb MsgDesc: 25 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a01a8
                            urb MsgDesc: 26 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a01b8
                            urb MsgDesc: 27 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a01c8
                            urb MsgDesc: 28 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a01d8
                            urb MsgDesc: 29 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a01e8
                            urb MsgDesc: 30 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a01f8
                            urb MsgDesc: 31 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g3<1>UD         g2<8,8,1>UD     0x041a0208
                            urb MsgDesc: 32 SIMD8 read per-slot mlen 2 rlen 1 { align1 1Q };
send(8)         g38<1>UW        g38<8,8,1>UD    0x084a8405
                            sampler MsgDesc: gather4 SIMD8 Surface = 5 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(8)         g46<1>UW        g23<8,8,1>UD    0x064a8304
                            sampler MsgDesc: gather4 SIMD8 Surface = 4 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
send(8)         g28<1>UW        g28<8,8,1>UD    0x064a8506
                            sampler MsgDesc: gather4 SIMD8 Surface = 6 Sampler = 5 mlen 3 rlen 4 { align1 1Q };
send(8)         g12<1>UW        g23<8,8,1>UD    0x064a8607
                            sampler MsgDesc: gather4 SIMD8 Surface = 7 Sampler = 6 mlen 3 rlen 4 { align1 1Q };
send(8)         g12<1>UW        g32<8,8,1>UD    0x084a8708
                            sampler MsgDesc: gather4 SIMD8 Surface = 8 Sampler = 7 mlen 4 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g13<8,8,1>UD    0x064a8809
                            sampler MsgDesc: gather4 SIMD8 Surface = 9 Sampler = 8 mlen 3 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x084b090a
                            sampler MsgDesc: gather4_c SIMD8 Surface = 10 Sampler = 9 mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x0a4b0a0b
                            sampler MsgDesc: gather4_c SIMD8 Surface = 11 Sampler = 10 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x084b0b0c
                            sampler MsgDesc: gather4_c SIMD8 Surface = 12 Sampler = 11 mlen 4 rlen 4 { align1 1Q };
send(16)        g30<1>UW        g73<8,8,1>UD    0x0a8c8304
                            sampler MsgDesc: gather4 SIMD16 Surface = 4 Sampler = 3 mlen 5 rlen 8 { align1 1H };
send(16)        g40<1>UW        g2<8,8,1>UD     0x0e8c8405
                            sampler MsgDesc: gather4 SIMD16 Surface = 5 Sampler = 4 mlen 7 rlen 8 { align1 1H };
send(16)        g5<1>UW         g33<8,8,1>UD    0x0a8c8506
                            sampler MsgDesc: gather4 SIMD16 Surface = 6 Sampler = 5 mlen 5 rlen 8 { align1 1H };
send(16)        g32<1>UW        g55<8,8,1>UD    0x0a8c8607
                            sampler MsgDesc: gather4 SIMD16 Surface = 7 Sampler = 6 mlen 5 rlen 8 { align1 1H };
send(16)        g30<1>UW        g23<8,8,1>UD    0x0e8c8708
                            sampler MsgDesc: gather4 SIMD16 Surface = 8 Sampler = 7 mlen 7 rlen 8 { align1 1H };
send(16)        g5<1>UW         g40<8,8,1>UD    0x0a8c8809
                            sampler MsgDesc: gather4 SIMD16 Surface = 9 Sampler = 8 mlen 5 rlen 8 { align1 1H };
send(16)        g38<1>UW        g67<8,8,1>UD    0x0e8d090a
                            sampler MsgDesc: gather4_c SIMD16 Surface = 10 Sampler = 9 mlen 7 rlen 8 { align1 1H };
send(16)        g38<1>UW        g2<8,8,1>UD     0x128d0a0b
                            sampler MsgDesc: gather4_c SIMD16 Surface = 11 Sampler = 10 mlen 9 rlen 8 { align1 1H };
send(16)        g10<1>UW        g39<8,8,1>UD    0x0e8d0b0c
                            sampler MsgDesc: gather4_c SIMD16 Surface = 12 Sampler = 11 mlen 7 rlen 8 { align1 1H };
send(8)         g2<1>UW         g6<8,8,1>UD     0x0e4b2000
                            sampler MsgDesc: gather4_po_c SIMD8 Surface = 0 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g67<1>UW        g2<8,8,1>UD     0x0410bbfe
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 254, SIMD8, imin) mlen 2 rlen 1 { align1 1Q };
send(8)         g13<1>UW        g4<8,8,1>UD     0x06495002
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 2, SIMD16, Mask = 0x0) mlen 3 rlen 4 { align1 1Q };
send(8)         g9<1>UW         g7<8,8,1>UD     0x06496002
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 2, SIMD8, Mask = 0x0) mlen 3 rlen 4 { align1 2Q };
send(8)         null<1>UW       g4<8,8,1>UD     0x0e0b5003
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 3, SIMD16, Mask = 0x0) mlen 7 rlen 0 { align1 1Q };
send(8)         null<1>UW       g4<8,8,1>UD     0x0e0b6003
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 3, SIMD8, Mask = 0x0) mlen 7 rlen 0 { align1 2Q };
(+f1.0) send(8) g3<1>UW         g10<8,8,1>UD    0x0410b701
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, add) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g5<1>UW         g10<8,8,1>UD    0x0410bd01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, umin) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g6<1>UW         g10<8,8,1>UD    0x0410bc01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, umax) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g7<1>UW         g10<8,8,1>UD    0x0410b101
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, and) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g9<1>UW         g10<8,8,1>UD    0x0410b301
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, xor) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g10<1>UW        g10<8,8,1>UD    0x0410b401
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, mov) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g11<1>UW        g11<8,8,1>UD    0x0610be01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, cmpwr) mlen 3 rlen 1 { align1 1Q };
(+f1.0) send(16) g3<1>UW        g19<8,8,1>UD    0x0820a701
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, add) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g7<1>UW        g19<8,8,1>UD    0x0820ad01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, umin) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g9<1>UW        g19<8,8,1>UD    0x0820ac01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, umax) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g11<1>UW       g19<8,8,1>UD    0x0820a101
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, and) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g15<1>UW       g19<8,8,1>UD    0x0820a301
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, xor) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g17<1>UW       g19<8,8,1>UD    0x0820a401
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, mov) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g19<1>UW       g21<8,8,1>UD    0x0c20ae01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, cmpwr) mlen 6 rlen 2 { align1 1H };
send(8)         null<1>F        g16<8,8,1>UD    0x0e0a8057
                            urb MsgDesc: 5 SIMD8 write per-slot masked mlen 7 rlen 0 { align1 1Q };
send(8)         g6<1>UD         g18<8,8,1>UD    0x043a0318
                            urb MsgDesc: 49 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g9<1>UD         g18<8,8,1>UD    0x043a0518
                            urb MsgDesc: 81 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g12<1>UD        g18<8,8,1>UD    0x043a0718
                            urb MsgDesc: 113 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g15<1>UD        g18<8,8,1>UD    0x043a0918
                            urb MsgDesc: 145 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g11<1>UD        g23<8,8,1>UD    0x043a0218
                            urb MsgDesc: 33 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g14<1>UD        g23<8,8,1>UD    0x043a0418
                            urb MsgDesc: 65 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g17<1>UD        g23<8,8,1>UD    0x043a0618
                            urb MsgDesc: 97 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g20<1>UD        g23<8,8,1>UD    0x043a0818
                            urb MsgDesc: 129 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         null<1>F        g12<8,8,1>UD    0x0c0a8227
                            urb MsgDesc: 34 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g13<8,8,1>UD    0x0c0a8237
                            urb MsgDesc: 35 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g14<8,8,1>UD    0x0c0a8247
                            urb MsgDesc: 36 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g15<8,8,1>UD    0x0c0a8257
                            urb MsgDesc: 37 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g16<8,8,1>UD    0x0c0a8267
                            urb MsgDesc: 38 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g17<8,8,1>UD    0x0c0a8277
                            urb MsgDesc: 39 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g18<8,8,1>UD    0x0c0a8287
                            urb MsgDesc: 40 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g19<8,8,1>UD    0x0c0a8297
                            urb MsgDesc: 41 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g20<8,8,1>UD    0x0c0a82a7
                            urb MsgDesc: 42 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g21<8,8,1>UD    0x0c0a82b7
                            urb MsgDesc: 43 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g22<8,8,1>UD    0x0c0a82c7
                            urb MsgDesc: 44 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g23<8,8,1>UD    0x0c0a82d7
                            urb MsgDesc: 45 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g24<8,8,1>UD    0x0c0a82e7
                            urb MsgDesc: 46 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g25<8,8,1>UD    0x0c0a82f7
                            urb MsgDesc: 47 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g26<8,8,1>UD    0x0c0a8307
                            urb MsgDesc: 48 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g27<8,8,1>UD    0x0c0a8317
                            urb MsgDesc: 49 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g28<8,8,1>UD    0x0c0a8327
                            urb MsgDesc: 50 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g29<8,8,1>UD    0x0c0a8337
                            urb MsgDesc: 51 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g30<8,8,1>UD    0x0c0a8347
                            urb MsgDesc: 52 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>UD    0x0c0a8357
                            urb MsgDesc: 53 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g32<8,8,1>UD    0x0c0a8367
                            urb MsgDesc: 54 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g33<8,8,1>UD    0x0c0a8377
                            urb MsgDesc: 55 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g34<8,8,1>UD    0x0c0a8387
                            urb MsgDesc: 56 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g35<8,8,1>UD    0x0c0a8397
                            urb MsgDesc: 57 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g36<8,8,1>UD    0x0c0a83a7
                            urb MsgDesc: 58 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g37<8,8,1>UD    0x0c0a83b7
                            urb MsgDesc: 59 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g38<8,8,1>UD    0x0c0a83c7
                            urb MsgDesc: 60 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g39<8,8,1>UD    0x0c0a83d7
                            urb MsgDesc: 61 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g40<8,8,1>UD    0x0c0a83e7
                            urb MsgDesc: 62 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         null<1>F        g41<8,8,1>UD    0x0c0a83f7
                            urb MsgDesc: 63 SIMD8 write per-slot masked mlen 6 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x10434001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g15<8,8,1>UD    0x10434102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 8 rlen 4 { align1 1Q };
send(8)         g16<1>UD        g16<8,8,1>UD    0x044a0148
                            urb MsgDesc: 20 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g38<8,8,1>UD    0x084a8404
                            sampler MsgDesc: gather4 SIMD8 Surface = 4 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(8)         g46<1>UW        g23<8,8,1>UD    0x064a8303
                            sampler MsgDesc: gather4 SIMD8 Surface = 3 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
send(8)         g28<1>UW        g28<8,8,1>UD    0x064a8505
                            sampler MsgDesc: gather4 SIMD8 Surface = 5 Sampler = 5 mlen 3 rlen 4 { align1 1Q };
send(8)         g12<1>UW        g23<8,8,1>UD    0x064a8606
                            sampler MsgDesc: gather4 SIMD8 Surface = 6 Sampler = 6 mlen 3 rlen 4 { align1 1Q };
send(8)         g12<1>UW        g32<8,8,1>UD    0x084a8707
                            sampler MsgDesc: gather4 SIMD8 Surface = 7 Sampler = 7 mlen 4 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g13<8,8,1>UD    0x064a8808
                            sampler MsgDesc: gather4 SIMD8 Surface = 8 Sampler = 8 mlen 3 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x084b0909
                            sampler MsgDesc: gather4_c SIMD8 Surface = 9 Sampler = 9 mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x0a4b0a0a
                            sampler MsgDesc: gather4_c SIMD8 Surface = 10 Sampler = 10 mlen 5 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x084b0b0b
                            sampler MsgDesc: gather4_c SIMD8 Surface = 11 Sampler = 11 mlen 4 rlen 4 { align1 1Q };
send(8)         null<1>UW       g1<8,8,1>UD     0x080b5e09
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 9, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g1<8,8,1>UD     0x080b6e09
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 9, SIMD8, Mask = 0xe) mlen 4 rlen 0 { align1 2Q };
send(8)         g2<1>UD         g15<8,8,1>UD    0x043a0048
                            urb MsgDesc: 4 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g12<1>UD        g15<8,8,1>UD    0x043a0058
                            urb MsgDesc: 5 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0068
                            urb MsgDesc: 6 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0078
                            urb MsgDesc: 7 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0088
                            urb MsgDesc: 8 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0098
                            urb MsgDesc: 9 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a00a8
                            urb MsgDesc: 10 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a00b8
                            urb MsgDesc: 11 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a00c8
                            urb MsgDesc: 12 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a00d8
                            urb MsgDesc: 13 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a00e8
                            urb MsgDesc: 14 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a00f8
                            urb MsgDesc: 15 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0108
                            urb MsgDesc: 16 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0118
                            urb MsgDesc: 17 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0138
                            urb MsgDesc: 19 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0148
                            urb MsgDesc: 20 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0158
                            urb MsgDesc: 21 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0168
                            urb MsgDesc: 22 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0178
                            urb MsgDesc: 23 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0188
                            urb MsgDesc: 24 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0198
                            urb MsgDesc: 25 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a01a8
                            urb MsgDesc: 26 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a01b8
                            urb MsgDesc: 27 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a01c8
                            urb MsgDesc: 28 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a01d8
                            urb MsgDesc: 29 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a01e8
                            urb MsgDesc: 30 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a01f8
                            urb MsgDesc: 31 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g2<1>UD         g2<8,8,1>UD     0x043a0208
                            urb MsgDesc: 32 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         null<1>F        g11<8,8,1>F     0x140a0047
                            urb MsgDesc: 4 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g31<8,8,1>F     0x140a0087
                            urb MsgDesc: 8 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q };
send(8)         null<1>F        g118<8,8,1>F    0x940a0087
                            urb MsgDesc: 8 SIMD8 write per-slot mlen 10 rlen 0 { align1 1Q EOT };
send(8)         g14<1>UW        g11<8,8,1>UD    0x0a4b0202
                            sampler MsgDesc: gather4_c SIMD8 Surface = 2 Sampler = 2 mlen 5 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x0c4b0303
                            sampler MsgDesc: gather4_c SIMD8 Surface = 3 Sampler = 3 mlen 6 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g24<8,8,1>UD    0x084b0404
                            sampler MsgDesc: gather4_c SIMD8 Surface = 4 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(8)         g24<1>UW        g2<8,8,1>UD     0x06423203
                            sampler MsgDesc: sample_c SIMD8 Surface = 3 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(16)        g19<1>UW        g27<8,8,1>UD    0x0c843203
                            sampler MsgDesc: sample_c SIMD16 Surface = 3 Sampler = 2 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x06422303
                            sampler MsgDesc: sample_l SIMD8 Surface = 3 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04009d01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, umin) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04009c01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, umax) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04009101
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, and) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04009201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, or) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04009301
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, xor) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04009401
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, mov) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g9<8,8,1>UD     0x06009e01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, cmpwr) mlen 3 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08008d01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, umin) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08008c01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, umax) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08008101
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, and) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08008201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, or) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08008301
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, xor) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08008401
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, mov) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g14<8,8,1>UD    0x0c008e01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, cmpwr) mlen 6 rlen 0 { align1 1H };
send(8)         g11<1>UD        g17<8,8,1>UD    0x043a0338
                            urb MsgDesc: 51 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g14<1>UD        g17<8,8,1>UD    0x043a0538
                            urb MsgDesc: 83 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g17<1>UD        g17<8,8,1>UD    0x043a0738
                            urb MsgDesc: 115 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g9<1>UD         g18<8,8,1>UD    0x043a0038
                            urb MsgDesc: 3 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g12<1>UD        g18<8,8,1>UD    0x043a0238
                            urb MsgDesc: 35 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g15<1>UD        g18<8,8,1>UD    0x043a0438
                            urb MsgDesc: 67 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g18<1>UD        g18<8,8,1>UD    0x043a0638
                            urb MsgDesc: 99 SIMD8 read per-slot mlen 2 rlen 3 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x08424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g9<1>UW         g5<8,8,1>UD     0x04420002
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g13<1>UW        g7<8,8,1>UD     0x08840002
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0419a501
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, inc) mlen 2 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0419b501
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, inc) mlen 2 rlen 1 { align1 2Q };
send(8)         null<1>UW       g20<8,8,1>UD    0x06098101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, and) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, and) mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g19<8,8,1>UD    0x06098201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, or) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, or) mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g19<8,8,1>UD    0x06098301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, xor) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, xor) mlen 3 rlen 0 { align1 2Q };
send(8)         g22<1>UD        g32<8,8,1>UD    0x02280238
                            urb MsgDesc: 35 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g24<1>UD        g32<8,8,1>UD    0x02280438
                            urb MsgDesc: 67 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g26<1>UD        g32<8,8,1>UD    0x02280638
                            urb MsgDesc: 99 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g28<1>UD        g32<8,8,1>UD    0x02280248
                            urb MsgDesc: 36 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g30<1>UD        g32<8,8,1>UD    0x02280448
                            urb MsgDesc: 68 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g32<1>UD        g32<8,8,1>UD    0x02280648
                            urb MsgDesc: 100 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g22<1>UD        g33<8,8,1>UD    0x02280258
                            urb MsgDesc: 37 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g24<1>UD        g33<8,8,1>UD    0x02280458
                            urb MsgDesc: 69 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g26<1>UD        g33<8,8,1>UD    0x02280658
                            urb MsgDesc: 101 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g22<1>UD        g34<8,8,1>UD    0x02280268
                            urb MsgDesc: 38 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g24<1>UD        g34<8,8,1>UD    0x02280468
                            urb MsgDesc: 70 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g26<1>UD        g34<8,8,1>UD    0x02280668
                            urb MsgDesc: 102 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g24<1>UD        g35<8,8,1>UD    0x02280478
                            urb MsgDesc: 71 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g35<8,8,1>UD    0x02280278
                            urb MsgDesc: 39 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g26<1>UD        g35<8,8,1>UD    0x02280678
                            urb MsgDesc: 103 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g24<1>UD        g36<8,8,1>UD    0x02280688
                            urb MsgDesc: 104 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g36<8,8,1>UD    0x02280288
                            urb MsgDesc: 40 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g36<8,8,1>UD    0x02280488
                            urb MsgDesc: 72 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g8<1>UD         g37<8,8,1>UD    0x02280298
                            urb MsgDesc: 41 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g37<8,8,1>UD    0x02280498
                            urb MsgDesc: 73 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g24<1>UD        g37<8,8,1>UD    0x02280698
                            urb MsgDesc: 105 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g38<8,8,1>UD    0x022802a8
                            urb MsgDesc: 42 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g38<8,8,1>UD    0x022804a8
                            urb MsgDesc: 74 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g24<1>UD        g38<8,8,1>UD    0x022806a8
                            urb MsgDesc: 106 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g39<8,8,1>UD    0x022802b8
                            urb MsgDesc: 43 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g39<8,8,1>UD    0x022804b8
                            urb MsgDesc: 75 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g24<1>UD        g39<8,8,1>UD    0x022806b8
                            urb MsgDesc: 107 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g40<8,8,1>UD    0x022802c8
                            urb MsgDesc: 44 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g40<8,8,1>UD    0x022804c8
                            urb MsgDesc: 76 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g40<8,8,1>UD    0x022806c8
                            urb MsgDesc: 108 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g41<8,8,1>UD    0x022802d8
                            urb MsgDesc: 45 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g41<8,8,1>UD    0x022804d8
                            urb MsgDesc: 77 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g41<8,8,1>UD    0x022806d8
                            urb MsgDesc: 109 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g42<8,8,1>UD    0x022802e8
                            urb MsgDesc: 46 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g42<8,8,1>UD    0x022804e8
                            urb MsgDesc: 78 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g42<8,8,1>UD    0x022806e8
                            urb MsgDesc: 110 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g43<8,8,1>UD    0x022802f8
                            urb MsgDesc: 47 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g43<8,8,1>UD    0x022804f8
                            urb MsgDesc: 79 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g22<1>UD        g43<8,8,1>UD    0x022806f8
                            urb MsgDesc: 111 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g44<8,8,1>UD    0x02280308
                            urb MsgDesc: 48 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g44<8,8,1>UD    0x02280508
                            urb MsgDesc: 80 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g44<8,8,1>UD    0x02280708
                            urb MsgDesc: 112 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g45<8,8,1>UD    0x02280318
                            urb MsgDesc: 49 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g45<8,8,1>UD    0x02280518
                            urb MsgDesc: 81 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g45<8,8,1>UD    0x02280718
                            urb MsgDesc: 113 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g46<8,8,1>UD    0x02280328
                            urb MsgDesc: 50 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g46<8,8,1>UD    0x02280528
                            urb MsgDesc: 82 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g46<8,8,1>UD    0x02280728
                            urb MsgDesc: 114 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g47<8,8,1>UD    0x02280338
                            urb MsgDesc: 51 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g47<8,8,1>UD    0x02280538
                            urb MsgDesc: 83 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g47<8,8,1>UD    0x02280738
                            urb MsgDesc: 115 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g48<8,8,1>UD    0x02280348
                            urb MsgDesc: 52 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g48<8,8,1>UD    0x02280548
                            urb MsgDesc: 84 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g48<8,8,1>UD    0x02280748
                            urb MsgDesc: 116 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g49<8,8,1>UD    0x02280358
                            urb MsgDesc: 53 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g49<8,8,1>UD    0x02280558
                            urb MsgDesc: 85 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g49<8,8,1>UD    0x02280758
                            urb MsgDesc: 117 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g50<8,8,1>UD    0x02280368
                            urb MsgDesc: 54 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g50<8,8,1>UD    0x02280568
                            urb MsgDesc: 86 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g50<8,8,1>UD    0x02280768
                            urb MsgDesc: 118 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g53<8,8,1>UD    0x02280378
                            urb MsgDesc: 55 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g53<8,8,1>UD    0x02280578
                            urb MsgDesc: 87 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g53<8,8,1>UD    0x02280778
                            urb MsgDesc: 119 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g54<8,8,1>UD    0x02280388
                            urb MsgDesc: 56 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g54<8,8,1>UD    0x02280588
                            urb MsgDesc: 88 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g54<8,8,1>UD    0x02280788
                            urb MsgDesc: 120 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g55<8,8,1>UD    0x02280398
                            urb MsgDesc: 57 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g55<8,8,1>UD    0x02280598
                            urb MsgDesc: 89 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g55<8,8,1>UD    0x02280798
                            urb MsgDesc: 121 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g56<8,8,1>UD    0x022803a8
                            urb MsgDesc: 58 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g56<8,8,1>UD    0x022805a8
                            urb MsgDesc: 90 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g56<8,8,1>UD    0x022807a8
                            urb MsgDesc: 122 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g57<8,8,1>UD    0x022803b8
                            urb MsgDesc: 59 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g57<8,8,1>UD    0x022805b8
                            urb MsgDesc: 91 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g57<8,8,1>UD    0x022807b8
                            urb MsgDesc: 123 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g58<8,8,1>UD    0x022803c8
                            urb MsgDesc: 60 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g58<8,8,1>UD    0x022805c8
                            urb MsgDesc: 92 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g58<8,8,1>UD    0x022807c8
                            urb MsgDesc: 124 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g59<8,8,1>UD    0x022803d8
                            urb MsgDesc: 61 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g59<8,8,1>UD    0x022805d8
                            urb MsgDesc: 93 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g59<8,8,1>UD    0x022807d8
                            urb MsgDesc: 125 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g60<8,8,1>UD    0x022803e8
                            urb MsgDesc: 62 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g60<8,8,1>UD    0x022805e8
                            urb MsgDesc: 94 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g60<8,8,1>UD    0x022807e8
                            urb MsgDesc: 126 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g61<8,8,1>UD    0x022803f8
                            urb MsgDesc: 63 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g61<8,8,1>UD    0x022805f8
                            urb MsgDesc: 95 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g61<8,8,1>UD    0x022807f8
                            urb MsgDesc: 127 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g10<1>UD        g62<8,8,1>UD    0x02280408
                            urb MsgDesc: 64 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g62<8,8,1>UD    0x02280608
                            urb MsgDesc: 96 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g14<1>UD        g62<8,8,1>UD    0x02280808
                            urb MsgDesc: 128 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g8<1>UD         g63<8,8,1>UD    0x02280218
                            urb MsgDesc: 33 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g10<1>UD        g63<8,8,1>UD    0x02280418
                            urb MsgDesc: 65 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g12<1>UD        g63<8,8,1>UD    0x02280618
                            urb MsgDesc: 97 SIMD8 read mlen 1 rlen 2        { align1 1Q };
send(8)         g14<1>UD        g63<8,8,1>UD    0x02280818
                            urb MsgDesc: 129 SIMD8 read mlen 1 rlen 2       { align1 1Q };
send(8)         g29<1>UW        g18<8,8,1>UD    0x04420008
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g35<1>UW        g18<8,8,1>UD    0x04420109
                            sampler MsgDesc: sample SIMD8 Surface = 9 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(8)         g41<1>UW        g18<8,8,1>UD    0x0442020a
                            sampler MsgDesc: sample SIMD8 Surface = 10 Sampler = 2 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g18<8,8,1>UD    0x0442030b
                            sampler MsgDesc: sample SIMD8 Surface = 11 Sampler = 3 mlen 2 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g18<8,8,1>UD    0x0442040c
                            sampler MsgDesc: sample SIMD8 Surface = 12 Sampler = 4 mlen 2 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g18<8,8,1>UD    0x0442050d
                            sampler MsgDesc: sample SIMD8 Surface = 13 Sampler = 5 mlen 2 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g18<8,8,1>UD    0x0442060e
                            sampler MsgDesc: sample SIMD8 Surface = 14 Sampler = 6 mlen 2 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x0442070f
                            sampler MsgDesc: sample SIMD8 Surface = 15 Sampler = 7 mlen 2 rlen 4 { align1 1Q };
send(16)        g32<1>UW        g22<8,8,1>UD    0x08840008
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g42<1>UW        g22<8,8,1>UD    0x08840109
                            sampler MsgDesc: sample SIMD16 Surface = 9 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(16)        g60<1>UW        g22<8,8,1>UD    0x0884020a
                            sampler MsgDesc: sample SIMD16 Surface = 10 Sampler = 2 mlen 4 rlen 8 { align1 1H };
send(16)        g70<1>UW        g22<8,8,1>UD    0x0884030b
                            sampler MsgDesc: sample SIMD16 Surface = 11 Sampler = 3 mlen 4 rlen 8 { align1 1H };
send(16)        g78<1>UW        g22<8,8,1>UD    0x0884040c
                            sampler MsgDesc: sample SIMD16 Surface = 12 Sampler = 4 mlen 4 rlen 8 { align1 1H };
send(16)        g86<1>UW        g22<8,8,1>UD    0x0884050d
                            sampler MsgDesc: sample SIMD16 Surface = 13 Sampler = 5 mlen 4 rlen 8 { align1 1H };
send(16)        g94<1>UW        g22<8,8,1>UD    0x0884060e
                            sampler MsgDesc: sample SIMD16 Surface = 14 Sampler = 6 mlen 4 rlen 8 { align1 1H };
send(16)        g52<1>UW        g22<8,8,1>UD    0x0884070f
                            sampler MsgDesc: sample SIMD16 Surface = 15 Sampler = 7 mlen 4 rlen 8 { align1 1H };
send(8)         g16<1>UW        g42<8,8,1>UD    0x06422101
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(8)         g20<1>UW        g42<8,8,1>UD    0x06422202
                            sampler MsgDesc: sample_l SIMD8 Surface = 2 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(8)         g29<1>UW        g42<8,8,1>UD    0x06422404
                            sampler MsgDesc: sample_l SIMD8 Surface = 4 Sampler = 4 mlen 3 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g42<8,8,1>UD    0x06422606
                            sampler MsgDesc: sample_l SIMD8 Surface = 6 Sampler = 6 mlen 3 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g42<8,8,1>UD    0x06422707
                            sampler MsgDesc: sample_l SIMD8 Surface = 7 Sampler = 7 mlen 3 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g16<8,8,1>UD    0x044a0058
                            urb MsgDesc: 5 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0068
                            urb MsgDesc: 6 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0078
                            urb MsgDesc: 7 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0088
                            urb MsgDesc: 8 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0098
                            urb MsgDesc: 9 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a00a8
                            urb MsgDesc: 10 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a00b8
                            urb MsgDesc: 11 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a00c8
                            urb MsgDesc: 12 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a00d8
                            urb MsgDesc: 13 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a00e8
                            urb MsgDesc: 14 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a00f8
                            urb MsgDesc: 15 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0108
                            urb MsgDesc: 16 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0118
                            urb MsgDesc: 17 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0158
                            urb MsgDesc: 21 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0168
                            urb MsgDesc: 22 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0178
                            urb MsgDesc: 23 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0188
                            urb MsgDesc: 24 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0198
                            urb MsgDesc: 25 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a01a8
                            urb MsgDesc: 26 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a01b8
                            urb MsgDesc: 27 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a01c8
                            urb MsgDesc: 28 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a01d8
                            urb MsgDesc: 29 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a01e8
                            urb MsgDesc: 30 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a01f8
                            urb MsgDesc: 31 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g12<1>UD        g2<8,8,1>UD     0x044a0208
                            urb MsgDesc: 32 SIMD8 read per-slot mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0a425102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g27<1>UW        g7<8,8,1>UD     0x14845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(16)        g35<1>UW        g17<8,8,1>UD    0x14845102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
send(8)         null<1>F        g120<8,8,1>F    0x8c0a0117
                            urb MsgDesc: 17 SIMD8 write per-slot mlen 6 rlen 0 { align1 1Q EOT };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380128
                            urb MsgDesc: 18 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380138
                            urb MsgDesc: 19 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380148
                            urb MsgDesc: 20 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380158
                            urb MsgDesc: 21 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380168
                            urb MsgDesc: 22 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380178
                            urb MsgDesc: 23 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380188
                            urb MsgDesc: 24 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x02380198
                            urb MsgDesc: 25 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x023801a8
                            urb MsgDesc: 26 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x023801b8
                            urb MsgDesc: 27 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x023801c8
                            urb MsgDesc: 28 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x023801d8
                            urb MsgDesc: 29 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x023801e8
                            urb MsgDesc: 30 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         g11<1>UD        g1<8,8,1>UD     0x023801f8
                            urb MsgDesc: 31 SIMD8 read mlen 1 rlen 3        { align1 1Q };
send(8)         null<1>UW       g18<8,8,1>UD    0x08098701
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, add) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099701
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, add) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g18<8,8,1>UD    0x08098d01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, umin) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099d01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, umin) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g18<8,8,1>UD    0x08098101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, and) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, and) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g18<8,8,1>UD    0x08098201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, or) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, or) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g18<8,8,1>UD    0x08098301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, xor) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, xor) mlen 4 rlen 0 { align1 2Q };
send(8)         g11<1>UD        g13<8,8,1>UD    0x042a0058
                            urb MsgDesc: 5 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a0068
                            urb MsgDesc: 6 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a0078
                            urb MsgDesc: 7 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a0088
                            urb MsgDesc: 8 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a0098
                            urb MsgDesc: 9 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a00a8
                            urb MsgDesc: 10 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a00b8
                            urb MsgDesc: 11 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a00c8
                            urb MsgDesc: 12 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a00d8
                            urb MsgDesc: 13 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a00e8
                            urb MsgDesc: 14 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a00f8
                            urb MsgDesc: 15 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a0108
                            urb MsgDesc: 16 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g11<8,8,1>UD    0x042a0118
                            urb MsgDesc: 17 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a0158
                            urb MsgDesc: 21 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a0168
                            urb MsgDesc: 22 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a0178
                            urb MsgDesc: 23 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a0188
                            urb MsgDesc: 24 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a0198
                            urb MsgDesc: 25 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a01a8
                            urb MsgDesc: 26 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a01b8
                            urb MsgDesc: 27 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a01c8
                            urb MsgDesc: 28 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a01d8
                            urb MsgDesc: 29 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a01e8
                            urb MsgDesc: 30 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a01f8
                            urb MsgDesc: 31 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UD         g3<8,8,1>UD     0x042a0208
                            urb MsgDesc: 32 SIMD8 read per-slot mlen 2 rlen 2 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x024ab102
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x024ab203
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 3 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g14<8,8,1>UD    0x024ab304
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 4 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x024ab405
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 5 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g22<8,8,1>UD    0x024ab506
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 6 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g26<8,8,1>UD    0x028cb102
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 2 Sampler = 1 mlen 1 rlen 8 { align1 1H };
send(16)        g28<1>UW        g27<8,8,1>UD    0x028cb203
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 3 Sampler = 2 mlen 1 rlen 8 { align1 1H };
send(16)        g36<1>UW        g44<8,8,1>UD    0x028cb304
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 4 Sampler = 3 mlen 1 rlen 8 { align1 1H };
send(16)        g2<1>UW         g53<8,8,1>UD    0x028cb506
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 6 Sampler = 5 mlen 1 rlen 8 { align1 1H };
send(16)        g44<1>UW        g52<8,8,1>UD    0x028cb405
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 5 Sampler = 4 mlen 1 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x0a4b0102
                            sampler MsgDesc: gather4_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g11<8,8,1>UD    0x0a4b0203
                            sampler MsgDesc: gather4_c SIMD8 Surface = 3 Sampler = 2 mlen 5 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x0c4b0304
                            sampler MsgDesc: gather4_c SIMD8 Surface = 4 Sampler = 3 mlen 6 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g24<8,8,1>UD    0x084b0405
                            sampler MsgDesc: gather4_c SIMD8 Surface = 5 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g26<8,8,1>UD    0x128d0203
                            sampler MsgDesc: gather4_c SIMD16 Surface = 3 Sampler = 2 mlen 9 rlen 8 { align1 1H };
send(16)        g10<1>UW        g53<8,8,1>UD    0x128d0102
                            sampler MsgDesc: gather4_c SIMD16 Surface = 2 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(16)        g26<1>UW        g35<8,8,1>UD    0x168d0304
                            sampler MsgDesc: gather4_c SIMD16 Surface = 4 Sampler = 3 mlen 11 rlen 8 { align1 1H };
send(16)        g34<1>UW        g46<8,8,1>UD    0x0e8d0405
                            sampler MsgDesc: gather4_c SIMD16 Surface = 5 Sampler = 4 mlen 7 rlen 8 { align1 1H };
send(8)         g124<1>UW       g9<8,8,1>UD     0x0c4b0000
                            sampler MsgDesc: gather4_c SIMD8 Surface = 0 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
