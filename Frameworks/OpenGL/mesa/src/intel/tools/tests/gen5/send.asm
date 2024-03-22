send(8) 2       g2<1>F          g2<8,8,1>F      0x02100001
                            math MsgDesc: inv mlen 1 rlen 1                 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c00
                            write MsgDesc: RT write SIMD8 LastRT Surface = 0 mlen 6 rlen 0 { align1 EOT };
send(16) 2      g12<1>F         g10<8,8,1>F     0x02100001
                            math MsgDesc: inv mlen 1 rlen 1                 { align1 compr };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 10 rlen 0 { align1 EOT };
send(8) 1       null<1>F        g0<4>F          0x8a08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 5 rlen 0 { align16 EOT };
send(8) 2       g2<1>UW         null<8,8,1>F    0x06410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 3 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x0c820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 6 rlen 8 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x04410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x08820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 };
send(8) 14      g3<1>UD         g0<4>F          0x04181000
                            read MsgDesc: OWord Dual Block Read MsgCtrl = 0x0 Surface = 0 mlen 2 rlen 1 { align16 };
send(8) 1       null<1>F        g0<4>F          0x8808c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 4 rlen 0 { align16 EOT };
send(8) 2       g13<1>UW        null<8,8,1>F    0x0241a001
                            sampler MsgDesc: resinfo SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 };
send(16) 2      g26<1>UW        null<8,8,1>F    0x0482a001
                            sampler MsgDesc: resinfo SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x08417001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 4 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x10827001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 8 rlen 8 { align1 };
send(8) 1       null<1>F        g0<4>F          0x8c08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 6 rlen 0 { align16 EOT };
send(8) 2       g2<1>F          g2<8,8,1>F      0x0410000a
                            math MsgDesc: pow mlen 2 rlen 1                 { align1 };
send(8) 2       g12<1>UW        null<8,8,1>F    0x02410001
                            sampler MsgDesc: sample SIMD8 Surface = 1 Sampler = 0 mlen 1 rlen 4 { align1 };
send(16) 2      g16<1>UW        null<8,8,1>F    0x04820001
                            sampler MsgDesc: sample SIMD16 Surface = 1 Sampler = 0 mlen 2 rlen 8 { align1 };
send(8) 2       g2<1>F          g2<8,8,1>F      0x02100007
                            math MsgDesc: cos mlen 1 rlen 1                 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x0a411001
                            sampler MsgDesc: sample_b SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x14821001
                            sampler MsgDesc: sample_b SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x90084c00
                            write MsgDesc: RT write SIMD8 LastRT Surface = 0 mlen 8 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x9c084800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 14 rlen 0 { align1 EOT };
send(8) 1       null<1>F        g0<4>F          0x1a084400
                            urb MsgDesc: 0 urb_write interleave used mlen 13 rlen 0 { align16 };
send(8) 1       null<1>F        g0<4>F          0x9008c460
                            urb MsgDesc: 6 urb_write interleave used complete mlen 8 rlen 0 { align16 EOT };
send(8) 1       g5<1>.yF        g6<4>.xF        0x02100006
                            math MsgDesc: sin mlen 1 rlen 1                 { align16 };
send(8) 1       g7<1>.xD        g1<0>.zD        0x0410001c
                            math MsgDesc: intdiv signed mlen 2 rlen 1       { align16 };
send(8) 2       g3<1>F          g2.3<0,1,0>F    0x02100081
                            math MsgDesc: inv scalar mlen 1 rlen 1          { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8e084c00
                            write MsgDesc: RT write SIMD8 LastRT Surface = 0 mlen 7 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x98084800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 12 rlen 0 { align1 EOT };
send(8) 1       g30<1>.xF       (abs)g30<4>.xF  0x02100005
                            math MsgDesc: rsq mlen 1 rlen 1                 { align16 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x0a412001
                            sampler MsgDesc: sample_l SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x14822001
                            sampler MsgDesc: sample_l SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 };
send(8) 2       g2<1>F          g2<8,8,1>F      0x02100004
                            math MsgDesc: sqrt mlen 1 rlen 1                { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x92084c00
                            write MsgDesc: RT write SIMD8 LastRT Surface = 0 mlen 9 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x9e084800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 15 rlen 0 { align1 EOT };
send(8) 2       g2<1>UW         null<8,8,1>F    0x04410304
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 3 mlen 2 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x08820304
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 3 mlen 4 rlen 8 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x0a413001
                            sampler MsgDesc: sample_c SIMD8 Surface = 1 Sampler = 0 mlen 5 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x14823001
                            sampler MsgDesc: sample_c SIMD16 Surface = 1 Sampler = 0 mlen 10 rlen 8 { align1 };
send(8) 1       null<1>F        g0<4>F          0x9008c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 8 rlen 0 { align16 EOT };
send(8) 1       null<1>F        g0<4>F          0x9608c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 11 rlen 0 { align16 EOT };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084400
                            write MsgDesc: RT write SIMD8 Surface = 0 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084401
                            write MsgDesc: RT write SIMD8 Surface = 1 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084402
                            write MsgDesc: RT write SIMD8 Surface = 2 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c03
                            write MsgDesc: RT write SIMD8 LastRT Surface = 3 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084000
                            write MsgDesc: RT write SIMD16 Surface = 0 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084001
                            write MsgDesc: RT write SIMD16 Surface = 1 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084002
                            write MsgDesc: RT write SIMD16 Surface = 2 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084803
                            write MsgDesc: RT write SIMD16 LastRT Surface = 3 mlen 10 rlen 0 { align1 EOT };
send(8) 2       g2<1>F          g2<8,8,1>F      0x02100002
                            math MsgDesc: log mlen 1 rlen 1                 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x0c416001
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 };
send(8) 2       g3<1>F          g2<0,1,0>F      0x041000ca
                            math MsgDesc: pow sat scalar mlen 2 rlen 1      { align1 };
send(8) 2       g7<1>UW         null<8,8,1>F    0x0a413102
                            sampler MsgDesc: sample_c SIMD8 Surface = 2 Sampler = 1 mlen 5 rlen 4 { align1 };
send(16) 2      g14<1>UW        null<8,8,1>F    0x14823102
                            sampler MsgDesc: sample_c SIMD16 Surface = 2 Sampler = 1 mlen 10 rlen 8 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c01
                            write MsgDesc: RT write SIMD8 LastRT Surface = 1 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084801
                            write MsgDesc: RT write SIMD16 LastRT Surface = 1 mlen 10 rlen 0 { align1 EOT };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c02
                            write MsgDesc: RT write SIMD8 LastRT Surface = 2 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084802
                            write MsgDesc: RT write SIMD16 LastRT Surface = 2 mlen 10 rlen 0 { align1 EOT };
send(8) 2       g9<1>UW         null<8,8,1>F    0x04410102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 2 rlen 4 { align1 };
send(16) 2      g20<1>UW        null<8,8,1>F    0x08820102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 4 rlen 8 { align1 };
send(8) 1       null<1>F        g0<4>F          0x8e08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 7 rlen 0 { align16 EOT };
send(8) 13      g0<1>F          g0<4>F          0x061890ff
                            write MsgDesc: OWord dual block write MsgCtrl = 0x0 Surface = 255 mlen 3 rlen 1 { align16 };
send(8) 14      g6<1>F          g0<4>F          0x041850ff
                            read MsgDesc: OWord Dual Block Read MsgCtrl = 0x0 Surface = 255 mlen 2 rlen 1 { align16 };
send(8) 2       g17<1>UW        null<8,8,1>F    0x0241a102
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 1 mlen 1 rlen 4 { align1 };
send(8) 2       g30<1>UW        null<8,8,1>F    0x0241a203
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 2 mlen 1 rlen 4 { align1 };
send(8) 2       g30<1>UW        null<8,8,1>F    0x04410203
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 2 mlen 2 rlen 4 { align1 };
send(8) 2       g13<1>UW        null<8,8,1>F    0x0241a304
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 3 mlen 1 rlen 4 { align1 };
send(8) 2       g34<1>UW        null<8,8,1>F    0x0241a405
                            sampler MsgDesc: resinfo SIMD8 Surface = 5 Sampler = 4 mlen 1 rlen 4 { align1 };
send(8) 2       g34<1>UW        null<8,8,1>F    0x04410405
                            sampler MsgDesc: sample SIMD8 Surface = 5 Sampler = 4 mlen 2 rlen 4 { align1 };
send(8) 2       g38<1>UW        null<8,8,1>F    0x0241a506
                            sampler MsgDesc: resinfo SIMD8 Surface = 6 Sampler = 5 mlen 1 rlen 4 { align1 };
send(8) 2       g9<1>UW         null<8,8,1>F    0x04410506
                            sampler MsgDesc: sample SIMD8 Surface = 6 Sampler = 5 mlen 2 rlen 4 { align1 };
send(8) 2       g38<1>UW        null<8,8,1>F    0x0241a607
                            sampler MsgDesc: resinfo SIMD8 Surface = 7 Sampler = 6 mlen 1 rlen 4 { align1 };
send(8) 2       g38<1>UW        null<8,8,1>F    0x04410607
                            sampler MsgDesc: sample SIMD8 Surface = 7 Sampler = 6 mlen 2 rlen 4 { align1 };
send(8) 2       g42<1>UW        null<8,8,1>F    0x0241a708
                            sampler MsgDesc: resinfo SIMD8 Surface = 8 Sampler = 7 mlen 1 rlen 4 { align1 };
send(8) 2       g42<1>UW        null<8,8,1>F    0x04410708
                            sampler MsgDesc: sample SIMD8 Surface = 8 Sampler = 7 mlen 2 rlen 4 { align1 };
send(16) 2      g14<1>UW        null<8,8,1>F    0x0482a102
                            sampler MsgDesc: resinfo SIMD16 Surface = 2 Sampler = 1 mlen 2 rlen 8 { align1 };
send(16) 2      g26<1>UW        null<8,8,1>F    0x0482a203
                            sampler MsgDesc: resinfo SIMD16 Surface = 3 Sampler = 2 mlen 2 rlen 8 { align1 };
send(16) 2      g26<1>UW        null<8,8,1>F    0x08820203
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 2 mlen 4 rlen 8 { align1 };
send(16) 2      g34<1>UW        null<8,8,1>F    0x0482a304
                            sampler MsgDesc: resinfo SIMD16 Surface = 4 Sampler = 3 mlen 2 rlen 8 { align1 };
send(16) 2      g42<1>UW        null<8,8,1>F    0x0482a405
                            sampler MsgDesc: resinfo SIMD16 Surface = 5 Sampler = 4 mlen 2 rlen 8 { align1 };
send(16) 2      g42<1>UW        null<8,8,1>F    0x08820405
                            sampler MsgDesc: sample SIMD16 Surface = 5 Sampler = 4 mlen 4 rlen 8 { align1 };
send(16) 2      g50<1>UW        null<8,8,1>F    0x0482a506
                            sampler MsgDesc: resinfo SIMD16 Surface = 6 Sampler = 5 mlen 2 rlen 8 { align1 };
send(16) 2      g50<1>UW        null<8,8,1>F    0x08820506
                            sampler MsgDesc: sample SIMD16 Surface = 6 Sampler = 5 mlen 4 rlen 8 { align1 };
send(16) 2      g58<1>UW        null<8,8,1>F    0x0482a607
                            sampler MsgDesc: resinfo SIMD16 Surface = 7 Sampler = 6 mlen 2 rlen 8 { align1 };
send(16) 2      g58<1>UW        null<8,8,1>F    0x08820607
                            sampler MsgDesc: sample SIMD16 Surface = 7 Sampler = 6 mlen 4 rlen 8 { align1 };
send(16) 2      g66<1>UW        null<8,8,1>F    0x0482a708
                            sampler MsgDesc: resinfo SIMD16 Surface = 8 Sampler = 7 mlen 2 rlen 8 { align1 };
send(16) 2      g66<1>UW        null<8,8,1>F    0x08820708
                            sampler MsgDesc: sample SIMD16 Surface = 8 Sampler = 7 mlen 4 rlen 8 { align1 };
send(8) 1       null<1>F        g0<4>F          0x9a08c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 13 rlen 0 { align16 EOT };
send(8) 1       null<1>F        g0<4>F          0x9808c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 12 rlen 0 { align16 EOT };
send(8) 2       g8<1>UW         null<8,8,1>F    0x0c416102
                            sampler MsgDesc: sample_l_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x96084800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 11 rlen 0 { align1 EOT };
send(8) 2       g3<1>F          null<4>UD       0x04102505
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 5 Sampler = 5 mlen 2 rlen 1 { align16 };
send(8) 2       g3<1>F          g2<0,1,0>F      0x021000c4
                            math MsgDesc: sqrt sat scalar mlen 1 rlen 1     { align1 };
send(8) 2       g3<1>F          g2<0,1,0>F      0x021000c3
                            math MsgDesc: exp sat scalar mlen 1 rlen 1      { align1 };
send(8) 2       g3<1>F          null<4>UD       0x04102000
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 0 Sampler = 0 mlen 2 rlen 1 { align16 };
send(8) 1       g3<1>F          g1<0>F          0x02100044
                            math MsgDesc: sqrt sat mlen 1 rlen 1            { align16 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084403
                            write MsgDesc: RT write SIMD8 Surface = 3 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c04
                            write MsgDesc: RT write SIMD8 LastRT Surface = 4 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084003
                            write MsgDesc: RT write SIMD16 Surface = 3 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084804
                            write MsgDesc: RT write SIMD16 LastRT Surface = 4 mlen 10 rlen 0 { align1 EOT };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084404
                            write MsgDesc: RT write SIMD8 Surface = 4 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c05
                            write MsgDesc: RT write SIMD8 LastRT Surface = 5 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084004
                            write MsgDesc: RT write SIMD16 Surface = 4 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084805
                            write MsgDesc: RT write SIMD16 LastRT Surface = 5 mlen 10 rlen 0 { align1 EOT };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084405
                            write MsgDesc: RT write SIMD8 Surface = 5 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c06
                            write MsgDesc: RT write SIMD8 LastRT Surface = 6 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084005
                            write MsgDesc: RT write SIMD16 Surface = 5 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084806
                            write MsgDesc: RT write SIMD16 LastRT Surface = 6 mlen 10 rlen 0 { align1 EOT };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x0c084406
                            write MsgDesc: RT write SIMD8 Surface = 6 mlen 6 rlen 0 { align1 };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x8c084c07
                            write MsgDesc: RT write SIMD8 LastRT Surface = 7 mlen 6 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x14084006
                            write MsgDesc: RT write SIMD16 Surface = 6 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x94084807
                            write MsgDesc: RT write SIMD16 LastRT Surface = 7 mlen 10 rlen 0 { align1 EOT };
send(8) 2       g2<1>UW         null<8,8,1>F    0x10414001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 8 rlen 4 { align1 };
send(8) 2       g6<1>UW         null<8,8,1>F    0x06410102
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 1 mlen 3 rlen 4 { align1 };
send(16) 2      g12<1>UW        null<8,8,1>F    0x0c820102
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 1 mlen 6 rlen 8 { align1 };
send(8) 13      g11<1>UW        g0<8,8,1>F      0x04497001
                            sampler MsgDesc: ld SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 };
send(16) 13     g4<1>UW         g0<8,8,1>F      0x068a7001
                            sampler MsgDesc: ld SIMD16 Surface = 1 Sampler = 0 mlen 3 rlen 8 { align1 };
send(8) 1       null<1>F        g0<4>F          0x9408c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 10 rlen 0 { align16 EOT };
send(8) 1       null<1>F        g0<4>F          0x8408c460
                            urb MsgDesc: 6 urb_write interleave used complete mlen 2 rlen 0 { align16 EOT };
send(8) 2       g4<1>F          g2<0,1,0>F      0x02100084
                            math MsgDesc: sqrt scalar mlen 1 rlen 1         { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x0c414001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 };
send(8) 1       g3<1>F          g1<0>F          0x02100043
                            math MsgDesc: exp sat mlen 1 rlen 1             { align16 };
send(8) 1       null<1>F        g0<4>F          0x9208c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 9 rlen 0 { align16 EOT };
send(8) 2       g2<1>UW         null<8,8,1>F    0x0c415001
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 1 Sampler = 0 mlen 6 rlen 4 { align1 };
send(8) 2       g5<1>F          g2<0,1,0>F      0x02100087
                            math MsgDesc: cos scalar mlen 1 rlen 1          { align1 };
send(8) 1       g4<1>.xF        g3<4>.xF        0x02100003
                            math MsgDesc: exp mlen 1 rlen 1                 { align16 };
send(8) 2       g7<1>UW         null<8,8,1>F    0x0c415102
                            sampler MsgDesc: sample_b_c SIMD8 Surface = 2 Sampler = 1 mlen 6 rlen 4 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x14414001
                            sampler MsgDesc: sample_d SIMD8 Surface = 1 Sampler = 0 mlen 10 rlen 4 { align1 };
send(8) 2       g3<1>F          g2<0,1,0>F      0x02100083
                            math MsgDesc: exp scalar mlen 1 rlen 1          { align1 };
send(8) 2       g6<1>UW         null<8,8,1>F    0x04410003
                            sampler MsgDesc: sample SIMD8 Surface = 3 Sampler = 0 mlen 2 rlen 4 { align1 };
send(8) 2       g10<1>UW        null<8,8,1>F    0x04410004
                            sampler MsgDesc: sample SIMD8 Surface = 4 Sampler = 0 mlen 2 rlen 4 { align1 };
send(16) 2      g14<1>UW        null<8,8,1>F    0x08820003
                            sampler MsgDesc: sample SIMD16 Surface = 3 Sampler = 0 mlen 4 rlen 8 { align1 };
send(16) 2      g22<1>UW        null<8,8,1>F    0x08820004
                            sampler MsgDesc: sample SIMD16 Surface = 4 Sampler = 0 mlen 4 rlen 8 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x04419001
                            sampler MsgDesc: lod SIMD8 Surface = 1 Sampler = 0 mlen 2 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x08829001
                            sampler MsgDesc: lod SIMD16 Surface = 1 Sampler = 0 mlen 4 rlen 8 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x04410f10
                            sampler MsgDesc: sample SIMD8 Surface = 16 Sampler = 15 mlen 2 rlen 4 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x08820f10
                            sampler MsgDesc: sample SIMD16 Surface = 16 Sampler = 15 mlen 4 rlen 8 { align1 };
send(8) 2       g3<1>F          null<4>UD       0x04102303
                            sampler MsgDesc: sample_l SIMD4x2 Surface = 3 Sampler = 3 mlen 2 rlen 1 { align16 };
send(8) 1       g3<1>F          g1<0>F          0x0410004a
                            math MsgDesc: pow sat mlen 2 rlen 1             { align16 };
send(8) 2       g4<1>UW         null<8,8,1>F    0x0241a004
                            sampler MsgDesc: resinfo SIMD8 Surface = 4 Sampler = 0 mlen 1 rlen 4 { align1 };
send(16) 2      g10<1>UW        null<8,8,1>F    0x0482a004
                            sampler MsgDesc: resinfo SIMD16 Surface = 4 Sampler = 0 mlen 2 rlen 8 { align1 };
send(8) 2       g4<1>UW         null<8,8,1>F    0x0241a003
                            sampler MsgDesc: resinfo SIMD8 Surface = 3 Sampler = 0 mlen 1 rlen 4 { align1 };
send(16) 2      g10<1>UW        null<8,8,1>F    0x0482a003
                            sampler MsgDesc: resinfo SIMD16 Surface = 3 Sampler = 0 mlen 2 rlen 8 { align1 };
send(8) 2       g4<1>UW         null<8,8,1>F    0x0241a002
                            sampler MsgDesc: resinfo SIMD8 Surface = 2 Sampler = 0 mlen 1 rlen 4 { align1 };
send(8) 2       g2<1>UW         null<8,8,1>F    0x04410002
                            sampler MsgDesc: sample SIMD8 Surface = 2 Sampler = 0 mlen 2 rlen 4 { align1 };
send(16) 2      g10<1>UW        null<8,8,1>F    0x0482a002
                            sampler MsgDesc: resinfo SIMD16 Surface = 2 Sampler = 0 mlen 2 rlen 8 { align1 };
send(16) 2      g4<1>UW         null<8,8,1>F    0x08820002
                            sampler MsgDesc: sample SIMD16 Surface = 2 Sampler = 0 mlen 4 rlen 8 { align1 };
send(8) 2       g5<1>F          g2<0,1,0>F      0x02100086
                            math MsgDesc: sin scalar mlen 1 rlen 1          { align1 };
