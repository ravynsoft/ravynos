send(8)         null<1>F        m1<4>F          0x8608c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         null<1>F        m1<4>F          0x8a08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 5 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         m2<8,8,1>F      0x08417001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x10827001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g0<1>F          m21<4>F         0x060920ff
                            render MsgDesc: OWORD dual block write MsgCtrl = 0x0 Surface = 255 mlen 3 rlen 0 { align16 1Q };
send(8)         g41<1>F         m22<4>F         0x041840ff
                            render MsgDesc: OWORD dual block read MsgCtrl = 0x0 Surface = 255 mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x8e08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 7 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         m1<8,8,1>F      0x16494001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 11 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m1<8,8,1>F      0x0e494001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m1<8,8,1>F      0x0e496001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g7<1>UW         m1<8,8,1>F      0x0e496102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 7 rlen 4 { align1 1Q };
send(8)         g14<1>D         m2<4>F          0x04107040
                            sampler MsgDesc: ld SIMD4x2 Surface = 64 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g7<1>.xUD       m1<4>UD         0x02182001
                            urb MsgDesc: 0 ff_sync allocate mlen 1 rlen 1   { align16 1Q };
send(8)         g7<1>UD         m1<4>F          0x0a18e400
                            urb MsgDesc: 0 urb_write interleave allocate used complete mlen 5 rlen 1 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x82088400
                            urb MsgDesc: 0 urb_write interleave complete mlen 1 rlen 0 { align16 1Q EOT };
send(8)         g8<1>UD         m1<4>F          0x0618e400
                            urb MsgDesc: 0 urb_write interleave allocate used complete mlen 3 rlen 1 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x1e084400
                            urb MsgDesc: 0 urb_write interleave used mlen 15 rlen 0 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x8608c470
                            urb MsgDesc: 7 urb_write interleave used complete mlen 3 rlen 0 { align16 1Q EOT };
send(8)         g7<1>UW         m2<8,8,1>F      0x04410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g9<1>UW         m2<8,8,1>F      0x08820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g7<1>UW         m2<8,8,1>F      0x02410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g9<1>UW         m2<8,8,1>F      0x04820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g24<1>UD        m17<4>F         0x04184000
                            dp_sampler MsgDesc: (0, 0, 2, 0) mlen 2 rlen 1  { align16 1Q };
send(8)         g8<1>UW         m2<8,8,1>F      0x0241a001
                            sampler MsgDesc: resinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x06418002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g11<1>UW        m2<8,8,1>F      0x0482a001
                            sampler MsgDesc: resinfo SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0c828002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g15<1>D         m2<4>F          0x02107040
                            sampler MsgDesc: ld SIMD4x2 Surface = 64 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x10414001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 1Q };
send(8)         g2<1>D          m2<4>F          0x0210a000
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g3<1>D          m2<4>F          0x0210a101
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 1 Sampler = 1 mlen 1 rlen 1 { align16 1Q };
send(8)         g5<1>D          m2<4>F          0x0210a202
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 2 Sampler = 2 mlen 1 rlen 1 { align16 1Q };
send(8)         g7<1>D          m2<4>F          0x0210a303
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 3 Sampler = 3 mlen 1 rlen 1 { align16 1Q };
send(8)         g9<1>D          m2<4>F          0x0210a404
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 4 Sampler = 4 mlen 1 rlen 1 { align16 1Q };
send(8)         g11<1>D         m2<4>F          0x0210a505
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 5 Sampler = 5 mlen 1 rlen 1 { align16 1Q };
send(8)         g13<1>D         m2<4>F          0x0210a606
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 6 Sampler = 6 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x9208c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 9 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         m2<8,8,1>F      0x04419001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x08829001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g13<1>UW        m17<8,8,1>UD    0x02280301
                            const MsgDesc: (1, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g11<1>D         m2<4>F          0x04188001
                            sampler MsgDesc: gather4 SIMD4x2 Surface = 1 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a000
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 0 mlen 1 rlen 0 { align16 1Q };
send(8)         g63<1>UD        m2<4>UD         0x021ba000
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x0a412001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x14822001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x02419001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x04829001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x0c416001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         null<1>F        m1<4>F          0x1e084470
                            urb MsgDesc: 7 urb_write interleave used mlen 15 rlen 0 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x8e08c4e0
                            urb MsgDesc: 14 urb_write interleave used complete mlen 7 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         m2<8,8,1>F      0x0a413001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x14823001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x06410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x0c820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x04418002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x08828002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g8<1>F          m2<4>F          0x02107000
                            sampler MsgDesc: ld SIMD4x2 Surface = 0 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x9e08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 15 rlen 0 { align16 1Q EOT };
send(8)         g2<1>UW         m16<8,8,1>F     0x04497001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m16<8,8,1>F     0x068a7001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 3 rlen 8 { align1 1H };
send(8)         g2<1>UW         m1<8,8,1>F      0x08498002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x0e8a8002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g2<1>UW         m1<8,8,1>F      0x0c491001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x168a1001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         g18<1>D         m2<4>F          0x0210a040
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 64 Sampler = 0 mlen 1 rlen 1 { align16 1Q };
send(8)         g9<1>UW         m2<8,8,1>F      0x0241a102
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(8)         g9<1>UW         m2<8,8,1>F      0x0c416102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 1Q };
send(16)        g7<1>UW         m2<8,8,1>F      0x0482a102
                            sampler MsgDesc: resinfo SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 1H };
send(8)         g9<1>UD         m1<4>F          0x1618e400
                            urb MsgDesc: 0 urb_write interleave allocate used complete mlen 11 rlen 1 { align16 1Q };
send(8)         g19<1>F         m17<4>F         0x04184040
                            dp_sampler MsgDesc: (64, 0, 2, 0) mlen 2 rlen 1 { align16 1Q };
send(8)         g3<1>UW         m1<8,8,1>F      0x0c493001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g7<1>UW         m1<8,8,1>F      0x0c493102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 1Q };
send(16)        g4<1>UW         m1<8,8,1>F      0x168a3001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(16)        g12<1>UW        m1<8,8,1>F      0x168a3102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 11 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x08418002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x10828002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x0a411001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x14821001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x0c415001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g6<1>UW         m2<8,8,1>F      0x0c415102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 1Q };
send(8)         g19<1>F         m17<4>F         0x04184001
                            dp_sampler MsgDesc: (1, 0, 2, 0) mlen 2 rlen 1  { align16 1Q };
send(8)         g2<1>UW         m1<8,8,1>F      0x06498002
                            sampler MsgDesc: gather4 SIMD8 Surface = 2 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x0a8a8002
                            sampler MsgDesc: gather4 SIMD16 Surface = 2 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         null<1>F        m1<4>F          0x9608c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 11 rlen 0 { align16 1Q EOT };
send(8)         g7<1>UD         m2<4>F          0x04107000
                            sampler MsgDesc: ld SIMD4x2 Surface = 0 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g12<1>UD        m1<4>F          0x1a18e400
                            urb MsgDesc: 0 urb_write interleave allocate used complete mlen 13 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x0a417001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x14827001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410304
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 3 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820304
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 3 mlen 4 rlen 8 { align1 1H };
send(8)         g11<1>UD        m1<4>F          0x0e18e400
                            urb MsgDesc: 0 urb_write interleave allocate used complete mlen 7 rlen 1 { align16 1Q };
send(8)         g26<1>UD        m2<4>UD         0x021ba001
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 1 mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x0c414001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x06419001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x0c829001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        m2<4>UD         0x0209a001
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 1 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a002
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 2 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a003
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 3 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a004
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 4 mlen 1 rlen 0 { align16 1Q };
send(8)         g43<1>UD        m2<4>UD         0x021ba005
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 5 mlen 1 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m1<8,8,1>F      0x12494001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 9 rlen 4 { align1 1Q };
send(16)        g3<1>UW         m17<8,8,1>UD    0x02280302
                            const MsgDesc: (2, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g7<1>UW         m2<8,8,1>F      0x0a413102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g12<1>UW        m2<8,8,1>F      0x14823102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
send(8)         g2<1>UW         m1<8,8,1>F      0x06490001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x0a8a0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 5 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410203
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 2 mlen 2 rlen 4 { align1 1Q };
send(8)         g9<1>UW         m2<8,8,1>F      0x04410102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820203
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 2 mlen 4 rlen 8 { align1 1H };
send(16)        g13<1>UW        m2<8,8,1>F      0x08820102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a203
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 2 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a304
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 3 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a405
                            sampler MsgDesc: resinfo SIMD8 Surface = 5 Sampler = 4 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a506
                            sampler MsgDesc: resinfo SIMD8 Surface = 6 Sampler = 5 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a607
                            sampler MsgDesc: resinfo SIMD8 Surface = 7 Sampler = 6 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a708
                            sampler MsgDesc: resinfo SIMD8 Surface = 8 Sampler = 7 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a809
                            sampler MsgDesc: resinfo SIMD8 Surface = 9 Sampler = 8 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241a90a
                            sampler MsgDesc: resinfo SIMD8 Surface = 10 Sampler = 9 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241aa0b
                            sampler MsgDesc: resinfo SIMD8 Surface = 11 Sampler = 10 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241ab0c
                            sampler MsgDesc: resinfo SIMD8 Surface = 12 Sampler = 11 mlen 1 rlen 4 { align1 1Q };
send(8)         g3<1>UW         m2<8,8,1>F      0x0241ac0d
                            sampler MsgDesc: resinfo SIMD8 Surface = 13 Sampler = 12 mlen 1 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a203
                            sampler MsgDesc: resinfo SIMD16 Surface = 3 Sampler = 2 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a304
                            sampler MsgDesc: resinfo SIMD16 Surface = 4 Sampler = 3 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a405
                            sampler MsgDesc: resinfo SIMD16 Surface = 5 Sampler = 4 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a506
                            sampler MsgDesc: resinfo SIMD16 Surface = 6 Sampler = 5 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a607
                            sampler MsgDesc: resinfo SIMD16 Surface = 7 Sampler = 6 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a708
                            sampler MsgDesc: resinfo SIMD16 Surface = 8 Sampler = 7 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a809
                            sampler MsgDesc: resinfo SIMD16 Surface = 9 Sampler = 8 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482a90a
                            sampler MsgDesc: resinfo SIMD16 Surface = 10 Sampler = 9 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482aa0b
                            sampler MsgDesc: resinfo SIMD16 Surface = 11 Sampler = 10 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482ab0c
                            sampler MsgDesc: resinfo SIMD16 Surface = 12 Sampler = 11 mlen 2 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0482ac0d
                            sampler MsgDesc: resinfo SIMD16 Surface = 13 Sampler = 12 mlen 2 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x14414001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 10 rlen 4 { align1 1Q };
send(8)         g17<1>F         m2<4>F          0x04102000
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 0 Sampler = 0 mlen 2 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410405
                            sampler MsgDesc: sample SIMD8 Surface = 5 Sampler = 4 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410506
                            sampler MsgDesc: sample SIMD8 Surface = 6 Sampler = 5 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410607
                            sampler MsgDesc: sample SIMD8 Surface = 7 Sampler = 6 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410708
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 7 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410809
                            sampler MsgDesc: sample SIMD8 Surface = 9 Sampler = 8 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x0441090a
                            sampler MsgDesc: sample SIMD8 Surface = 10 Sampler = 9 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410a0b
                            sampler MsgDesc: sample SIMD8 Surface = 11 Sampler = 10 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410b0c
                            sampler MsgDesc: sample SIMD8 Surface = 12 Sampler = 11 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410c0d
                            sampler MsgDesc: sample SIMD8 Surface = 13 Sampler = 12 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410d0e
                            sampler MsgDesc: sample SIMD8 Surface = 14 Sampler = 13 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410e0f
                            sampler MsgDesc: sample SIMD8 Surface = 15 Sampler = 14 mlen 2 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410f10
                            sampler MsgDesc: sample SIMD8 Surface = 16 Sampler = 15 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820405
                            sampler MsgDesc: sample SIMD16 Surface = 5 Sampler = 4 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820506
                            sampler MsgDesc: sample SIMD16 Surface = 6 Sampler = 5 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820607
                            sampler MsgDesc: sample SIMD16 Surface = 7 Sampler = 6 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820708
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 7 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820809
                            sampler MsgDesc: sample SIMD16 Surface = 9 Sampler = 8 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x0882090a
                            sampler MsgDesc: sample SIMD16 Surface = 10 Sampler = 9 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820a0b
                            sampler MsgDesc: sample SIMD16 Surface = 11 Sampler = 10 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820b0c
                            sampler MsgDesc: sample SIMD16 Surface = 12 Sampler = 11 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820c0d
                            sampler MsgDesc: sample SIMD16 Surface = 13 Sampler = 12 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820d0e
                            sampler MsgDesc: sample SIMD16 Surface = 14 Sampler = 13 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820e0f
                            sampler MsgDesc: sample SIMD16 Surface = 15 Sampler = 14 mlen 4 rlen 8 { align1 1H };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820f10
                            sampler MsgDesc: sample SIMD16 Surface = 16 Sampler = 15 mlen 4 rlen 8 { align1 1H };
send(8)         g6<1>UW         m2<8,8,1>F      0x02410102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 1Q };
send(16)        g10<1>UW        m2<8,8,1>F      0x04820102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 1H };
send(8)         g2<1>UW         m1<8,8,1>F      0x0c492001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x168a2001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 11 rlen 8 { align1 1H };
send(8)         g3<1>UW         m1<8,8,1>F      0x0e495001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 7 rlen 4 { align1 1Q };
send(8)         g7<1>UW         m1<8,8,1>F      0x0e495102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 7 rlen 4 { align1 1Q };
send(8)         g2<1>UW         m1<8,8,1>F      0x04490001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x068a0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 3 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410003
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x08820003
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x08417008
                            sampler MsgDesc: ld SIMD8 Surface = 8 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(8)         g6<1>UW         m2<8,8,1>F      0x08417109
                            sampler MsgDesc: ld SIMD8 Surface = 9 Sampler = 1 mlen 4 rlen 4 { align1 1Q };
send(8)         g7<1>UW         m2<8,8,1>F      0x0841720a
                            sampler MsgDesc: ld SIMD8 Surface = 10 Sampler = 2 mlen 4 rlen 4 { align1 1Q };
send(8)         g8<1>UW         m2<8,8,1>F      0x0841730b
                            sampler MsgDesc: ld SIMD8 Surface = 11 Sampler = 3 mlen 4 rlen 4 { align1 1Q };
send(8)         g9<1>UW         m2<8,8,1>F      0x0841740c
                            sampler MsgDesc: ld SIMD8 Surface = 12 Sampler = 4 mlen 4 rlen 4 { align1 1Q };
send(8)         g10<1>UW        m2<8,8,1>F      0x0841750d
                            sampler MsgDesc: ld SIMD8 Surface = 13 Sampler = 5 mlen 4 rlen 4 { align1 1Q };
send(8)         g11<1>UW        m2<8,8,1>F      0x0841760e
                            sampler MsgDesc: ld SIMD8 Surface = 14 Sampler = 6 mlen 4 rlen 4 { align1 1Q };
send(8)         g12<1>UW        m2<8,8,1>F      0x0841770f
                            sampler MsgDesc: ld SIMD8 Surface = 15 Sampler = 7 mlen 4 rlen 4 { align1 1Q };
send(16)        g4<1>UW         m2<8,8,1>F      0x10827008
                            sampler MsgDesc: ld SIMD16 Surface = 8 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(16)        g12<1>UW        m2<8,8,1>F      0x10827109
                            sampler MsgDesc: ld SIMD16 Surface = 9 Sampler = 1 mlen 8 rlen 8 { align1 1H };
send(16)        g12<1>UW        m2<8,8,1>F      0x1082720a
                            sampler MsgDesc: ld SIMD16 Surface = 10 Sampler = 2 mlen 8 rlen 8 { align1 1H };
send(16)        g13<1>UW        m2<8,8,1>F      0x1082730b
                            sampler MsgDesc: ld SIMD16 Surface = 11 Sampler = 3 mlen 8 rlen 8 { align1 1H };
send(16)        g14<1>UW        m2<8,8,1>F      0x1082740c
                            sampler MsgDesc: ld SIMD16 Surface = 12 Sampler = 4 mlen 8 rlen 8 { align1 1H };
send(16)        g15<1>UW        m2<8,8,1>F      0x1082750d
                            sampler MsgDesc: ld SIMD16 Surface = 13 Sampler = 5 mlen 8 rlen 8 { align1 1H };
send(16)        g16<1>UW        m2<8,8,1>F      0x1082760e
                            sampler MsgDesc: ld SIMD16 Surface = 14 Sampler = 6 mlen 8 rlen 8 { align1 1H };
send(16)        g17<1>UW        m2<8,8,1>F      0x1082770f
                            sampler MsgDesc: ld SIMD16 Surface = 15 Sampler = 7 mlen 8 rlen 8 { align1 1H };
send(8)         g30<1>UD        m2<4>UD         0x021ba002
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 2 mlen 1 rlen 1 { align16 1Q };
send(8)         g5<1>F          m2<4>F          0x04102505
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 5 Sampler = 5 mlen 2 rlen 1 { align16 1Q };
send(8)         g11<1>UW        m16<8,8,1>F     0x04497002
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(16)        g19<1>UW        m16<8,8,1>F     0x068a7002
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 0 mlen 3 rlen 8 { align1 1H };
send(8)         g6<1>UW         m2<8,8,1>F      0x06410102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 1Q };
send(16)        g10<1>UW        m2<8,8,1>F      0x0c820102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 1H };
send(8)         null<1>F        m1<4>F          0x8a08c470
                            urb MsgDesc: 7 urb_write interleave used complete mlen 5 rlen 0 { align16 1Q EOT };
send(8)         g6<1>UD         m1<4>F          0x1218e400
                            urb MsgDesc: 0 urb_write interleave allocate used complete mlen 9 rlen 1 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a005
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 5 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a006
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 6 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a007
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 7 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a008
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 8 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a009
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 9 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a00a
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 10 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a00b
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 11 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a00c
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 12 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a00d
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 13 mlen 1 rlen 0 { align16 1Q };
send(8)         null<1>F        m2<4>UD         0x0209a00e
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 14 mlen 1 rlen 0 { align16 1Q };
send(8)         g18<1>UD        m2<4>UD         0x021ba00f
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 15 mlen 1 rlen 1 { align16 1Q };
send(8)         g9<1>UD         m1<4>F          0x1a18e470
                            urb MsgDesc: 7 urb_write interleave allocate used complete mlen 13 rlen 1 { align16 1Q };
send(8)         null<1>F        m1<4>F          0x9a08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 13 rlen 0 { align16 1Q EOT };
send(16)        g2<1>UW         m17<8,8,1>UD    0x02280304
                            const MsgDesc: (4, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g2<1>UW         m17<8,8,1>UD    0x02280303
                            const MsgDesc: (3, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g2<1>UW         m17<8,8,1>UD    0x02280306
                            const MsgDesc: (6, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(16)        g2<1>UW         m17<8,8,1>UD    0x02280305
                            const MsgDesc: (5, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g34<1>UD        m2<4>UD         0x021ba003
                            render MsgDesc: streamed VB write MsgCtrl = 0x0 Surface = 3 mlen 1 rlen 1 { align16 1Q };
send(8)         g15<1>D         m2<4>F          0x0210a707
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 7 Sampler = 7 mlen 1 rlen 1 { align16 1Q };
send(8)         g17<1>D         m2<4>F          0x0210a808
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 8 Sampler = 8 mlen 1 rlen 1 { align16 1Q };
send(8)         g19<1>D         m2<4>F          0x0210a909
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 9 Sampler = 9 mlen 1 rlen 1 { align16 1Q };
send(8)         g21<1>D         m2<4>F          0x0210aa0a
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 10 Sampler = 10 mlen 1 rlen 1 { align16 1Q };
send(8)         g23<1>D         m2<4>F          0x0210ab0b
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 11 Sampler = 11 mlen 1 rlen 1 { align16 1Q };
send(8)         g25<1>D         m2<4>F          0x0210ac0c
                            sampler MsgDesc: resinfo SIMD4x2 Surface = 12 Sampler = 12 mlen 1 rlen 1 { align16 1Q };
send(8)         null<1>UW       m22<8,8,1>UD    0x040902ff
                            render MsgDesc: OWORD block write MsgCtrl = 0x2 Surface = 255 mlen 2 rlen 0 { align1 1Q };
send(8)         g69<1>UW        m22<8,8,1>UD    0x021802ff
                            render MsgDesc: OWORD block read MsgCtrl = 0x2 Surface = 255 mlen 1 rlen 1 { align1 WE_all 1Q };
send(8)         g9<1>UD         m1<4>F          0x0e18e4e0
                            urb MsgDesc: 14 urb_write interleave allocate used complete mlen 7 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m1<8,8,1>F      0x08490001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m1<8,8,1>F      0x0e8a0001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 7 rlen 8 { align1 1H };
send(8)         g2<1>UW         m2<8,8,1>F      0x08410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 1Q };
send(16)        g2<1>UW         m2<8,8,1>F      0x10820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 1H };
send(8)         null<1>F        m1<4>F          0x9a08c470
                            urb MsgDesc: 7 urb_write interleave used complete mlen 13 rlen 0 { align16 1Q EOT };
send(8)         g15<1>F         m17<4>F         0x04184043
                            dp_sampler MsgDesc: (67, 0, 2, 0) mlen 2 rlen 1 { align16 1Q };
send(8)         g21<1>F         m17<4>F         0x04184042
                            dp_sampler MsgDesc: (66, 0, 2, 0) mlen 2 rlen 1 { align16 1Q };
send(8)         g23<1>F         m17<4>F         0x04184041
                            dp_sampler MsgDesc: (65, 0, 2, 0) mlen 2 rlen 1 { align16 1Q };
send(8)         g4<1>F          m17<4>F         0x04184003
                            dp_sampler MsgDesc: (3, 0, 2, 0) mlen 2 rlen 1  { align16 1Q };
send(8)         g13<1>F         m17<4>F         0x04184002
                            dp_sampler MsgDesc: (2, 0, 2, 0) mlen 2 rlen 1  { align16 1Q };
send(8)         g14<1>UW        m2<8,8,1>F      0x0a417102
                            sampler MsgDesc: ld SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 1Q };
send(16)        g24<1>UW        m2<8,8,1>F      0x14827102
                            sampler MsgDesc: ld SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 1H };
send(16)        g2<1>UW         m17<8,8,1>UD    0x02280307
                            const MsgDesc: (7, 3, 0, 0) mlen 1 rlen 2       { align1 WE_all 1H };
send(8)         g6<1>UW         m2<8,8,1>F      0x0a413203
                            sampler MsgDesc: sample_c SIMD8 Surface = 3 Sampler = 2 mlen 5 rlen 4 { align1 1Q };
send(16)        g13<1>UW        m2<8,8,1>F      0x14823203
                            sampler MsgDesc: sample_c SIMD16 Surface = 3 Sampler = 2 mlen 10 rlen 8 { align1 1H };
send(8)         g5<1>F          m2<4>F          0x04102303
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 3 Sampler = 3 mlen 2 rlen 1 { align16 1Q };
send(8)         g2<1>UW         m2<8,8,1>F      0x04410002
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g20<1>UW        m2<8,8,1>F      0x04410008
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 0 mlen 2 rlen 4 { align1 1Q };
send(8)         g24<1>UW        m2<8,8,1>F      0x04410109
                            sampler MsgDesc: sample SIMD8 Surface = 9 Sampler = 1 mlen 2 rlen 4 { align1 1Q };
send(8)         g28<1>UW        m2<8,8,1>F      0x0441020a
                            sampler MsgDesc: sample SIMD8 Surface = 10 Sampler = 2 mlen 2 rlen 4 { align1 1Q };
send(8)         g32<1>UW        m2<8,8,1>F      0x0441030b
                            sampler MsgDesc: sample SIMD8 Surface = 11 Sampler = 3 mlen 2 rlen 4 { align1 1Q };
send(8)         g36<1>UW        m2<8,8,1>F      0x0441040c
                            sampler MsgDesc: sample SIMD8 Surface = 12 Sampler = 4 mlen 2 rlen 4 { align1 1Q };
send(8)         g40<1>UW        m2<8,8,1>F      0x0441050d
                            sampler MsgDesc: sample SIMD8 Surface = 13 Sampler = 5 mlen 2 rlen 4 { align1 1Q };
send(8)         g44<1>UW        m2<8,8,1>F      0x0441060e
                            sampler MsgDesc: sample SIMD8 Surface = 14 Sampler = 6 mlen 2 rlen 4 { align1 1Q };
send(8)         g48<1>UW        m2<8,8,1>F      0x0441070f
                            sampler MsgDesc: sample SIMD8 Surface = 15 Sampler = 7 mlen 2 rlen 4 { align1 1Q };
send(16)        g22<1>UW        m2<8,8,1>F      0x08820008
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 0 mlen 4 rlen 8 { align1 1H };
send(16)        g30<1>UW        m2<8,8,1>F      0x08820109
                            sampler MsgDesc: sample SIMD16 Surface = 9 Sampler = 1 mlen 4 rlen 8 { align1 1H };
send(16)        g22<1>UW        m2<8,8,1>F      0x0882020a
                            sampler MsgDesc: sample SIMD16 Surface = 10 Sampler = 2 mlen 4 rlen 8 { align1 1H };
send(16)        g38<1>UW        m2<8,8,1>F      0x0882030b
                            sampler MsgDesc: sample SIMD16 Surface = 11 Sampler = 3 mlen 4 rlen 8 { align1 1H };
send(16)        g30<1>UW        m2<8,8,1>F      0x0882040c
                            sampler MsgDesc: sample SIMD16 Surface = 12 Sampler = 4 mlen 4 rlen 8 { align1 1H };
send(16)        g46<1>UW        m2<8,8,1>F      0x0882050d
                            sampler MsgDesc: sample SIMD16 Surface = 13 Sampler = 5 mlen 4 rlen 8 { align1 1H };
send(16)        g22<1>UW        m2<8,8,1>F      0x0882060e
                            sampler MsgDesc: sample SIMD16 Surface = 14 Sampler = 6 mlen 4 rlen 8 { align1 1H };
send(16)        g54<1>UW        m2<8,8,1>F      0x0882070f
                            sampler MsgDesc: sample SIMD16 Surface = 15 Sampler = 7 mlen 4 rlen 8 { align1 1H };
send(8)         g5<1>F          m2<4>F          0x04102101
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 1 Sampler = 1 mlen 2 rlen 1 { align16 1Q };
send(8)         g6<1>F          m2<4>F          0x04102202
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 2 Sampler = 2 mlen 2 rlen 1 { align16 1Q };
send(8)         g8<1>F          m2<4>F          0x04102404
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 4 Sampler = 4 mlen 2 rlen 1 { align16 1Q };
send(8)         g10<1>F         m2<4>F          0x04102606
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 6 Sampler = 6 mlen 2 rlen 1 { align16 1Q };
send(8)         g11<1>F         m2<4>F          0x04102707
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 7 Sampler = 7 mlen 2 rlen 1 { align16 1Q };
send(8)         g9<1>UD         m1<4>F          0x0a18e470
                            urb MsgDesc: 7 urb_write interleave allocate used complete mlen 5 rlen 1 { align16 1Q };
