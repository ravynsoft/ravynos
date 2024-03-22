send(8)         null<1>F        g113<4>F        0x8a08c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 5 rlen 0 { align16 1Q EOT };
send(8)         null<1>F        g113<4>F        0x8608c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         g124<1>UW       g13<8,8,1>UD    0x08427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g10<8,8,1>UD    0x10847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g50<1>D         g51<4>UD        0x02194013
                            urb MsgDesc: 2 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g12<4>UD        0x04094019
                            urb MsgDesc: 3 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g13<4>UD        0x04094011
                            urb MsgDesc: 2 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g12<4>UD        0x04094009
                            urb MsgDesc: 1 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g12<4>UD        0x04094001
                            urb MsgDesc: 0 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g14<1>D         g15<4>UD        0x0219400b
                            urb MsgDesc: 1 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g13<1>D         g12<4>UD        0x02194003
                            urb MsgDesc: 0 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>UW       g12<4,4,1>UD    0x02008004
                            gateway MsgDesc: (barrier msg) mlen 1 rlen 0    { align1 WE_all 1Q };
send(8)         null<1>F        g13<4>UD        0x0208c003
                            urb MsgDesc: 0 read OWord interleave complete mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        g126<4>F        0x84080001
                            urb MsgDesc: 0 write OWord mlen 2 rlen 0        { align16 1Q EOT };
send(8)         g12<1>D         g114<4>F        0x02107000
                            sampler MsgDesc: ld SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x0a094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 5 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x82084000
                            urb MsgDesc: 0 write HWord interleave mlen 1 rlen 0 { align16 1Q EOT };
send(8)         g0<1>F          g125<4>F        0x060a80ff
                            data MsgDesc: ( DC OWORD dual block write, 255, 0) mlen 3 rlen 0 { align16 1Q };
send(8)         g41<1>F         g126<4>F        0x041880ff
                            data MsgDesc: ( DC OWORD dual block read, 255, 0) mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x8e08c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 7 rlen 0 { align16 1Q EOT };
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
send(16)        g48<1>UW        g10<8,8,1>UD    0x1485e001
                            sampler MsgDesc: ld2dms SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g124<1>UW       g11<8,8,1>UD    0x06420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g16<8,8,1>UD    0x0c840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g7<8,8,1>UD     0x144a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 10 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     a0<0,1,0>UD     0x00000200
                            sampler MsgDesc: indirect                       { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x084a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x064a8002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x0a8c8002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0a4a6001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g12<8,8,1>UD    0x0a4a6102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x128c6001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(16)        g10<1>UW        g20<8,8,1>UD    0x128c6102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(8)         g10<1>D         g114<4>F        0x0411e000
                            sampler MsgDesc: ld2dms SIMD4x2 Surface = 0 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        g9<4>UD         0x04094021
                            urb MsgDesc: 4 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g9<4>UD         0x02088003
                            urb MsgDesc: 0 read OWord complete mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x06094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 3 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x1a084000
                            urb MsgDesc: 0 write HWord interleave mlen 13 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x8a08c030
                            urb MsgDesc: 6 write HWord interleave complete mlen 5 rlen 0 { align16 1Q EOT };
send(8)         g5<1>UW         g21<8,8,1>UD    0x02420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g26<8,8,1>UD    0x04840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g12<1>D         g114<4>F        0x0210a000
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g8<1>UW         g10<8,8,1>UD    0x0242a001
                            sampler MsgDesc: resinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0c4b1002
                            sampler MsgDesc: gather4_po SIMD8 Surface = 2 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g22<1>UW        g19<8,8,1>UD    0x0484a001
                            sampler MsgDesc: resinfo SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(16)        g18<1>UW        g7<8,8,1>UD     0x168d1002
                            sampler MsgDesc: gather4_po SIMD16 Surface = 2 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         null<1>F        g9<4>UD         0x04094029
                            urb MsgDesc: 5 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x084a8002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g14<1>UW        g7<8,8,1>UD     0x0e8c8002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g14<1>D         g114<4>F        0x06191001
                            sampler MsgDesc: gather4_po SIMD4x2 Surface = 1 Sampler = 0 mlen 3 rlen 1 { align16 1Q };
send(8)         g3<1>D          g114<4>F        0x0210a101
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 1 Sampler = 1 mlen 1 rlen 1 { align16 1Q };
send(8)         g5<1>D          g114<4>F        0x0210a202
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 2 Sampler = 2 mlen 1 rlen 1 { align16 1Q };
send(8)         g7<1>D          g114<4>F        0x0210a303
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 3 Sampler = 3 mlen 1 rlen 1 { align16 1Q };
send(8)         g9<1>D          g114<4>F        0x0210a404
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 4 Sampler = 4 mlen 1 rlen 1 { align16 1Q };
send(8)         g11<1>D         g114<4>F        0x0210a505
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 5 Sampler = 5 mlen 1 rlen 1 { align16 1Q };
send(8)         g13<1>D         g114<4>F        0x0210a606
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 6 Sampler = 6 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g54<4>UD        0x04094109
                            urb MsgDesc: 33 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094111
                            urb MsgDesc: 34 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094119
                            urb MsgDesc: 35 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094121
                            urb MsgDesc: 36 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094129
                            urb MsgDesc: 37 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094131
                            urb MsgDesc: 38 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094139
                            urb MsgDesc: 39 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094141
                            urb MsgDesc: 40 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094149
                            urb MsgDesc: 41 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094151
                            urb MsgDesc: 42 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094159
                            urb MsgDesc: 43 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094161
                            urb MsgDesc: 44 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094169
                            urb MsgDesc: 45 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094171
                            urb MsgDesc: 46 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094179
                            urb MsgDesc: 47 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094181
                            urb MsgDesc: 48 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094189
                            urb MsgDesc: 49 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094191
                            urb MsgDesc: 50 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x04094199
                            urb MsgDesc: 51 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g53<4>UD        0x040941a1
                            urb MsgDesc: 52 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g54<4>UD        0x040941a9
                            urb MsgDesc: 53 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g55<4>UD        0x040941b1
                            urb MsgDesc: 54 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g56<4>UD        0x040941b9
                            urb MsgDesc: 55 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g59<4>UD        0x040941c1
                            urb MsgDesc: 56 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g60<4>UD        0x040941c9
                            urb MsgDesc: 57 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g61<4>UD        0x040941d1
                            urb MsgDesc: 58 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g62<4>UD        0x040941d9
                            urb MsgDesc: 59 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g63<4>UD        0x040941e1
                            urb MsgDesc: 60 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g64<4>UD        0x040941e9
                            urb MsgDesc: 61 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g65<4>UD        0x040941f1
                            urb MsgDesc: 62 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g66<4>UD        0x040941f9
                            urb MsgDesc: 63 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g71<4>UD        0x04094031
                            urb MsgDesc: 6 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g72<4>UD        0x04094039
                            urb MsgDesc: 7 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g73<4>UD        0x04094041
                            urb MsgDesc: 8 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g74<4>UD        0x04094049
                            urb MsgDesc: 9 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g75<4>UD        0x04094051
                            urb MsgDesc: 10 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g76<4>UD        0x04094059
                            urb MsgDesc: 11 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g77<4>UD        0x04094061
                            urb MsgDesc: 12 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g78<4>UD        0x04094069
                            urb MsgDesc: 13 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g79<4>UD        0x04094071
                            urb MsgDesc: 14 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g80<4>UD        0x04094079
                            urb MsgDesc: 15 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g81<4>UD        0x04094081
                            urb MsgDesc: 16 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g82<4>UD        0x04094089
                            urb MsgDesc: 17 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g83<4>UD        0x04094091
                            urb MsgDesc: 18 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g84<4>UD        0x04094099
                            urb MsgDesc: 19 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g85<4>UD        0x040940a1
                            urb MsgDesc: 20 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g86<4>UD        0x040940a9
                            urb MsgDesc: 21 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g87<4>UD        0x040940b1
                            urb MsgDesc: 22 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g88<4>UD        0x040940b9
                            urb MsgDesc: 23 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g89<4>UD        0x040940c1
                            urb MsgDesc: 24 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g90<4>UD        0x040940c9
                            urb MsgDesc: 25 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g91<4>UD        0x040940d1
                            urb MsgDesc: 26 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g92<4>UD        0x040940d9
                            urb MsgDesc: 27 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g93<4>UD        0x040940e1
                            urb MsgDesc: 28 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g94<4>UD        0x040940e9
                            urb MsgDesc: 29 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g95<4>UD        0x040940f1
                            urb MsgDesc: 30 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g96<4>UD        0x040940f9
                            urb MsgDesc: 31 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g8<4>UD         0x04094101
                            urb MsgDesc: 32 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g17<1>D         g16<4>UD        0x0219418b
                            urb MsgDesc: 49 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g22<1>D         g21<4>UD        0x0219428b
                            urb MsgDesc: 81 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g27<1>D         g26<4>UD        0x0219438b
                            urb MsgDesc: 113 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g32<1>D         g31<4>UD        0x0219448b
                            urb MsgDesc: 145 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g39<1>D         g38<4>UD        0x02194093
                            urb MsgDesc: 18 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g42<1>D         g41<4>UD        0x0219410b
                            urb MsgDesc: 33 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g47<1>D         g46<4>UD        0x0219420b
                            urb MsgDesc: 65 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g52<1>D         g51<4>UD        0x0219430b
                            urb MsgDesc: 97 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g57<1>D         g56<4>UD        0x0219440b
                            urb MsgDesc: 129 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g71<1>D         g3<4>UD         0x02194103
                            urb MsgDesc: 32 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g74<1>D         g3<4>UD         0x02194203
                            urb MsgDesc: 64 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g77<1>D         g3<4>UD         0x02194303
                            urb MsgDesc: 96 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g80<1>D         g3<4>UD         0x02194403
                            urb MsgDesc: 128 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x9208c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 9 rlen 0 { align16 1Q EOT };
send(8)         g5<1>UW         g3<8,8,1>UD     0x02427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g8<1>UW         g5<8,8,1>UD     0x04847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x1a084030
                            urb MsgDesc: 6 write HWord interleave mlen 13 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x9608c060
                            urb MsgDesc: 12 write HWord interleave complete mlen 11 rlen 0 { align16 1Q EOT };
send(8)         g58<1>D         g59<4>UD        0x0219401b
                            urb MsgDesc: 3 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g65<1>D         g66<4>UD        0x02194023
                            urb MsgDesc: 4 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g72<1>D         g73<4>UD        0x0219402b
                            urb MsgDesc: 5 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g79<1>D         g80<4>UD        0x02194033
                            urb MsgDesc: 6 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g86<1>D         g87<4>UD        0x0219403b
                            urb MsgDesc: 7 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g93<1>D         g94<4>UD        0x02194043
                            urb MsgDesc: 8 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g100<1>D        g101<4>UD       0x0219404b
                            urb MsgDesc: 9 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g108<4>UD       0x02194053
                            urb MsgDesc: 10 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g13<1>D         g14<4>UD        0x0219405b
                            urb MsgDesc: 11 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g20<1>D         g21<4>UD        0x02194063
                            urb MsgDesc: 12 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g27<1>D         g28<4>UD        0x0219406b
                            urb MsgDesc: 13 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g34<1>D         g35<4>UD        0x02194073
                            urb MsgDesc: 14 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g43<1>D         g47<4>UD        0x0219407b
                            urb MsgDesc: 15 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g53<1>D         g54<4>UD        0x02194083
                            urb MsgDesc: 16 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g60<1>D         g61<4>UD        0x0219408b
                            urb MsgDesc: 17 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g74<1>D         g75<4>UD        0x0219409b
                            urb MsgDesc: 19 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g81<1>D         g82<4>UD        0x021940a3
                            urb MsgDesc: 20 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g88<1>D         g89<4>UD        0x021940ab
                            urb MsgDesc: 21 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g95<1>D         g96<4>UD        0x021940b3
                            urb MsgDesc: 22 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g102<1>D        g103<4>UD       0x021940bb
                            urb MsgDesc: 23 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g110<4>UD       0x021940c3
                            urb MsgDesc: 24 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g13<1>D         g14<4>UD        0x021940cb
                            urb MsgDesc: 25 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g20<1>D         g21<4>UD        0x021940d3
                            urb MsgDesc: 26 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g27<1>D         g28<4>UD        0x021940db
                            urb MsgDesc: 27 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g34<1>D         g35<4>UD        0x021940e3
                            urb MsgDesc: 28 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g43<1>D         g47<4>UD        0x021940eb
                            urb MsgDesc: 29 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g53<1>D         g54<4>UD        0x021940f3
                            urb MsgDesc: 30 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g60<1>D         g61<4>UD        0x021940fb
                            urb MsgDesc: 31 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x02429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x04849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g124<1>UW       g10<8,8,1>UD    0x0e434001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
(+f1.0) send(8) g12<1>UW        g2<8,8,1>UD     0x0410b201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, or) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) null<1>UW       g2<8,8,1>UD     0x02009501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, inc) mlen 1 rlen 0 { align1 1Q };
(+f1.0) send(16) g14<1>UW       g16<8,8,1>UD    0x0820a201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, or) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) null<1>UW      g2<8,8,1>UD     0x04008501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, inc) mlen 2 rlen 0 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x08434001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g5<1>UW         g11<8,8,1>UD    0x06495001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 3 rlen 4 { align1 1Q };
send(8)         null<1>UW       g14<8,8,1>UD    0x0e0b5002
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0x0) mlen 7 rlen 0 { align1 1Q };
send(8)         g2<1>UW         g11<8,8,1>UD    0x06496001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 3 rlen 4 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x0e0b6002
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0x0) mlen 7 rlen 0 { align1 2Q };
send(8)         null<1>F        g113<4>F        0x8608c030
                            urb MsgDesc: 6 write HWord interleave complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         g12<1>F         g114<4>F        0x06190001
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 1 Sampler = 0 mlen 3 rlen 1 { align16 1Q };
send(8)         g7<1>UW         g0<8,8,1>UD     0x02200008
                            pixel interp MsgDesc: (persp, per_message_offset, 0x08) mlen 1 rlen 2 { align1 1Q };
send(16)        g9<1>UW         g0<8,8,1>UD     0x02410008
                            pixel interp MsgDesc: (persp, per_message_offset, 0x08) mlen 1 rlen 4 { align1 1H };
send(8)         g2<1>UW         g11<8,8,1>UD    0x0443d001
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g9<8,8,1>UD     0x0843e001
                            sampler MsgDesc: ld2dms SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g15<8,8,1>UD    0x0885d001
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g43<1>UW        g11<8,8,1>UD    0x1085e001
                            sampler MsgDesc: ld2dms SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g11<8,8,1>UD    0x0a4a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g19<8,8,1>UD    0x128c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x9608c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 11 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a4a8002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g7<8,8,1>UD     0x128c8002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x06094008
                            urb MsgDesc: 1 write HWord per-slot interleave mlen 3 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x04084001
                            urb MsgDesc: 0 write OWord interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g14<1>UD        g114<4>F        0x04188001
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 1 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g16<1>.xD       g114<4>F        0x0218b000
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x0a094008
                            urb MsgDesc: 1 write HWord per-slot interleave mlen 5 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x16094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 11 rlen 0 { align16 1Q };
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
send(8)         g2<1>UW         g6<8,8,1>UD     0x064a3001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g9<8,8,1>UD     0x064a3102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0a8c3001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0a8c3102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 5 rlen 8 { align1 1H };
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
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a4b1002
                            sampler MsgDesc: gather4_po SIMD8 Surface = 2 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g35<1>UW        g7<8,8,1>UD     0x128d1002
                            sampler MsgDesc: gather4_po SIMD16 Surface = 2 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         g124<1>UW       g12<8,8,1>UD    0x084b0002
                            sampler MsgDesc: gather4_c SIMD8 Surface = 2 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x0e8d0002
                            sampler MsgDesc: gather4_c SIMD16 Surface = 2 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0e434102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 7 rlen 4 { align1 1Q };
send(8)         g4<1>D          g114<4>F        0x04188003
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 3 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g5<1>D          g114<4>F        0x04188104
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 4 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g8<1>D          g114<4>F        0x04188205
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 5 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g4<8,8,1>UD     0x0c424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g5<8,8,1>UD     0x06427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0c847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g9<1>F          g114<4>F        0x04102000
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 0 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g3<1>.xUW       g1<4>UD         0x0410eb00
                            dp data 1 MsgDesc: ( DC untyped 4x2 atomic op, Surface = 0,  imin) mlen 2 rlen 1 { align16 1Q };
send(8)         g5<1>UW         g4<8,8,1>UD     0x08495001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x08496001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 4 rlen 4 { align1 2Q };
send(8)         g2<1>UW         g9<8,8,1>UD     0x08427002
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g14<8,8,1>UD    0x10847002
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x06423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x0c843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x1a094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 13 rlen 0 { align16 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x08422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g10<8,8,1>UD    0x10842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g16<1>F         g17<4>.xUD      0x02107001
                            sampler MsgDesc: ld SIMD4x2 Surface = 1 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         g9<8,8,1>UD     0x08423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g8<8,8,1>UD     0x10843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g4<1>UW         g0<8,8,1>UD     0x02201000
                            pixel interp MsgDesc: (persp, sample_position, 0x00) mlen 1 rlen 2 { align1 1Q };
send(16)        g6<1>UW         g0<8,8,1>UD     0x02411000
                            pixel interp MsgDesc: (persp, sample_position, 0x00) mlen 1 rlen 4 { align1 1H };
send(8)         g124<1>UW       g14<8,8,1>UD    0x0a4b0002
                            sampler MsgDesc: gather4_c SIMD8 Surface = 2 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x128d0002
                            sampler MsgDesc: gather4_c SIMD16 Surface = 2 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         g2<1>UW         g13<8,8,1>UD    0x06422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g14<8,8,1>UD    0x04429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g14<1>UW        g8<8,8,1>UD     0x0c842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        g14<1>UW        g10<8,8,1>UD    0x08849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x0e094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 7 rlen 0 { align16 1Q };
send(8)         g36<1>D         g35<4>UD        0x0219419b
                            urb MsgDesc: 51 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g41<1>D         g40<4>UD        0x0219429b
                            urb MsgDesc: 83 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g46<1>D         g45<4>UD        0x0219439b
                            urb MsgDesc: 115 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g58<1>D         g57<4>UD        0x0219411b
                            urb MsgDesc: 35 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g63<1>D         g62<4>UD        0x0219421b
                            urb MsgDesc: 67 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g68<1>D         g67<4>UD        0x0219431b
                            urb MsgDesc: 99 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g78<1>D         g19<4>UD        0x02194113
                            urb MsgDesc: 34 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g81<1>D         g19<4>UD        0x02194213
                            urb MsgDesc: 66 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g84<1>D         g19<4>UD        0x02194313
                            urb MsgDesc: 98 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x08842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x06429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g12<8,8,1>UD    0x0c849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x12094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 9 rlen 0 { align16 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x06426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x0c846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g5<8,8,1>UD     0x08425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x10845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x064a8006
                            sampler MsgDesc: gather4 SIMD8 Surface = 6 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g14<8,8,1>UD    0x064a840a
                            sampler MsgDesc: gather4 SIMD8 Surface = 10 Sampler = 4 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x084a8107
                            sampler MsgDesc: gather4 SIMD8 Surface = 7 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g10<8,8,1>UD    0x084a8208
                            sampler MsgDesc: gather4 SIMD8 Surface = 8 Sampler = 2 mlen 4 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g26<8,8,1>UD    0x0a4a8309
                            sampler MsgDesc: gather4 SIMD8 Surface = 9 Sampler = 3 mlen 5 rlen 4 { align1 1Q };
send(16)        g35<1>UW        g2<8,8,1>UD     0x0a8c8006
                            sampler MsgDesc: gather4 SIMD16 Surface = 6 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g18<1>UW        g43<8,8,1>UD    0x0a8c840a
                            sampler MsgDesc: gather4 SIMD16 Surface = 10 Sampler = 4 mlen 5 rlen 8 { align1 1H };
send(16)        g43<1>UW        g7<8,8,1>UD     0x0e8c8107
                            sampler MsgDesc: gather4 SIMD16 Surface = 7 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(16)        g2<1>UW         g51<8,8,1>UD    0x0e8c8208
                            sampler MsgDesc: gather4 SIMD16 Surface = 8 Sampler = 2 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g26<8,8,1>UD    0x128c8309
                            sampler MsgDesc: gather4 SIMD16 Surface = 9 Sampler = 3 mlen 9 rlen 8 { align1 1H };
(+f1.0) send(8) null<1>UW       g7<8,8,1>UD     0x0a026001
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g9<8,8,1>UD     0x14025001
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD16, Mask = 0x0) mlen 10 rlen 0 { align1 1H };
send(8)         g124<1>UW       g10<8,8,1>UD    0x0e4a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g9<8,8,1>UD     0x0a422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x14842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g124<1>UW       g9<8,8,1>UD     0x0a421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x14841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g25<1>UW        g13<8,8,1>UD    0x06195e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xe) mlen 3 rlen 1 { align1 1Q };
send(8)         null<1>UW       g26<8,8,1>UD    0x080b5e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1Q };
send(8)         g11<1>UW        g18<8,8,1>UD    0x06196e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xe) mlen 3 rlen 1 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x080b6e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0xe) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g13<8,8,1>UD    0x06098501
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, inc) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099501
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, inc) mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g26<8,8,1>UD    0x08098c01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, umax) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099c01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, umax) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g26<8,8,1>UD    0x08098401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, mov) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, mov) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g28<8,8,1>UD    0x0a098e01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, cmpwr) mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>UW       g16<8,8,1>UD    0x0a099e01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, cmpwr) mlen 5 rlen 0 { align1 2Q };
send(8)         g124<1>UW       g5<8,8,1>UD     0x064a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x0a8c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g26<8,8,1>UD    0x0c843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g22<1>UW        g22<8,8,1>UD    0x0242a203
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x0242a304
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g30<1>UW        g30<8,8,1>UD    0x0242a405
                            sampler MsgDesc: resinfo SIMD8 Surface = 5 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g34<1>UW        g34<8,8,1>UD    0x0242a506
                            sampler MsgDesc: resinfo SIMD8 Surface = 6 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g20<8,8,1>UD    0x0242a102
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g38<8,8,1>UD    0x0242a607
                            sampler MsgDesc: resinfo SIMD8 Surface = 7 Sampler = 6 mlen 1 rlen 4 { align1 1Q };
send(8)         g42<1>UW        g42<8,8,1>UD    0x0242a708
                            sampler MsgDesc: resinfo SIMD8 Surface = 8 Sampler = 7 mlen 1 rlen 4 { align1 1Q };
send(8)         g46<1>UW        g46<8,8,1>UD    0x0242a809
                            sampler MsgDesc: resinfo SIMD8 Surface = 9 Sampler = 8 mlen 1 rlen 4 { align1 1Q };
send(8)         g50<1>UW        g50<8,8,1>UD    0x0242a90a
                            sampler MsgDesc: resinfo SIMD8 Surface = 10 Sampler = 9 mlen 1 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g54<8,8,1>UD    0x0242aa0b
                            sampler MsgDesc: resinfo SIMD8 Surface = 11 Sampler = 10 mlen 1 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g55<8,8,1>UD    0x0242ab0c
                            sampler MsgDesc: resinfo SIMD8 Surface = 12 Sampler = 11 mlen 1 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g56<8,8,1>UD    0x0242ac0d
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
send(8)         g124<1>UW       g2<8,8,1>UD     0x06424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x04420203
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 2 mlen 2 rlen 4 { align1 1Q };
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
send(16)        g2<1>UW         g26<8,8,1>UD    0x08840203
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 2 mlen 4 rlen 8 { align1 1H };
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
send(8)         g124<1>UW       g2<8,8,1>UD     0x02406001
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 1 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x04805001
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 2 rlen 8 { align1 1H };
send(8)         g29<1>UW        g5<8,8,1>UD     0x0e4b2002
                            sampler MsgDesc: gather4_po_c SIMD8 Surface = 2 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x084a2001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x0e8c2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g124<1>UW       g3<8,8,1>UD     0x044a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g4<8,8,1>UD     0x068c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 3 rlen 8 { align1 1H };
send(8)         g17<1>UW        g12<8,8,1>UD    0x04420003
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g39<8,8,1>UD    0x08840003
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g8<8,8,1>UD     0x064a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g15<8,8,1>UD    0x0a8c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x9a08c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 13 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         g7<8,8,1>UD     0x084a6001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x084a6102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x0e8c6001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c6102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(8)         g108<1>D        g105<4>UD       0x02194223
                            urb MsgDesc: 68 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x02194323
                            urb MsgDesc: 100 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x02194123
                            urb MsgDesc: 36 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x0219412b
                            urb MsgDesc: 37 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x0219422b
                            urb MsgDesc: 69 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x0219432b
                            urb MsgDesc: 101 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x02194133
                            urb MsgDesc: 38 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x02194233
                            urb MsgDesc: 70 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x02194333
                            urb MsgDesc: 102 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x0219413b
                            urb MsgDesc: 39 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x0219423b
                            urb MsgDesc: 71 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x0219433b
                            urb MsgDesc: 103 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x02194143
                            urb MsgDesc: 40 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x02194243
                            urb MsgDesc: 72 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x02194343
                            urb MsgDesc: 104 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x0219414b
                            urb MsgDesc: 41 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x0219424b
                            urb MsgDesc: 73 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x0219434b
                            urb MsgDesc: 105 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x02194153
                            urb MsgDesc: 42 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x02194253
                            urb MsgDesc: 74 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x02194353
                            urb MsgDesc: 106 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x0219415b
                            urb MsgDesc: 43 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x0219425b
                            urb MsgDesc: 75 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x0219435b
                            urb MsgDesc: 107 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x02194163
                            urb MsgDesc: 44 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x02194263
                            urb MsgDesc: 76 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x02194363
                            urb MsgDesc: 108 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x0219416b
                            urb MsgDesc: 45 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x0219426b
                            urb MsgDesc: 77 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x0219436b
                            urb MsgDesc: 109 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x02194173
                            urb MsgDesc: 46 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x02194273
                            urb MsgDesc: 78 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x02194373
                            urb MsgDesc: 110 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x0219417b
                            urb MsgDesc: 47 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x0219427b
                            urb MsgDesc: 79 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x0219437b
                            urb MsgDesc: 111 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x02194183
                            urb MsgDesc: 48 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x02194283
                            urb MsgDesc: 80 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x02194383
                            urb MsgDesc: 112 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x02194193
                            urb MsgDesc: 50 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x02194293
                            urb MsgDesc: 82 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x02194393
                            urb MsgDesc: 114 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021941a3
                            urb MsgDesc: 52 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x021942a3
                            urb MsgDesc: 84 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021943a3
                            urb MsgDesc: 116 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x021941ab
                            urb MsgDesc: 53 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021942ab
                            urb MsgDesc: 85 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021943ab
                            urb MsgDesc: 117 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g107<1>D        g105<4>UD       0x021941b3
                            urb MsgDesc: 54 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021942b3
                            urb MsgDesc: 86 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021943b3
                            urb MsgDesc: 118 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021941bb
                            urb MsgDesc: 55 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x021942bb
                            urb MsgDesc: 87 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021943bb
                            urb MsgDesc: 119 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x021941c3
                            urb MsgDesc: 56 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021942c3
                            urb MsgDesc: 88 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021943c3
                            urb MsgDesc: 120 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021941cb
                            urb MsgDesc: 57 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x021942cb
                            urb MsgDesc: 89 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021943cb
                            urb MsgDesc: 121 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021941d3
                            urb MsgDesc: 58 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021942d3
                            urb MsgDesc: 90 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x021943d3
                            urb MsgDesc: 122 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021941db
                            urb MsgDesc: 59 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x021942db
                            urb MsgDesc: 91 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021943db
                            urb MsgDesc: 123 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021941e3
                            urb MsgDesc: 60 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021942e3
                            urb MsgDesc: 92 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x021943e3
                            urb MsgDesc: 124 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021941eb
                            urb MsgDesc: 61 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x021942eb
                            urb MsgDesc: 93 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021943eb
                            urb MsgDesc: 125 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g108<1>D        g105<4>UD       0x021941f3
                            urb MsgDesc: 62 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g105<4>UD       0x021942f3
                            urb MsgDesc: 94 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021943f3
                            urb MsgDesc: 126 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g109<1>D        g105<4>UD       0x021941fb
                            urb MsgDesc: 63 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g111<1>D        g105<4>UD       0x021942fb
                            urb MsgDesc: 95 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g106<1>D        g105<4>UD       0x021943fb
                            urb MsgDesc: 127 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x02106e01
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x04205e01
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
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
send(8)         g2<1>UW         g12<8,8,1>UD    0x0c4b2002
                            sampler MsgDesc: gather4_po_c SIMD8 Surface = 2 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g15<8,8,1>UD    0x168d2002
                            sampler MsgDesc: gather4_po_c SIMD16 Surface = 2 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         g68<1>.xUW      g65<4>UD        0x0210e500
                            dp data 1 MsgDesc: ( DC untyped 4x2 atomic op, Surface = 0,  inc) mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x124b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 9 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g16<8,8,1>UD    0x124b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 9 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x0a4a2001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g9<8,8,1>UD     0x128c2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         g24<1>F         g25<4>.xUD      0x02107002
                            sampler MsgDesc: ld SIMD4x2 Surface = 2 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g30<1>F         g31<4>.xUD      0x02107003
                            sampler MsgDesc: ld SIMD4x2 Surface = 3 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g35<1>F         g36<4>.xUD      0x02107004
                            sampler MsgDesc: ld SIMD4x2 Surface = 4 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g41<1>F         g42<4>.xUD      0x02107005
                            sampler MsgDesc: ld SIMD4x2 Surface = 5 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g47<1>F         g48<4>.xUD      0x02107006
                            sampler MsgDesc: ld SIMD4x2 Surface = 6 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g53<1>F         g54<4>.xUD      0x02107007
                            sampler MsgDesc: ld SIMD4x2 Surface = 7 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g59<1>F         g60<4>.xUD      0x02107008
                            sampler MsgDesc: ld SIMD4x2 Surface = 8 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g65<1>F         g66<4>.xUD      0x02107009
                            sampler MsgDesc: ld SIMD4x2 Surface = 9 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g71<1>F         g72<4>.xUD      0x0210700a
                            sampler MsgDesc: ld SIMD4x2 Surface = 10 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g77<1>F         g78<4>.xUD      0x0210700b
                            sampler MsgDesc: ld SIMD4x2 Surface = 11 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g83<1>F         g84<4>.xUD      0x0210700c
                            sampler MsgDesc: ld SIMD4x2 Surface = 12 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g89<1>F         g90<4>.xUD      0x0210700d
                            sampler MsgDesc: ld SIMD4x2 Surface = 13 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g5<1>F          g114<4>F        0x04102505
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 5 Sampler = 5 mlen 2 rlen 1 { align16 1Q };
(+f1.0) send(8) g3<1>UW         g3<8,8,1>UD     0x0410b702
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, add) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(16) g4<1>UW        g6<8,8,1>UD     0x0820a702
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, add) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(8) g2<1>UW         g5<8,8,1>UD     0x0210b501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, inc) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(16) g2<1>UW        g7<8,8,1>UD     0x0420a501
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, inc) mlen 2 rlen 2 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x08420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x10840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
(+f1.0) send(8) g4<1>UW         g12<8,8,1>UD    0x0210b502
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, inc) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(16) g5<1>UW        g17<8,8,1>UD    0x0420a502
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, inc) mlen 2 rlen 2 { align1 1H };
send(8)         g2<1>UW         g11<8,8,1>UD    0x084a5001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g15<8,8,1>UD    0x084a5102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g21<8,8,1>UD    0x0e8c5001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g28<8,8,1>UD    0x0e8c5102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(8)         g2<1>UW         g9<8,8,1>UD     0x024ab001
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x028cb001
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 1 Sampler = 0 mlen 1 rlen 8 { align1 1H };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x06026c01
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD8, Mask = 0xc) mlen 3 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x0c025c01
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 1, SIMD16, Mask = 0xc) mlen 6 rlen 0 { align1 1H };
send(8)         g3<1>UW         g8<8,8,1>UD     0x02427002
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g4<1>UW         g26<8,8,1>UD    0x04847002
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 0 mlen 2 rlen 8 { align1 1H };
(+f1.0) send(8) g14<1>UW        g13<8,8,1>UD    0x0410bb02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, imin) mlen 2 rlen 1 { align1 1Q };
send(8)         g15<1>UW        g3<8,8,1>UD     0x02106e02
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD8, Mask = 0xe) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(8) g18<1>UW        g4<8,8,1>UD     0x0410b402
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, mov) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(16) g21<1>UW       g23<8,8,1>UD    0x0820ab02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, imin) mlen 4 rlen 2 { align1 1H };
send(16)        g22<1>UW        g3<8,8,1>UD     0x04205e02
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD16, Mask = 0xe) mlen 2 rlen 2 { align1 1H };
(+f1.0) send(16) g25<1>UW       g6<8,8,1>UD     0x0820a402
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, mov) mlen 4 rlen 2 { align1 1H };
send(8)         g2<1>UW         g3<8,8,1>UD     0x04203000
                            pixel interp MsgDesc: (persp, per_slot_offset, 0x00) mlen 2 rlen 2 { align1 1Q };
send(16)        g2<1>UW         g15<8,8,1>UD    0x08413000
                            pixel interp MsgDesc: (persp, per_slot_offset, 0x00) mlen 4 rlen 4 { align1 1H };
send(1)         g2<1>UW         g2<0,1,0>UW     0x0209c000
                            data MsgDesc: ( DC mfence, 0, 0) mlen 1 rlen 0  { align1 WE_all 1N };
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
(+f1.0) send(8) null<1>UW       g119<8,8,1>UD   0x02009601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, dec) mlen 1 rlen 0 { align1 1Q };
(+f1.0) send(8) g48<1>UW        g119<8,8,1>UD   0x0210b601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, dec) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(16) null<1>UW      g3<8,8,1>UD     0x04008601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, dec) mlen 2 rlen 0 { align1 1H };
(+f1.0) send(16) g97<1>UW       g3<8,8,1>UD     0x0420a601
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, dec) mlen 2 rlen 2 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x064a8004
                            sampler MsgDesc: gather4 SIMD8 Surface = 4 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x084a8105
                            sampler MsgDesc: gather4 SIMD8 Surface = 5 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g10<8,8,1>UD    0x064a8206
                            sampler MsgDesc: gather4 SIMD8 Surface = 6 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0a8c8004
                            sampler MsgDesc: gather4 SIMD16 Surface = 4 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c8105
                            sampler MsgDesc: gather4 SIMD16 Surface = 5 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(16)        g26<1>UW        g34<8,8,1>UD    0x0a8c8206
                            sampler MsgDesc: gather4 SIMD16 Surface = 6 Sampler = 2 mlen 5 rlen 8 { align1 1H };
send(8)         g39<1>.xUW      g36<4>UD        0x0210e600
                            dp data 1 MsgDesc: ( DC untyped 4x2 atomic op, Surface = 0,  dec) mlen 1 rlen 1 { align16 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0c840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x8e08c030
                            urb MsgDesc: 6 write HWord interleave complete mlen 7 rlen 0 { align16 1Q EOT };
send(8)         g6<1>UW         g9<8,8,1>UD     0x02406002
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD8, Mask = 0x0) mlen 1 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g14<8,8,1>UD    0x04805002
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 2, SIMD16, Mask = 0x0) mlen 2 rlen 8 { align1 1H };
send(8)         g5<1>UW         g5<8,8,1>UD     0x04195e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xe) mlen 2 rlen 1 { align1 1Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x060b5e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0xe) mlen 3 rlen 0 { align1 1Q };
send(8)         g7<1>UW         g10<8,8,1>UD    0x04196e01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xe) mlen 2 rlen 1 { align1 2Q };
send(8)         null<1>UW       g12<8,8,1>UD    0x060b6e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0xe) mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>F        g113<4>F        0x1a094030
                            urb MsgDesc: 6 write HWord per-slot interleave mlen 13 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x06094060
                            urb MsgDesc: 12 write HWord per-slot interleave mlen 3 rlen 0 { align16 1Q };
send(8)         g124<1>UW       g10<8,8,1>UD    0x084a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g15<8,8,1>UD    0x0e8c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g13<1>UD        g114<4>F        0x0211d000
                            sampler MsgDesc: ld_mcs SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g3<1>UD         g114<4>F        0x04188005
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 5 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g4<1>UD         g114<4>F        0x04188106
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 6 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g6<1>UD         g114<4>F        0x04188207
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 7 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g9<1>UD         g114<4>F        0x04188308
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 8 Sampler = 3 mlen 2 rlen 1 { align16 1Q };
send(8)         g11<1>UD        g114<4>F        0x04188409
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 9 Sampler = 4 mlen 2 rlen 1 { align16 1Q };
(+f1.0) send(8) null<1>UW       g11<8,8,1>UD    0x0a026002
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x14025002
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 2, SIMD16, Mask = 0x0) mlen 10 rlen 0 { align1 1H };
send(8)         null<1>F        g113<4>F        0x16094060
                            urb MsgDesc: 12 write HWord per-slot interleave mlen 11 rlen 0 { align16 1Q };
send(8)         null<1>UW       g126<8,8,1>UD   0x040a02ff
                            data MsgDesc: ( DC OWORD block write, 255, 2) mlen 2 rlen 0 { align1 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0009
                            data MsgDesc: ( DC OWORD block read, 9, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0001
                            data MsgDesc: ( DC OWORD block read, 1, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0008
                            data MsgDesc: ( DC OWORD block read, 8, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0002
                            data MsgDesc: ( DC OWORD block read, 2, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0007
                            data MsgDesc: ( DC OWORD block read, 7, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0000
                            data MsgDesc: ( DC OWORD block read, 0, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g68<1>UW        g0<8,8,1>F      0x021c0005
                            data MsgDesc: ( DC OWORD block read, 5, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g66<1>UW        g0<8,8,1>F      0x021c0004
                            data MsgDesc: ( DC OWORD block read, 4, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g64<1>UW        g0<8,8,1>F      0x021c0006
                            data MsgDesc: ( DC OWORD block read, 6, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g64<1>UW        g0<8,8,1>F      0x021c0003
                            data MsgDesc: ( DC OWORD block read, 3, 0) mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g2<1>UW         g9<8,8,1>UD     0x0a423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g19<1>UW        g9<8,8,1>UD     0x14843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g124<1>UW       g8<8,8,1>UD     0x08426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g10<8,8,1>UD    0x10846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
(+f1.0) send(8) g14<1>UW        g13<8,8,1>UD    0x0410bd02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD8, umin) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(16) g21<1>UW       g23<8,8,1>UD    0x0820ad02
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 2, SIMD16, umin) mlen 4 rlen 2 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x12424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 9 rlen 4 { align1 1Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x080b5e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0xe) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x080b6e02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0xe) mlen 4 rlen 0 { align1 2Q };
send(8)         g26<1>UW        g20<8,8,1>UD    0x06295c01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xc) mlen 3 rlen 2 { align1 1Q };
send(8)         null<1>UW       g23<8,8,1>UD    0x0a0b5c02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD16, Mask = 0xc) mlen 5 rlen 0 { align1 1Q };
send(8)         g5<1>UW         g43<8,8,1>UD    0x06296c01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xc) mlen 3 rlen 2 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x0a0b6c02
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 2, SIMD8, Mask = 0xc) mlen 5 rlen 0 { align1 2Q };
send(8)         null<1>F        g113<4>F        0x8608c060
                            urb MsgDesc: 12 write HWord interleave complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         g16<8,8,1>UD    0x04495001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0x0) mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g31<8,8,1>UD    0x04496001
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0x0) mlen 2 rlen 4 { align1 2Q };
send(8)         g2<1>UW         g16<8,8,1>UD    0x04295c01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD16, Mask = 0xc) mlen 2 rlen 2 { align1 1Q };
send(8)         g2<1>UW         g31<8,8,1>UD    0x04296c01
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 1, SIMD8, Mask = 0xc) mlen 2 rlen 2 { align1 2Q };
send(8)         g20<1>UW        g16<8,8,1>UD    0x04195e02
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 2, SIMD16, Mask = 0xe) mlen 2 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619a701
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, add) mlen 3 rlen 1 { align1 1Q };
send(8)         g4<1>UW         g31<8,8,1>UD    0x04196e02
                            dp data 1 MsgDesc: ( DC typed surface read, Surface = 2, SIMD8, Mask = 0xe) mlen 2 rlen 1 { align1 2Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619b701
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, add) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619ad01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, umin) mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619bd01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, umin) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619ac01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, umax) mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619bc01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, umax) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619a101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, and) mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619b101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, and) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619a201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, or) mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619b201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, or) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619a301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, xor) mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619b301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, xor) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0619a401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, mov) mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0619b401
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, mov) mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g18<8,8,1>UD    0x0819ae01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, cmpwr) mlen 4 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0819be01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, cmpwr) mlen 4 rlen 1 { align1 2Q };
send(8)         g9<1>UW         g19<8,8,1>UD    0x0843e102
                            sampler MsgDesc: ld2dms SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g7<8,8,1>UD     0x1085e102
                            sampler MsgDesc: ld2dms SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g5<8,8,1>UD     0x0c4b0002
                            sampler MsgDesc: gather4_c SIMD8 Surface = 2 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x168d0002
                            sampler MsgDesc: gather4_c SIMD16 Surface = 2 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         g124<1>UW       g5<8,8,1>UD     0x06425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x0c845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
(+f1.0) send(8) null<1>UW       g9<8,8,1>UD     0x0a026003
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 3, SIMD8, Mask = 0x0) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g11<8,8,1>UD    0x14025003
                            dp data 1 MsgDesc: ( DC untyped surface write, Surface = 3, SIMD16, Mask = 0x0) mlen 10 rlen 0 { align1 1H };
send(8)         null<1>UW       g2<8,8,1>UD     0x060b5e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0xe) mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x060b6e01
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0xe) mlen 3 rlen 0 { align1 2Q };
send(8)         g2<1>UW         g11<8,8,1>UD    0x084a3001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g15<8,8,1>UD    0x084a3102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g22<8,8,1>UD    0x0e8c3001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g29<8,8,1>UD    0x0e8c3102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(16)        g2<1>UD         g26<8,8,1>UD    0x02280302
                            const MsgDesc: (2, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g16<1>UD        g27<8,8,1>UD    0x02280303
                            const MsgDesc: (3, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x0a4a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
(+f1.0) send(8) null<1>UW       g12<8,8,1>UD    0x04009701
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD8, add) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g19<8,8,1>UD    0x08008701
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, add) mlen 4 rlen 0 { align1 1H };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0c4b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g13<8,8,1>UD    0x0c4b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x0443d002
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0885d002
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x02306801
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD8, Mask = 0x8) mlen 1 rlen 3 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x04605801
                            dp data 1 MsgDesc: ( untyped surface read, Surface = 1, SIMD16, Mask = 0x8) mlen 2 rlen 6 { align1 1H };
(+f1.0) send(8) null<1>UW       g2<8,8,1>UD     a0<0,1,0>UD     0x00000200
                            dp data 1 MsgDesc: indirect                     { align1 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x104b4001
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g20<8,8,1>UD    0x104b4102
                            sampler MsgDesc: sample_d_c SIMD8 Surface = 2 Sampler = 1 mlen 8 rlen 4 { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x04420004
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g26<8,8,1>UD    0x08840004
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g11<1>F         g114<4>F        0x06192001
                            sampler MsgDesc: gather4_po_c SIMD4x2 Surface = 1 Sampler = 0 mlen 3 rlen 1 { align16 1Q };
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
send(8)         null<1>UW       g5<8,8,1>UD     0x0e0b5001
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD16, Mask = 0x0) mlen 7 rlen 0 { align1 1Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x0e0b6001
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 1, SIMD8, Mask = 0x0) mlen 7 rlen 0 { align1 2Q };
send(8)         g4<1>F          g114<4>F        0x06190005
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 5 Sampler = 0 mlen 3 rlen 1 { align16 1Q };
send(8)         g5<1>F          g114<4>F        0x06190106
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 6 Sampler = 1 mlen 3 rlen 1 { align16 1Q };
send(8)         g7<1>F          g114<4>F        0x06190207
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 7 Sampler = 2 mlen 3 rlen 1 { align16 1Q };
send(8)         g10<1>F         g114<4>F        0x06190308
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 8 Sampler = 3 mlen 3 rlen 1 { align16 1Q };
send(8)         g12<1>F         g114<4>F        0x06190409
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 9 Sampler = 4 mlen 3 rlen 1 { align16 1Q };
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
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x08008d01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, umin) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x08008c01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, umax) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x08008101
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, and) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x08008201
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, or) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x08008301
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, xor) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x08008401
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, mov) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x0c008e01
                            dp data 1 MsgDesc: ( DC untyped atomic op, Surface = 1, SIMD16, cmpwr) mlen 6 rlen 0 { align1 1H };
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
send(8)         g5<1>F          g114<4>F        0x04102101
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 1 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g6<1>F          g114<4>F        0x04102202
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 2 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g7<1>F          g114<4>F        0x04102303
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 3 Sampler = 3 mlen 2 rlen 1 { align16 1Q };
send(8)         g8<1>F          g114<4>F        0x04102404
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 4 Sampler = 4 mlen 2 rlen 1 { align16 1Q };
send(8)         g10<1>F         g114<4>F        0x04102606
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 6 Sampler = 6 mlen 2 rlen 1 { align16 1Q };
send(8)         g11<1>F         g114<4>F        0x04102707
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 7 Sampler = 7 mlen 2 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g10<8,8,1>UD    0x084a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g18<8,8,1>UD    0x0e8c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x0e094030
                            urb MsgDesc: 6 write HWord per-slot interleave mlen 7 rlen 0 { align16 1Q };
send(8)         null<1>UW       g20<8,8,1>UD    0x08098701
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, add) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099701
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, add) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g20<8,8,1>UD    0x08098d01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, umin) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099d01
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, umin) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g20<8,8,1>UD    0x08098101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, and) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099101
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, and) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g20<8,8,1>UD    0x08098201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, or) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099201
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, or) mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g20<8,8,1>UD    0x08098301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD16, xor) mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099301
                            dp data 1 MsgDesc: ( DC typed atomic, Surface = 1, SIMD8, xor) mlen 4 rlen 0 { align1 2Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x084b0006
                            sampler MsgDesc: gather4_c SIMD8 Surface = 6 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x0a4b0107
                            sampler MsgDesc: gather4_c SIMD8 Surface = 7 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g11<8,8,1>UD    0x0a4b0208
                            sampler MsgDesc: gather4_c SIMD8 Surface = 8 Sampler = 2 mlen 5 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x0c4b0309
                            sampler MsgDesc: gather4_c SIMD8 Surface = 9 Sampler = 3 mlen 6 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g24<8,8,1>UD    0x084b040a
                            sampler MsgDesc: gather4_c SIMD8 Surface = 10 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g26<8,8,1>UD    0x128d0208
                            sampler MsgDesc: gather4_c SIMD16 Surface = 8 Sampler = 2 mlen 9 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0e8d0006
                            sampler MsgDesc: gather4_c SIMD16 Surface = 6 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g26<1>UW        g35<8,8,1>UD    0x168d0309
                            sampler MsgDesc: gather4_c SIMD16 Surface = 9 Sampler = 3 mlen 11 rlen 8 { align1 1H };
send(16)        g10<1>UW        g53<8,8,1>UD    0x128d0107
                            sampler MsgDesc: gather4_c SIMD16 Surface = 7 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(16)        g34<1>UW        g46<8,8,1>UD    0x0e8d040a
                            sampler MsgDesc: gather4_c SIMD16 Surface = 10 Sampler = 4 mlen 7 rlen 8 { align1 1H };
send(8)         null<1>UW       g9<8,8,1>UD     0x0e0b5003
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 3, SIMD16, Mask = 0x0) mlen 7 rlen 0 { align1 1Q };
send(8)         null<1>UW       g15<8,8,1>UD    0x0e0b6003
                            dp data 1 MsgDesc: ( DC typed surface write, Surface = 3, SIMD8, Mask = 0x0) mlen 7 rlen 0 { align1 2Q };
