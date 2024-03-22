send(8)         null<1>F        g113<4>F        0x8608c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         null<1>F        g113<4>F        0x8a08c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 5 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         g8<8,8,1>UD     0x08427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g14<8,8,1>UD    0x10847001
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
send(8)         g0<1>F          g125<4>F        0x060a80ff
                            data MsgDesc: ( DC OWORD dual block write, 255, 0) mlen 3 rlen 0 { align16 1Q };
send(8)         g41<1>F         g126<4>F        0x041880ff
                            data MsgDesc: ( DC OWORD dual block read, 255, 0) mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x8e08c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 7 rlen 0 { align16 1Q EOT };
send(8)         g124<1>UW       g11<8,8,1>UD    0x06420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g16<8,8,1>UD    0x0c840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g7<8,8,1>UD     0x144a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 10 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x084a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g38<1>UD        g87<4>.xUD      0x02107000
                            sampler MsgDesc: ld SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
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
send(8)         null<1>F        g113<4>F        0x0a094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 5 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x82084000
                            urb MsgDesc: 0 write HWord interleave mlen 1 rlen 0 { align16 1Q EOT };
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
send(16)        g7<1>UW         g21<8,8,1>UD    0x04840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g12<1>D         g114<4>F        0x0210a000
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g9<4>UD         0x04094029
                            urb MsgDesc: 5 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g2<1>UW         g10<8,8,1>UD    0x06427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g18<8,8,1>UD    0x0c847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x0c424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
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
send(8)         null<1>F        g113<4>F        0x9208c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 9 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         g2<8,8,1>UD     0x04429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g13<1>UD        g3<8,8,1>UD     0x02280301
                            const MsgDesc: (1, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g11<1>D         g114<4>F        0x06191001
                            sampler MsgDesc: gather4_po SIMD4x2 Surface = 1 Sampler = 0 mlen 3 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x08842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g5<1>UW         g3<8,8,1>UD     0x02427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g8<1>UW         g5<8,8,1>UD     0x04847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g124<1>UW       g9<8,8,1>UD     0x08421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g14<8,8,1>UD    0x10841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x02429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x04849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g2<1>UW         g9<8,8,1>UD     0x0242a001
                            sampler MsgDesc: resinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0484a001
                            sampler MsgDesc: resinfo SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g2<1>UW         g21<8,8,1>UD    0x08426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g2<8,8,1>UD     0x10846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g2<1>UW         g8<8,8,1>UD     0x06426001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g20<8,8,1>UD    0x0c846001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x1a084030
                            urb MsgDesc: 6 write HWord interleave mlen 13 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x9608c060
                            urb MsgDesc: 12 write HWord interleave complete mlen 11 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         g6<8,8,1>UD     0x06423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0c843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x0a094008
                            urb MsgDesc: 1 write HWord per-slot interleave mlen 5 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x04084001
                            urb MsgDesc: 0 write OWord interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g9<1>F          g114<4>F        0x04188001
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 1 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x8608c030
                            urb MsgDesc: 6 write HWord interleave complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         g124<1>UW       g11<8,8,1>UD    0x0a4a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g19<8,8,1>UD    0x128c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x9608c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 11 rlen 0 { align16 1Q EOT };
send(8)         g9<1>UW         g2<8,8,1>UD     0x0242a102
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g16<8,8,1>UD    0x06426102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g29<1>UW        g37<8,8,1>UD    0x0484a102
                            sampler MsgDesc: resinfo SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         g15<8,8,1>UD    0x0c846102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g5<1>UW         g17<8,8,1>UD    0x06427102
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(8)         g9<1>UW         g20<8,8,1>UD    0x0843e001
                            sampler MsgDesc: ld2dms SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g21<1>UW        g7<8,8,1>UD     0x0c847102
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(16)        g29<1>UW        g13<8,8,1>UD    0x1085e001
                            sampler MsgDesc: ld2dms SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g13<8,8,1>UD    0x0a43e001
                            sampler MsgDesc: ld2dms SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g10<8,8,1>UD    0x1485e001
                            sampler MsgDesc: ld2dms SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         g13<8,8,1>UD    0x0643d001
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0c85d001
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g9<1>UW         g19<8,8,1>UD    0x0443d001
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g11<8,8,1>UD    0x0885d001
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g16<1>.xD       g114<4>F        0x0218b000
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x16094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 11 rlen 0 { align16 1Q };
send(8)         g2<1>UW         g6<8,8,1>UD     0x064a3001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g9<8,8,1>UD     0x064a3102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0a8c3001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0a8c3102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 5 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x06421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x0c841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g2<1>UW         g11<8,8,1>UD    0x06425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g14<8,8,1>UD    0x06425102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g21<8,8,1>UD    0x0c845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(16)        g10<1>UW        g27<8,8,1>UD    0x0c845102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g9<8,8,1>UD     0x08420001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x10840001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g11<1>UD        g114<4>F        0x0211d000
                            sampler MsgDesc: ld_mcs SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x0e424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g16<1>F         g17<4>.xUD      0x02107001
                            sampler MsgDesc: ld SIMD4x2 Surface = 1 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g6<1>UW         g9<8,8,1>UD     0x08426102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g15<8,8,1>UD    0x10846102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g4<1>D          g114<4>F        0x04188003
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 3 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g5<1>D          g114<4>F        0x04188104
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 4 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g8<1>D          g114<4>F        0x04188205
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 5 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g10<8,8,1>UD    0x08422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g15<8,8,1>UD    0x10842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g7<8,8,1>UD     a0<0,1,0>UD     0x00000200
                            sampler MsgDesc: indirect                       { align1 1Q };
send(8)         g28<1>D         g29<4>UD        0x0219401b
                            urb MsgDesc: 3 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g36<1>D         g37<4>UD        0x02194023
                            urb MsgDesc: 4 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g43<1>D         g44<4>UD        0x0219402b
                            urb MsgDesc: 5 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g3<1>.xUW       g1<4>UD         0x0411bb00
                            data MsgDesc: ( DC untyped atomic, 0,  imin) mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x1a094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 13 rlen 0 { align16 1Q };
send(8)         g124<1>UW       g5<8,8,1>UD     0x04420304
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 3 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x08840304
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 3 mlen 4 rlen 8 { align1 1H };
send(8)         g2<1>UW         g9<8,8,1>UD     0x08423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g16<1>UW        g8<8,8,1>UD     0x10843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g124<1>UW       g10<8,8,1>UD    0x0a421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g9<8,8,1>UD     0x14841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         g13<8,8,1>UD    0x06422001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g14<1>UW        g8<8,8,1>UD     0x0c842001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x0e094000
                            urb MsgDesc: 0 write HWord per-slot interleave mlen 7 rlen 0 { align16 1Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x06424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x06429001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g12<8,8,1>UD    0x0c849001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x04427001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x08847001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
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
(+f1.0) send(8) null<1>UW       g7<8,8,1>UD     0x0a036001
                            data MsgDesc: ( DC untyped surface write, 1, 32) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g9<8,8,1>UD     0x14035001
                            data MsgDesc: ( DC untyped surface write, 1, 16) mlen 10 rlen 0 { align1 1H };
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
send(16)        g3<1>UD         g5<8,8,1>UD     0x02280302
                            const MsgDesc: (2, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g25<1>UW        g13<8,8,1>UD    0x06194e01
                            render MsgDesc: typed surface read MsgCtrl = 0x14 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         null<1>UW       g26<8,8,1>UD    0x080b4e01
                            render MsgDesc: typed surface write MsgCtrl = 0x14 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         g40<1>UW        g18<8,8,1>UD    0x06196e01
                            render MsgDesc: typed surface read MsgCtrl = 0x46 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x080b6e01
                            render MsgDesc: typed surface write MsgCtrl = 0x46 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06098501
                            render MsgDesc: typed atomic op MsgCtrl = 0x5 Surface = 1 mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099501
                            render MsgDesc: typed atomic op MsgCtrl = 0x21 Surface = 1 mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098c01
                            render MsgDesc: typed atomic op MsgCtrl = 0x12 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099c01
                            render MsgDesc: typed atomic op MsgCtrl = 0x28 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098401
                            render MsgDesc: typed atomic op MsgCtrl = 0x4 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099401
                            render MsgDesc: typed atomic op MsgCtrl = 0x20 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x0a098e01
                            render MsgDesc: typed atomic op MsgCtrl = 0x14 Surface = 1 mlen 5 rlen 0 { align1 1Q };
send(8)         null<1>UW       g16<8,8,1>UD    0x0a099e01
                            render MsgDesc: typed atomic op MsgCtrl = 0x30 Surface = 1 mlen 5 rlen 0 { align1 2Q };
send(8)         g2<1>UW         g6<8,8,1>UD     0x04423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g8<8,8,1>UD     0x04423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x08843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g5<8,8,1>UD     0x064a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g7<8,8,1>UD     0x0a8c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g26<8,8,1>UD    0x0c843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         g5<1>UW         g15<8,8,1>UD    0x04420203
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 2 mlen 2 rlen 4 { align1 1Q };
send(8)         g9<1>UW         g15<8,8,1>UD    0x04420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(16)        g7<1>UW         g27<8,8,1>UD    0x08840203
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 2 mlen 4 rlen 8 { align1 1H };
send(16)        g15<1>UW        g27<8,8,1>UD    0x08840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(8)         g22<1>UW        g22<8,8,1>UD    0x0242a203
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x0242a304
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g30<1>UW        g30<8,8,1>UD    0x0242a405
                            sampler MsgDesc: resinfo SIMD8 Surface = 5 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g34<1>UW        g34<8,8,1>UD    0x0242a506
                            sampler MsgDesc: resinfo SIMD8 Surface = 6 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
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
send(8)         g124<1>UW       g11<8,8,1>UD    0x12424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 9 rlen 4 { align1 1Q };
send(8)         g17<1>F         g114<4>F        0x04102000
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 0 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x04420405
                            sampler MsgDesc: sample SIMD8 Surface = 5 Sampler = 4 mlen 2 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g14<8,8,1>UD    0x04420506
                            sampler MsgDesc: sample SIMD8 Surface = 6 Sampler = 5 mlen 2 rlen 4 { align1 1Q };
send(8)         g18<1>UW        g18<8,8,1>UD    0x04420607
                            sampler MsgDesc: sample SIMD8 Surface = 7 Sampler = 6 mlen 2 rlen 4 { align1 1Q };
send(8)         g22<1>UW        g22<8,8,1>UD    0x04420708
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 7 mlen 2 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x04420809
                            sampler MsgDesc: sample SIMD8 Surface = 9 Sampler = 8 mlen 2 rlen 4 { align1 1Q };
send(8)         g30<1>UW        g30<8,8,1>UD    0x0442090a
                            sampler MsgDesc: sample SIMD8 Surface = 10 Sampler = 9 mlen 2 rlen 4 { align1 1Q };
send(8)         g34<1>UW        g34<8,8,1>UD    0x04420a0b
                            sampler MsgDesc: sample SIMD8 Surface = 11 Sampler = 10 mlen 2 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g38<8,8,1>UD    0x04420b0c
                            sampler MsgDesc: sample SIMD8 Surface = 12 Sampler = 11 mlen 2 rlen 4 { align1 1Q };
send(8)         g42<1>UW        g42<8,8,1>UD    0x04420c0d
                            sampler MsgDesc: sample SIMD8 Surface = 13 Sampler = 12 mlen 2 rlen 4 { align1 1Q };
send(8)         g46<1>UW        g46<8,8,1>UD    0x04420d0e
                            sampler MsgDesc: sample SIMD8 Surface = 14 Sampler = 13 mlen 2 rlen 4 { align1 1Q };
send(8)         g50<1>UW        g50<8,8,1>UD    0x04420e0f
                            sampler MsgDesc: sample SIMD8 Surface = 15 Sampler = 14 mlen 2 rlen 4 { align1 1Q };
send(8)         g54<1>UW        g54<8,8,1>UD    0x04420f10
                            sampler MsgDesc: sample SIMD8 Surface = 16 Sampler = 15 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840405
                            sampler MsgDesc: sample SIMD16 Surface = 5 Sampler = 4 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g18<8,8,1>UD    0x08840506
                            sampler MsgDesc: sample SIMD16 Surface = 6 Sampler = 5 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840607
                            sampler MsgDesc: sample SIMD16 Surface = 7 Sampler = 6 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g18<8,8,1>UD    0x08840708
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 7 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840809
                            sampler MsgDesc: sample SIMD16 Surface = 9 Sampler = 8 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g18<8,8,1>UD    0x0884090a
                            sampler MsgDesc: sample SIMD16 Surface = 10 Sampler = 9 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840a0b
                            sampler MsgDesc: sample SIMD16 Surface = 11 Sampler = 10 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g18<8,8,1>UD    0x08840b0c
                            sampler MsgDesc: sample SIMD16 Surface = 12 Sampler = 11 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840c0d
                            sampler MsgDesc: sample SIMD16 Surface = 13 Sampler = 12 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g18<8,8,1>UD    0x08840d0e
                            sampler MsgDesc: sample SIMD16 Surface = 14 Sampler = 13 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x08840e0f
                            sampler MsgDesc: sample SIMD16 Surface = 15 Sampler = 14 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         g18<8,8,1>UD    0x08840f10
                            sampler MsgDesc: sample SIMD16 Surface = 16 Sampler = 15 mlen 4 rlen 8 { align1 1H };
send(8)         g6<1>UW         g2<8,8,1>UD     0x02420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g2<8,8,1>UD     0x04840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x02416001
                            data MsgDesc: ( DC untyped surface read, 1, 32) mlen 1 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g2<8,8,1>UD     0x04815001
                            data MsgDesc: ( DC untyped surface read, 1, 16) mlen 2 rlen 8 { align1 1H };
send(8)         null<1>F        g11<4>UD        0x04094031
                            urb MsgDesc: 6 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g9<4>UD         0x04094039
                            urb MsgDesc: 7 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g52<1>D         g53<4>UD        0x02194033
                            urb MsgDesc: 6 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g59<1>D         g60<4>UD        0x0219403b
                            urb MsgDesc: 7 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g66<1>D         g67<4>UD        0x02194043
                            urb MsgDesc: 8 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g73<1>D         g74<4>UD        0x0219404b
                            urb MsgDesc: 9 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
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
send(8)         g2<1>UW         g7<8,8,1>UD     0x084a5001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x084a5102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x0e8c5001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c5102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
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
send(16)        g7<1>UW         g37<8,8,1>UD    0x08840003
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
send(8)         g124<1>UW       g8<8,8,1>UD     0x064a1001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g15<8,8,1>UD    0x0a8c1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g5<1>F          g114<4>F        0x06190003
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 3 Sampler = 0 mlen 3 rlen 1 { align16 1Q };
send(8)         g6<1>F          g114<4>F        0x06190104
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 4 Sampler = 1 mlen 3 rlen 1 { align16 1Q };
send(8)         g9<1>F          g114<4>F        0x06190205
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 5 Sampler = 2 mlen 3 rlen 1 { align16 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x084a6001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x084a6102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x0e8c6001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0e8c6102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a4a5001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0a4a5102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g25<1>UW        g7<8,8,1>UD     0x128c5001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 9 rlen 8 { align1 1H };
send(16)        g33<1>UW        g16<8,8,1>UD    0x128c5102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(8)         g69<1>.xUW      g66<4>UD        0x0211b500
                            data MsgDesc: ( DC untyped atomic, 0,  inc) mlen 1 rlen 1 { align16 1Q };
send(8)         g3<1>D          g114<4>F        0x04188005
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 5 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g4<1>D          g114<4>F        0x04188106
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 6 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g6<1>D          g114<4>F        0x04188207
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 7 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g9<1>D          g114<4>F        0x04188308
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 8 Sampler = 3 mlen 2 rlen 1 { align16 1Q };
send(8)         g11<1>D         g114<4>F        0x04188409
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 9 Sampler = 4 mlen 2 rlen 1 { align16 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x0c4a6001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g13<8,8,1>UD    0x0c4a6102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 1Q };
send(16)        g31<1>UW        g9<8,8,1>UD     0x168c6001
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(16)        g2<1>UW         g20<8,8,1>UD    0x168c6102
                            sampler MsgDesc: sample_l_c SIMD16 Surface = 2 Sampler = 1 mlen 11 rlen 8 { align1 1H };
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
send(8)         null<1>F        g113<4>F        0x06094008
                            urb MsgDesc: 1 write HWord per-slot interleave mlen 3 rlen 0 { align16 1Q };
(+f1.0) send(8) g2<1>UW         g6<8,8,1>UD     0x0211b501
                            data MsgDesc: ( DC untyped atomic, 1,  inc) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(16) g2<1>UW        g8<8,8,1>UD     0x0421a501
                            data MsgDesc: ( DC untyped atomic, 1,  inc) mlen 2 rlen 2 { align1 1H };
(+f1.0) send(8) null<1>UW       g2<8,8,1>UD     0x02019501
                            data MsgDesc: ( DC untyped atomic, 1,  inc) mlen 1 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g2<8,8,1>UD     0x04018501
                            data MsgDesc: ( DC untyped atomic, 1,  inc) mlen 2 rlen 0 { align1 1H };
send(8)         g2<1>UW         g10<8,8,1>UD    0x0a423001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g15<8,8,1>UD    0x0a423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g29<1>UW        g9<8,8,1>UD     0x14843001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(16)        g37<1>UW        g19<8,8,1>UD    0x14843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
(+f1.0) send(8) g4<1>UW         g12<8,8,1>UD    0x0211b502
                            data MsgDesc: ( DC untyped atomic, 2,  inc) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(16) g5<1>UW        g17<8,8,1>UD    0x0421a502
                            data MsgDesc: ( DC untyped atomic, 2,  inc) mlen 2 rlen 2 { align1 1H };
send(8)         g2<1>UW         g9<8,8,1>UD     0x024ab001
                            sampler MsgDesc: sampleinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x028cb001
                            sampler MsgDesc: sampleinfo SIMD16 Surface = 1 Sampler = 0 mlen 1 rlen 8 { align1 1H };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x04036e01
                            data MsgDesc: ( DC untyped surface write, 1, 46) mlen 2 rlen 0 { align1 1Q };
(+f1.0) send(8) null<1>UW       g3<8,8,1>UD     0x06036c01
                            data MsgDesc: ( DC untyped surface write, 1, 44) mlen 3 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x08035e01
                            data MsgDesc: ( DC untyped surface write, 1, 30) mlen 4 rlen 0 { align1 1H };
(+f1.0) send(16) null<1>UW      g4<8,8,1>UD     0x0c035c01
                            data MsgDesc: ( DC untyped surface write, 1, 28) mlen 6 rlen 0 { align1 1H };
send(1)         g2<1>UW         g2<0,1,0>UW     0x0219e000
                            data MsgDesc: ( DC mfence, 0, 32) mlen 1 rlen 1 { align1 WE_all 1N };
send(8)         g2<1>UW         g94<8,8,1>UD    0x02116e01
                            data MsgDesc: ( DC untyped surface read, 1, 46) mlen 1 rlen 1 { align1 1Q };
(+f1.0) send(8) null<1>UW       g119<8,8,1>UD   0x02019601
                            data MsgDesc: ( DC untyped atomic, 1,  dec) mlen 1 rlen 0 { align1 1Q };
(+f1.0) send(8) g48<1>UW        g119<8,8,1>UD   0x0211b601
                            data MsgDesc: ( DC untyped atomic, 1,  dec) mlen 1 rlen 1 { align1 1Q };
send(16)        g5<1>UW         g23<8,8,1>UD    0x04215e01
                            data MsgDesc: ( DC untyped surface read, 1, 30) mlen 2 rlen 2 { align1 1H };
(+f1.0) send(16) null<1>UW      g3<8,8,1>UD     0x04018601
                            data MsgDesc: ( DC untyped atomic, 1,  dec) mlen 2 rlen 0 { align1 1H };
(+f1.0) send(16) g97<1>UW       g3<8,8,1>UD     0x0421a601
                            data MsgDesc: ( DC untyped atomic, 1,  dec) mlen 2 rlen 2 { align1 1H };
send(8)         g47<1>.xUW      g44<4>UD        0x0211b600
                            data MsgDesc: ( DC untyped atomic, 0,  dec) mlen 1 rlen 1 { align16 1Q };
send(8)         g101<1>D        g99<4>UD        0x021940c3
                            urb MsgDesc: 24 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         g110<1>D        g99<4>UD        0x021940cb
                            urb MsgDesc: 25 read OWord per-slot interleave mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        g9<4>UD         0x04094041
                            urb MsgDesc: 8 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         null<1>F        g9<4>UD         0x04094049
                            urb MsgDesc: 9 write OWord per-slot interleave mlen 2 rlen 0 { align16 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x04421001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x08841001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g6<1>UW         g6<8,8,1>UD     0x06420102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        g18<8,8,1>UD    0x0c840102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x8e08c030
                            urb MsgDesc: 6 write HWord interleave complete mlen 7 rlen 0 { align16 1Q EOT };
send(8)         g6<1>UW         g11<8,8,1>UD    0x02416002
                            data MsgDesc: ( DC untyped surface read, 2, 32) mlen 1 rlen 4 { align1 1Q };
send(16)        g19<1>UW        g17<8,8,1>UD    0x04815002
                            data MsgDesc: ( DC untyped surface read, 2, 16) mlen 2 rlen 8 { align1 1H };
send(8)         g2<1>.xD        g114<4>F        0x0218b101
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 1 Sampler = 1 mlen 1 rlen 1 { align16 1Q };
send(8)         g4<1>.xD        g114<4>F        0x0218b202
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 2 Sampler = 2 mlen 1 rlen 1 { align16 1Q };
send(8)         g6<1>.xD        g114<4>F        0x0218b303
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 3 Sampler = 3 mlen 1 rlen 1 { align16 1Q };
send(8)         g8<1>.xD        g114<4>F        0x0218b404
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 4 Sampler = 4 mlen 1 rlen 1 { align16 1Q };
send(8)         g10<1>.xD       g114<4>F        0x0218b505
                            sampler MsgDesc: sampleinfo SIMD4x2 Surface = 5 Sampler = 5 mlen 1 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g8<8,8,1>UD     0x064a2001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g12<8,8,1>UD    0x0a8c2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g6<1>UW         g15<8,8,1>UD    0x08423102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g2<8,8,1>UD     0x10843102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g5<1>UW         g5<8,8,1>UD     0x04194e01
                            render MsgDesc: typed surface read MsgCtrl = 0x14 Surface = 1 mlen 2 rlen 1 { align1 1Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x060b4e02
                            render MsgDesc: typed surface write MsgCtrl = 0x14 Surface = 2 mlen 3 rlen 0 { align1 1Q };
send(8)         g7<1>UW         g10<8,8,1>UD    0x04196e01
                            render MsgDesc: typed surface read MsgCtrl = 0x46 Surface = 1 mlen 2 rlen 1 { align1 2Q };
send(8)         null<1>UW       g12<8,8,1>UD    0x060b6e02
                            render MsgDesc: typed surface write MsgCtrl = 0x46 Surface = 2 mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>F        g113<4>F        0x1a094030
                            urb MsgDesc: 6 write HWord per-slot interleave mlen 13 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x06094060
                            urb MsgDesc: 12 write HWord per-slot interleave mlen 3 rlen 0 { align16 1Q };
send(8)         null<1>F        g113<4>F        0x9a08c000
                            urb MsgDesc: 0 write HWord interleave complete mlen 13 rlen 0 { align16 1Q EOT };
send(8)         g14<1>UW        g11<8,8,1>UD    0x084b0206
                            sampler MsgDesc: gather4_c SIMD8 Surface = 6 Sampler = 2 mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x084b0004
                            sampler MsgDesc: gather4_c SIMD8 Surface = 4 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x0a4b0105
                            sampler MsgDesc: gather4_c SIMD8 Surface = 5 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g26<1>UW        g2<8,8,1>UD     0x0e8d0206
                            sampler MsgDesc: gather4_c SIMD16 Surface = 6 Sampler = 2 mlen 7 rlen 8 { align1 1H };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0e8d0004
                            sampler MsgDesc: gather4_c SIMD16 Surface = 4 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(16)        g10<1>UW        g34<8,8,1>UD    0x128d0105
                            sampler MsgDesc: gather4_c SIMD16 Surface = 5 Sampler = 1 mlen 9 rlen 8 { align1 1H };
send(16)        g52<1>UD        g6<8,8,1>UD     0x02280304
                            const MsgDesc: (4, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g30<1>UD        g11<8,8,1>UD    0x02280303
                            const MsgDesc: (3, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g32<1>UD        g14<8,8,1>UD    0x02280306
                            const MsgDesc: (6, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g34<1>UD        g16<8,8,1>UD    0x02280305
                            const MsgDesc: (5, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g7<1>UW         g24<8,8,1>UD    0x02116e02
                            data MsgDesc: ( DC untyped surface read, 2, 46) mlen 1 rlen 1 { align1 1Q };
send(8)         g8<1>UW         g24<8,8,1>UD    0x02116e04
                            data MsgDesc: ( DC untyped surface read, 4, 46) mlen 1 rlen 1 { align1 1Q };
send(8)         g5<1>UW         g21<8,8,1>UD    0x02116e03
                            data MsgDesc: ( DC untyped surface read, 3, 46) mlen 1 rlen 1 { align1 1Q };
send(16)        g12<1>UW        g40<8,8,1>UD    0x04215e02
                            data MsgDesc: ( DC untyped surface read, 2, 30) mlen 2 rlen 2 { align1 1H };
send(16)        g14<1>UW        g40<8,8,1>UD    0x04215e04
                            data MsgDesc: ( DC untyped surface read, 4, 30) mlen 2 rlen 2 { align1 1H };
send(16)        g8<1>UW         g36<8,8,1>UD    0x04215e03
                            data MsgDesc: ( DC untyped surface read, 3, 30) mlen 2 rlen 2 { align1 1H };
(+f1.0) send(8) null<1>UW       g11<8,8,1>UD    0x0a036002
                            data MsgDesc: ( DC untyped surface write, 2, 32) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g13<8,8,1>UD    0x14035002
                            data MsgDesc: ( DC untyped surface write, 2, 16) mlen 10 rlen 0 { align1 1H };
send(8)         g15<1>D         g114<4>F        0x0210a707
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 7 Sampler = 7 mlen 1 rlen 1 { align16 1Q };
send(8)         g17<1>D         g114<4>F        0x0210a808
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 8 Sampler = 8 mlen 1 rlen 1 { align16 1Q };
send(8)         g19<1>D         g114<4>F        0x0210a909
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 9 Sampler = 9 mlen 1 rlen 1 { align16 1Q };
send(8)         g21<1>D         g114<4>F        0x0210aa0a
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 10 Sampler = 10 mlen 1 rlen 1 { align16 1Q };
send(8)         g23<1>D         g114<4>F        0x0210ab0b
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 11 Sampler = 11 mlen 1 rlen 1 { align16 1Q };
send(8)         g25<1>D         g114<4>F        0x0210ac0c
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 12 Sampler = 12 mlen 1 rlen 1 { align16 1Q };
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
send(8)         null<1>F        g113<4>F        0x16094060
                            urb MsgDesc: 12 write HWord per-slot interleave mlen 11 rlen 0 { align16 1Q };
send(8)         null<1>UW       g9<8,8,1>UD     0x0e0b4002
                            render MsgDesc: typed surface write MsgCtrl = 0x0 Surface = 2 mlen 7 rlen 0 { align1 1Q };
send(8)         null<1>UW       g21<8,8,1>UD    0x0e0b6002
                            render MsgDesc: typed surface write MsgCtrl = 0x32 Surface = 2 mlen 7 rlen 0 { align1 2Q };
send(8)         g2<1>UW         g50<8,8,1>UD    0x02216c01
                            data MsgDesc: ( DC untyped surface read, 1, 44) mlen 1 rlen 2 { align1 1Q };
(+f1.0) send(8) null<1>UW       g2<8,8,1>UD     0x06036c02
                            data MsgDesc: ( DC untyped surface write, 2, 44) mlen 3 rlen 0 { align1 1Q };
send(16)        g15<1>UW        g85<8,8,1>UD    0x04415c01
                            data MsgDesc: ( DC untyped surface read, 1, 28) mlen 2 rlen 4 { align1 1H };
(+f1.0) send(16) null<1>UW      g2<8,8,1>UD     0x0c035c02
                            data MsgDesc: ( DC untyped surface write, 2, 28) mlen 6 rlen 0 { align1 1H };
send(8)         null<1>UW       g7<8,8,1>UD     0x080b4e02
                            render MsgDesc: typed surface write MsgCtrl = 0x14 Surface = 2 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x080b6e02
                            render MsgDesc: typed surface write MsgCtrl = 0x46 Surface = 2 mlen 4 rlen 0 { align1 2Q };
send(8)         g124<1>UW       g7<8,8,1>UD     0x104a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x084a0001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g8<8,8,1>UD     0x0e8c0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g4<1>UW         g16<8,8,1>UD    0x04194e02
                            render MsgDesc: typed surface read MsgCtrl = 0x14 Surface = 2 mlen 2 rlen 1 { align1 1Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619a701
                            render MsgDesc: typed atomic op MsgCtrl = 0x39 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g7<1>UW         g31<8,8,1>UD    0x04196e02
                            render MsgDesc: typed surface read MsgCtrl = 0x46 Surface = 2 mlen 2 rlen 1 { align1 2Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619b701
                            render MsgDesc: typed atomic op MsgCtrl = 0x55 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619ad01
                            render MsgDesc: typed atomic op MsgCtrl = 0x45 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619bd01
                            render MsgDesc: typed atomic op MsgCtrl = 0x61 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619ac01
                            render MsgDesc: typed atomic op MsgCtrl = 0x44 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619bc01
                            render MsgDesc: typed atomic op MsgCtrl = 0x60 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619a101
                            render MsgDesc: typed atomic op MsgCtrl = 0x33 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619b101
                            render MsgDesc: typed atomic op MsgCtrl = 0x49 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619a201
                            render MsgDesc: typed atomic op MsgCtrl = 0x34 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619b201
                            render MsgDesc: typed atomic op MsgCtrl = 0x50 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619a301
                            render MsgDesc: typed atomic op MsgCtrl = 0x35 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619b301
                            render MsgDesc: typed atomic op MsgCtrl = 0x51 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0619a401
                            render MsgDesc: typed atomic op MsgCtrl = 0x36 Surface = 1 mlen 3 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g5<8,8,1>UD     0x0619b401
                            render MsgDesc: typed atomic op MsgCtrl = 0x52 Surface = 1 mlen 3 rlen 1 { align1 2Q };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0819ae01
                            render MsgDesc: typed atomic op MsgCtrl = 0x46 Surface = 1 mlen 4 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0819be01
                            render MsgDesc: typed atomic op MsgCtrl = 0x62 Surface = 1 mlen 4 rlen 1 { align1 2Q };
send(8)         null<1>F        g113<4>F        0x8608c060
                            urb MsgDesc: 12 write HWord interleave complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         g9<1>UW         g19<8,8,1>UD    0x0843e102
                            sampler MsgDesc: ld2dms SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g7<8,8,1>UD     0x1085e102
                            sampler MsgDesc: ld2dms SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(8)         g9<1>UW         g21<8,8,1>UD    0x0443d002
                            sampler MsgDesc: ld_mcs SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g23<1>UW        g11<8,8,1>UD    0x0885d002
                            sampler MsgDesc: ld_mcs SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         null<1>UW       g2<8,8,1>UD     0x060b4e01
                            render MsgDesc: typed surface write MsgCtrl = 0x14 Surface = 1 mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x060b6e01
                            render MsgDesc: typed surface write MsgCtrl = 0x46 Surface = 1 mlen 3 rlen 0 { align1 2Q };
(+f1.0) send(8) null<1>UW       g11<8,8,1>UD    0x0a036003
                            data MsgDesc: ( DC untyped surface write, 3, 32) mlen 5 rlen 0 { align1 1Q };
(+f1.0) send(16) null<1>UW      g11<8,8,1>UD    0x14035003
                            data MsgDesc: ( DC untyped surface write, 3, 16) mlen 10 rlen 0 { align1 1H };
send(8)         g3<1>UW         g4<8,8,1>UD     0x02427002
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g4<1>UW         g12<8,8,1>UD    0x04847002
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g124<1>UW       g6<8,8,1>UD     0x0a4a4001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g7<8,8,1>UD     0x08425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g11<8,8,1>UD    0x08425102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g11<8,8,1>UD    0x10845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(16)        g10<1>UW        g19<8,8,1>UD    0x10845102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(16)        g2<1>UD         g4<8,8,1>UD     0x02280307
                            const MsgDesc: (7, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g2<1>UW         g2<8,8,1>UD     0x064a8002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0a8c8002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 5 rlen 8 { align1 1H };
(+f1.0) send(8) null<1>UW       g9<8,8,1>UD     a0<0,1,0>UD     0x00000200
                            data MsgDesc: indirect                          { align1 1Q };
send(8)         g10<1>UW        g10<8,8,1>UD    0x04420004
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g18<1>UW        g26<8,8,1>UD    0x08840004
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g22<1>UW        g22<8,8,1>UD    0x064a800d
                            sampler MsgDesc: gather4 SIMD8 Surface = 13 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(8)         g38<1>UW        g38<8,8,1>UD    0x084a810e
                            sampler MsgDesc: gather4 SIMD8 Surface = 14 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g5<1>UW         g19<8,8,1>UD    0x064a820f
                            sampler MsgDesc: gather4 SIMD8 Surface = 15 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(8)         g47<1>UW        g41<8,8,1>UD    0x064a8310
                            sampler MsgDesc: gather4 SIMD8 Surface = 16 Sampler = 3 mlen 3 rlen 4 { align1 1Q };
send(8)         g14<1>UW        g14<8,8,1>UD    0x084a8411
                            sampler MsgDesc: gather4 SIMD8 Surface = 17 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(8)         g47<1>UW        g7<8,8,1>UD     0x064a8512
                            sampler MsgDesc: gather4 SIMD8 Surface = 18 Sampler = 5 mlen 3 rlen 4 { align1 1Q };
send(8)         g25<1>UW        g25<8,8,1>UD    0x064a8613
                            sampler MsgDesc: gather4 SIMD8 Surface = 19 Sampler = 6 mlen 3 rlen 4 { align1 1Q };
send(8)         g29<1>UW        g45<8,8,1>UD    0x084a8714
                            sampler MsgDesc: gather4 SIMD8 Surface = 20 Sampler = 7 mlen 4 rlen 4 { align1 1Q };
send(8)         g11<1>UW        g8<8,8,1>UD     0x064a8815
                            sampler MsgDesc: gather4 SIMD8 Surface = 21 Sampler = 8 mlen 3 rlen 4 { align1 1Q };
send(8)         g26<1>UW        g26<8,8,1>UD    0x084b0916
                            sampler MsgDesc: gather4_c SIMD8 Surface = 22 Sampler = 9 mlen 4 rlen 4 { align1 1Q };
send(8)         g2<1>UW         g2<8,8,1>UD     0x0a4b0a17
                            sampler MsgDesc: gather4_c SIMD8 Surface = 23 Sampler = 10 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g6<8,8,1>UD     0x084b0b18
                            sampler MsgDesc: gather4_c SIMD8 Surface = 24 Sampler = 11 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         g10<8,8,1>UD    0x0a8c800d
                            sampler MsgDesc: gather4 SIMD16 Surface = 13 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(16)        g34<1>UW        g42<8,8,1>UD    0x0e8c810e
                            sampler MsgDesc: gather4 SIMD16 Surface = 14 Sampler = 1 mlen 7 rlen 8 { align1 1H };
send(16)        g34<1>UW        g89<8,8,1>UD    0x0a8c820f
                            sampler MsgDesc: gather4 SIMD16 Surface = 15 Sampler = 2 mlen 5 rlen 8 { align1 1H };
send(16)        g30<1>UW        g73<8,8,1>UD    0x0a8c8310
                            sampler MsgDesc: gather4 SIMD16 Surface = 16 Sampler = 3 mlen 5 rlen 8 { align1 1H };
send(16)        g30<1>UW        g23<8,8,1>UD    0x0e8c8411
                            sampler MsgDesc: gather4 SIMD16 Surface = 17 Sampler = 4 mlen 7 rlen 8 { align1 1H };
send(16)        g5<1>UW         g33<8,8,1>UD    0x0a8c8512
                            sampler MsgDesc: gather4 SIMD16 Surface = 18 Sampler = 5 mlen 5 rlen 8 { align1 1H };
send(16)        g33<1>UW        g56<8,8,1>UD    0x0a8c8613
                            sampler MsgDesc: gather4 SIMD16 Surface = 19 Sampler = 6 mlen 5 rlen 8 { align1 1H };
send(16)        g34<1>UW        g23<8,8,1>UD    0x0e8c8714
                            sampler MsgDesc: gather4 SIMD16 Surface = 20 Sampler = 7 mlen 7 rlen 8 { align1 1H };
send(16)        g5<1>UW         g34<8,8,1>UD    0x0a8c8815
                            sampler MsgDesc: gather4 SIMD16 Surface = 21 Sampler = 8 mlen 5 rlen 8 { align1 1H };
send(16)        g38<1>UW        g67<8,8,1>UD    0x0e8d0916
                            sampler MsgDesc: gather4_c SIMD16 Surface = 22 Sampler = 9 mlen 7 rlen 8 { align1 1H };
send(16)        g38<1>UW        g2<8,8,1>UD     0x128d0a17
                            sampler MsgDesc: gather4_c SIMD16 Surface = 23 Sampler = 10 mlen 9 rlen 8 { align1 1H };
send(16)        g18<1>UW        g33<8,8,1>UD    0x0e8d0b18
                            sampler MsgDesc: gather4_c SIMD16 Surface = 24 Sampler = 11 mlen 7 rlen 8 { align1 1H };
(+f1.0) send(8) g3<1>UW         g10<8,8,1>UD    0x0411b701
                            data MsgDesc: ( DC untyped atomic, 1,  add) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g5<1>UW         g10<8,8,1>UD    0x0411bd01
                            data MsgDesc: ( DC untyped atomic, 1,  umin) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g6<1>UW         g10<8,8,1>UD    0x0411bc01
                            data MsgDesc: ( DC untyped atomic, 1,  umax) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g7<1>UW         g10<8,8,1>UD    0x0411b101
                            data MsgDesc: ( DC untyped atomic, 1,  and) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g8<1>UW         g10<8,8,1>UD    0x0411b201
                            data MsgDesc: ( DC untyped atomic, 1,  or) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g9<1>UW         g10<8,8,1>UD    0x0411b301
                            data MsgDesc: ( DC untyped atomic, 1,  xor) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g10<1>UW        g10<8,8,1>UD    0x0411b401
                            data MsgDesc: ( DC untyped atomic, 1,  mov) mlen 2 rlen 1 { align1 1Q };
(+f1.0) send(8) g11<1>UW        g11<8,8,1>UD    0x0611be01
                            data MsgDesc: ( DC untyped atomic, 1,  cmpwr) mlen 3 rlen 1 { align1 1Q };
(+f1.0) send(16) g3<1>UW        g19<8,8,1>UD    0x0821a701
                            data MsgDesc: ( DC untyped atomic, 1,  add) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g7<1>UW        g19<8,8,1>UD    0x0821ad01
                            data MsgDesc: ( DC untyped atomic, 1,  umin) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g9<1>UW        g19<8,8,1>UD    0x0821ac01
                            data MsgDesc: ( DC untyped atomic, 1,  umax) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g11<1>UW       g19<8,8,1>UD    0x0821a101
                            data MsgDesc: ( DC untyped atomic, 1,  and) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g13<1>UW       g19<8,8,1>UD    0x0821a201
                            data MsgDesc: ( DC untyped atomic, 1,  or) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g15<1>UW       g19<8,8,1>UD    0x0821a301
                            data MsgDesc: ( DC untyped atomic, 1,  xor) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g17<1>UW       g19<8,8,1>UD    0x0821a401
                            data MsgDesc: ( DC untyped atomic, 1,  mov) mlen 4 rlen 2 { align1 1H };
(+f1.0) send(16) g19<1>UW       g21<8,8,1>UD    0x0c21ae01
                            data MsgDesc: ( DC untyped atomic, 1,  cmpwr) mlen 6 rlen 2 { align1 1H };
send(8)         g26<1>F         g114<4>F        0x0418800c
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 12 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g30<1>F         g114<4>F        0x0418810d
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 13 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g36<1>F         g114<4>F        0x0418820e
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 14 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g55<1>D         g114<4>F        0x0418830f
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 15 Sampler = 3 mlen 2 rlen 1 { align16 1Q };
send(8)         g61<1>D         g114<4>F        0x04188410
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 16 Sampler = 4 mlen 2 rlen 1 { align16 1Q };
send(8)         g67<1>D         g114<4>F        0x04188511
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 17 Sampler = 5 mlen 2 rlen 1 { align16 1Q };
send(8)         g86<1>UD        g114<4>F        0x04188612
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 18 Sampler = 6 mlen 2 rlen 1 { align16 1Q };
send(8)         g92<1>UD        g114<4>F        0x04188713
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 19 Sampler = 7 mlen 2 rlen 1 { align16 1Q };
send(8)         g98<1>UD        g114<4>F        0x04188814
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 20 Sampler = 8 mlen 2 rlen 1 { align16 1Q };
send(8)         g6<1>F          g114<4>F        0x06190915
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 21 Sampler = 9 mlen 3 rlen 1 { align16 1Q };
send(8)         g11<1>F         g114<4>F        0x06190a16
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 22 Sampler = 10 mlen 3 rlen 1 { align16 1Q };
send(8)         g16<1>F         g114<4>F        0x06190b17
                            sampler MsgDesc: gather4_c SIMD4x2 Surface = 23 Sampler = 11 mlen 3 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g3<8,8,1>UD     0x0a4b1002
                            sampler MsgDesc: gather4_po SIMD8 Surface = 2 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g120<1>UW       g3<8,8,1>UD     0x128d1002
                            sampler MsgDesc: gather4_po SIMD16 Surface = 2 Sampler = 0 mlen 9 rlen 8 { align1 1H };
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
send(8)         g24<1>UW        g2<8,8,1>UD     0x06423203
                            sampler MsgDesc: sample_c SIMD8 Surface = 3 Sampler = 2 mlen 3 rlen 4 { align1 1Q };
send(16)        g19<1>UW        g27<8,8,1>UD    0x0c843203
                            sampler MsgDesc: sample_c SIMD16 Surface = 3 Sampler = 2 mlen 6 rlen 8 { align1 1H };
send(8)         g5<1>F          g114<4>F        0x04102303
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 3 Sampler = 3 mlen 2 rlen 1 { align16 1Q };
send(8)         g124<1>UW       g6<8,8,1>UD     0x08424001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g9<1>UW         g5<8,8,1>UD     0x04420002
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g13<1>UW        g7<8,8,1>UD     0x08840002
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g124<1>UW       g2<8,8,1>UD     0x0419a501
                            render MsgDesc: typed atomic op MsgCtrl = 0x37 Surface = 1 mlen 2 rlen 1 { align1 1Q };
send(8)         g121<1>UW       g2<8,8,1>UD     0x0419b501
                            render MsgDesc: typed atomic op MsgCtrl = 0x53 Surface = 1 mlen 2 rlen 1 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06098101
                            render MsgDesc: typed atomic op MsgCtrl = 0x1 Surface = 1 mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099101
                            render MsgDesc: typed atomic op MsgCtrl = 0x17 Surface = 1 mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06098201
                            render MsgDesc: typed atomic op MsgCtrl = 0x2 Surface = 1 mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099201
                            render MsgDesc: typed atomic op MsgCtrl = 0x18 Surface = 1 mlen 3 rlen 0 { align1 2Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06098301
                            render MsgDesc: typed atomic op MsgCtrl = 0x3 Surface = 1 mlen 3 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x06099301
                            render MsgDesc: typed atomic op MsgCtrl = 0x19 Surface = 1 mlen 3 rlen 0 { align1 2Q };
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
send(8)         g8<1>F          g114<4>F        0x04102404
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 4 Sampler = 4 mlen 2 rlen 1 { align16 1Q };
send(8)         g10<1>F         g114<4>F        0x04102606
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 6 Sampler = 6 mlen 2 rlen 1 { align16 1Q };
send(8)         g11<1>F         g114<4>F        0x04102707
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 7 Sampler = 7 mlen 2 rlen 1 { align16 1Q };
send(8)         g2<1>UW         g12<8,8,1>UD    0x0a425001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(8)         g6<1>UW         g17<8,8,1>UD    0x0a425102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g27<1>UW        g7<8,8,1>UD     0x14845001
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(16)        g35<1>UW        g17<8,8,1>UD    0x14845102
                            sampler MsgDesc: sample_b_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
send(8)         null<1>F        g113<4>F        0x0e094030
                            urb MsgDesc: 6 write HWord per-slot interleave mlen 7 rlen 0 { align16 1Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098701
                            render MsgDesc: typed atomic op MsgCtrl = 0x7 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099701
                            render MsgDesc: typed atomic op MsgCtrl = 0x23 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098d01
                            render MsgDesc: typed atomic op MsgCtrl = 0x13 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099d01
                            render MsgDesc: typed atomic op MsgCtrl = 0x29 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098101
                            render MsgDesc: typed atomic op MsgCtrl = 0x1 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099101
                            render MsgDesc: typed atomic op MsgCtrl = 0x17 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098201
                            render MsgDesc: typed atomic op MsgCtrl = 0x2 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099201
                            render MsgDesc: typed atomic op MsgCtrl = 0x18 Surface = 1 mlen 4 rlen 0 { align1 2Q };
send(8)         null<1>UW       g7<8,8,1>UD     0x08098301
                            render MsgDesc: typed atomic op MsgCtrl = 0x3 Surface = 1 mlen 4 rlen 0 { align1 1Q };
send(8)         null<1>UW       g2<8,8,1>UD     0x08099301
                            render MsgDesc: typed atomic op MsgCtrl = 0x19 Surface = 1 mlen 4 rlen 0 { align1 2Q };
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
send(8)         null<1>UW       g9<8,8,1>UD     0x0e0b4003
                            render MsgDesc: typed surface write MsgCtrl = 0x0 Surface = 3 mlen 7 rlen 0 { align1 1Q };
send(8)         null<1>UW       g15<8,8,1>UD    0x0e0b6003
                            render MsgDesc: typed surface write MsgCtrl = 0x32 Surface = 3 mlen 7 rlen 0 { align1 2Q };
