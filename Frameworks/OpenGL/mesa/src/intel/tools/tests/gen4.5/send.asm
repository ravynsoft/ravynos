send(16) 2      g12<1>F         g10<8,8,1>F     0x01110001
                            math MsgDesc: inv mlen 1 rlen 1                 { align1 compr };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 10 rlen 0 { align1 EOT };
send(8) 1       g8<1>.wF        g6<4>.wF        0x01110001
                            math MsgDesc: inv mlen 1 rlen 1                 { align16 };
send(8) 1       null<1>F        g0<4>F          0x8650c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 5 rlen 0 { align16 EOT };
send(8) 1       null<1>F        g0<4>F          0x8640c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 4 rlen 0 { align16 EOT };
send(8) 13      g0<1>F          g0<4>F          0x053190ff
                            write MsgDesc: OWord dual block write MsgCtrl = 0x0 Surface = 255 mlen 3 rlen 1 { align16 };
send(8) 14      g9<1>F          g0<4>F          0x042150ff
                            read MsgDesc: OWord Dual Block Read MsgCtrl = 0x0 Surface = 255 mlen 2 rlen 1 { align16 };
send(8) 1       null<1>F        g0<4>F          0x8680c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 8 rlen 0 { align16 EOT };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02780001
                            sampler MsgDesc: (1, 0, 0, ) mlen 7 rlen 8      { align1 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02580001
                            sampler MsgDesc: (1, 0, 0, ) mlen 5 rlen 8      { align1 };
send(8) 14      g3<1>UD         g0<4>F          0x04211000
                            read MsgDesc: OWord Dual Block Read MsgCtrl = 0x0 Surface = 0 mlen 2 rlen 1 { align16 };
send(8) 1       g6<1>.xF        g6<4>.xF        0x01110004
                            math MsgDesc: sqrt mlen 1 rlen 1                { align16 };
send(16) 1      g26<1>UW        g0<8,8,1>UW     0x02382001
                            sampler MsgDesc: (1, 0, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02983001
                            sampler MsgDesc: (1, 0, 3, ) mlen 9 rlen 8      { align1 };
send(8) 1       null<1>F        g0<4>F          0x06d04400
                            urb MsgDesc: 0 urb_write interleave used mlen 13 rlen 0 { align16 };
send(8) 1       null<1>F        g0<4>F          0x8650c460
                            urb MsgDesc: 6 urb_write interleave used complete mlen 5 rlen 0 { align16 EOT };
send(8) 1       null<1>F        g0<4>F          0x8660c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 6 rlen 0 { align16 EOT };
send(8) 2       g6<1>F          g4<8,8,1>F      0x0121000a
                            math MsgDesc: pow mlen 2 rlen 1                 { align1 };
send(16) 1      g16<1>UW        g0<8,8,1>UW     0x02380001
                            sampler MsgDesc: (1, 0, 0, ) mlen 3 rlen 8      { align1 };
send(16) 2      g6<1>F          g4<8,8,1>F      0x01110007
                            math MsgDesc: cos mlen 1 rlen 1                 { align1 compr };
send(16) 13     g8<1>UW         g0<8,8,1>F      0x02383001
                            sampler MsgDesc: (1, 0, 3, ) mlen 3 rlen 8      { align1 };
send(16) 2      g4<1>F          g2.4<0,1,0>F    0x01110081
                            math MsgDesc: inv scalar mlen 1 rlen 1          { align1 compr };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02980001
                            sampler MsgDesc: (1, 0, 0, ) mlen 9 rlen 8      { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85e04800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 14 rlen 0 { align1 EOT };
send(8) 1       null<1>F        g0<4>F          0x8670c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 7 rlen 0 { align16 EOT };
send(8) 1       null<1>UW       g0<8,8,1>UW     0x85604c00
                            write MsgDesc: RT write SIMD8 LastRT Surface = 0 mlen 6 rlen 0 { align1 EOT };
send(8) 1       null<1>F        g0<4>F          0x8680c460
                            urb MsgDesc: 6 urb_write interleave used complete mlen 8 rlen 0 { align16 EOT };
send(8) 1       g5<1>.yF        g6<4>.xF        0x01110006
                            math MsgDesc: sin mlen 1 rlen 1                 { align16 };
send(8) 1       g7<1>.xD        g1<0>.zD        0x0121001c
                            math MsgDesc: intdiv signed mlen 2 rlen 1       { align16 };
send(8) 1       null<1>F        g0<4>F          0x8640c460
                            urb MsgDesc: 6 urb_write interleave used complete mlen 4 rlen 0 { align16 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85c04800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 12 rlen 0 { align1 EOT };
send(8) 1       null<1>F        g0<4>F          0x86b0c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 11 rlen 0 { align16 EOT };
send(16) 2      g6<1>F          g4<8,8,1>F      0x01110003
                            math MsgDesc: exp mlen 1 rlen 1                 { align1 compr };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02983005
                            sampler MsgDesc: (5, 0, 3, ) mlen 9 rlen 8      { align1 };
send(16) 1      g12<1>UW        g0<8,8,1>UW     0x02983006
                            sampler MsgDesc: (6, 0, 3, ) mlen 9 rlen 8      { align1 };
send(16) 1      g20<1>UW        g0<8,8,1>UW     0x02983007
                            sampler MsgDesc: (7, 0, 3, ) mlen 9 rlen 8      { align1 };
send(16) 1      g28<1>UW        g0<8,8,1>UW     0x02983008
                            sampler MsgDesc: (8, 0, 3, ) mlen 9 rlen 8      { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04000
                            write MsgDesc: RT write SIMD16 Surface = 0 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04001
                            write MsgDesc: RT write SIMD16 Surface = 1 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04002
                            write MsgDesc: RT write SIMD16 Surface = 2 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04803
                            write MsgDesc: RT write SIMD16 LastRT Surface = 3 mlen 10 rlen 0 { align1 EOT };
send(8) 2       g4<1>D          g2.4<0,1,0>D    0x0121009c
                            math MsgDesc: intdiv signed scalar mlen 2 rlen 1 { align1 };
send(16) 14     g8<1>UW         null<8,8,1>F    0x04120301
                            read MsgDesc: OWord Block Read MsgCtrl = 0x3 Surface = 1 mlen 1 rlen 2 { align1 nomask };
send(8) 1       g30<1>.xF       (abs)g30<4>.xF  0x01110005
                            math MsgDesc: rsq mlen 1 rlen 1                 { align16 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02981001
                            sampler MsgDesc: (1, 0, 1, ) mlen 9 rlen 8      { align1 };
send(16) 2      g4<1>F          g2<0,1,0>F      0x01110086
                            math MsgDesc: sin scalar mlen 1 rlen 1          { align1 compr };
send(16) 2      g6<1>F          g2<0,1,0>F      0x01110087
                            math MsgDesc: cos scalar mlen 1 rlen 1          { align1 compr };
send(16) 2      g4<1>F          g2.1<0,1,0>F    0x01110085
                            math MsgDesc: rsq scalar mlen 1 rlen 1          { align1 compr };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85f04800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 15 rlen 0 { align1 EOT };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02982001
                            sampler MsgDesc: (1, 0, 2, ) mlen 9 rlen 8      { align1 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02580304
                            sampler MsgDesc: (4, 3, 0, ) mlen 5 rlen 8      { align1 };
send(8) 1       g5<1>.xF        g1<0>.xF        0x01110002
                            math MsgDesc: log mlen 1 rlen 1                 { align16 };
send(8) 1       g6<1>UW         g0<8,8,1>UW     0x02640001
                            sampler MsgDesc: (1, 0, 0, ) mlen 6 rlen 4      { align1 };
send(8) 1       g10<1>UW        g0<8,8,1>UW     0x02641001
                            sampler MsgDesc: (1, 0, 1, ) mlen 6 rlen 4      { align1 };
send(8) 2       g4<1>F          g2<0,1,0>F      0x012100ca
                            math MsgDesc: pow sat scalar mlen 2 rlen 1      { align1 };
send(16) 1      g16<1>UW        g0<8,8,1>UW     0x02982102
                            sampler MsgDesc: (2, 1, 2, ) mlen 9 rlen 8      { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04801
                            write MsgDesc: RT write SIMD16 LastRT Surface = 1 mlen 10 rlen 0 { align1 EOT };
send(8) 1       null<1>F        g0<4>F          0x8690c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 9 rlen 0 { align16 EOT };
send(8) 1       null<1>F        g0<4>F          0x86c0c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 12 rlen 0 { align16 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04802
                            write MsgDesc: RT write SIMD16 LastRT Surface = 2 mlen 10 rlen 0 { align1 EOT };
send(16) 1      g20<1>UW        g0<8,8,1>UW     0x02580102
                            sampler MsgDesc: (2, 1, 0, ) mlen 5 rlen 8      { align1 };
send(8) 1       null<1>F        g0<4>F          0x86a0c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 10 rlen 0 { align16 EOT };
send(16) 2      g4<1>F          g2<0,1,0>F      0x01110082
                            math MsgDesc: log scalar mlen 1 rlen 1          { align1 compr };
send(16) 1      g14<1>UW        g0<8,8,1>UW     0x02382102
                            sampler MsgDesc: (2, 1, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g26<1>UW        g0<8,8,1>UW     0x02382203
                            sampler MsgDesc: (3, 2, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g26<1>UW        g0<8,8,1>UW     0x02580203
                            sampler MsgDesc: (3, 2, 0, ) mlen 5 rlen 8      { align1 };
send(16) 1      g34<1>UW        g0<8,8,1>UW     0x02382304
                            sampler MsgDesc: (4, 3, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g42<1>UW        g0<8,8,1>UW     0x02382405
                            sampler MsgDesc: (5, 4, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g42<1>UW        g0<8,8,1>UW     0x02580405
                            sampler MsgDesc: (5, 4, 0, ) mlen 5 rlen 8      { align1 };
send(16) 1      g50<1>UW        g0<8,8,1>UW     0x02382506
                            sampler MsgDesc: (6, 5, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g50<1>UW        g0<8,8,1>UW     0x02580506
                            sampler MsgDesc: (6, 5, 0, ) mlen 5 rlen 8      { align1 };
send(16) 1      g58<1>UW        g0<8,8,1>UW     0x02382607
                            sampler MsgDesc: (7, 6, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g58<1>UW        g0<8,8,1>UW     0x02580607
                            sampler MsgDesc: (7, 6, 0, ) mlen 5 rlen 8      { align1 };
send(16) 1      g66<1>UW        g0<8,8,1>UW     0x02382708
                            sampler MsgDesc: (8, 7, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g66<1>UW        g0<8,8,1>UW     0x02580708
                            sampler MsgDesc: (8, 7, 0, ) mlen 5 rlen 8      { align1 };
send(8) 1       null<1>F        g0<4>F          0x86d0c400
                            urb MsgDesc: 0 urb_write interleave used complete mlen 13 rlen 0 { align16 EOT };
send(8) 1       g10<1>UW        g0<8,8,1>UW     0x02641102
                            sampler MsgDesc: (2, 1, 1, ) mlen 6 rlen 4      { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85b04800
                            write MsgDesc: RT write SIMD16 LastRT Surface = 0 mlen 11 rlen 0 { align1 EOT };
send(8) 2       g3<1>F          g0<4>F          0x02211505
                            sampler MsgDesc: (5, 5, 1, ) mlen 2 rlen 1      { align16 };
send(16) 2      g4<1>F          g2<0,1,0>F      0x011100c4
                            math MsgDesc: sqrt sat scalar mlen 1 rlen 1     { align1 compr };
send(16) 2      g4<1>F          g2<0,1,0>F      0x011100c3
                            math MsgDesc: exp sat scalar mlen 1 rlen 1      { align1 compr };
send(8) 2       g3<1>F          g0<4>F          0x02211000
                            sampler MsgDesc: (0, 0, 1, ) mlen 2 rlen 1      { align16 };
send(16) 13     g24<1>UW        g0<8,8,1>F      0x02383002
                            sampler MsgDesc: (2, 0, 3, ) mlen 3 rlen 8      { align1 };
send(8) 1       g3<1>F          g1<0>F          0x01110044
                            math MsgDesc: sqrt sat mlen 1 rlen 1            { align16 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02983002
                            sampler MsgDesc: (2, 0, 3, ) mlen 9 rlen 8      { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04003
                            write MsgDesc: RT write SIMD16 Surface = 3 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04804
                            write MsgDesc: RT write SIMD16 LastRT Surface = 4 mlen 10 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04004
                            write MsgDesc: RT write SIMD16 Surface = 4 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04805
                            write MsgDesc: RT write SIMD16 LastRT Surface = 5 mlen 10 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04005
                            write MsgDesc: RT write SIMD16 Surface = 5 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04806
                            write MsgDesc: RT write SIMD16 LastRT Surface = 6 mlen 10 rlen 0 { align1 EOT };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x05a04006
                            write MsgDesc: RT write SIMD16 Surface = 6 mlen 10 rlen 0 { align1 };
send(16) 1      null<1>UW       g0<8,8,1>UW     0x85a04807
                            write MsgDesc: RT write SIMD16 LastRT Surface = 7 mlen 10 rlen 0 { align1 EOT };
send(8) 1       g8<1>UW         g0<8,8,1>UW     0x02742001
                            sampler MsgDesc: (1, 0, 2, ) mlen 7 rlen 4      { align1 };
send(16) 1      g12<1>UW        g0<8,8,1>UW     0x02780102
                            sampler MsgDesc: (2, 1, 0, ) mlen 7 rlen 8      { align1 };
send(8) 1       null<1>F        g0<4>F          0x8620c460
                            urb MsgDesc: 6 urb_write interleave used complete mlen 2 rlen 0 { align16 EOT };
send(16) 2      g6<1>F          g2<0,1,0>F      0x01110084
                            math MsgDesc: sqrt scalar mlen 1 rlen 1         { align1 compr };
send(8) 1       g3<1>F          g1<0>F          0x01110043
                            math MsgDesc: exp sat mlen 1 rlen 1             { align16 };
send(8) 2       g4<1>F          g2<0,1,0>F      0x0121008a
                            math MsgDesc: pow scalar mlen 2 rlen 1          { align1 };
send(8) 1       g8<1>UW         g0<8,8,1>UW     0x02640102
                            sampler MsgDesc: (2, 1, 0, ) mlen 6 rlen 4      { align1 };
send(16) 2      g4<1>F          g2<0,1,0>F      0x01110083
                            math MsgDesc: exp scalar mlen 1 rlen 1          { align1 compr };
send(8) 1       g8<1>UW         g0<8,8,1>UW     0x02a42001
                            sampler MsgDesc: (1, 0, 2, ) mlen 10 rlen 4     { align1 };
send(16) 1      g14<1>UW        g0<8,8,1>UW     0x02580003
                            sampler MsgDesc: (3, 0, 0, ) mlen 5 rlen 8      { align1 };
send(16) 1      g22<1>UW        g0<8,8,1>UW     0x02580004
                            sampler MsgDesc: (4, 0, 0, ) mlen 5 rlen 8      { align1 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02580f10
                            sampler MsgDesc: (16, 15, 0, ) mlen 5 rlen 8    { align1 };
send(8) 2       g3<1>F          g0<4>F          0x02211303
                            sampler MsgDesc: (3, 3, 1, ) mlen 2 rlen 1      { align16 };
send(8) 1       g3<1>F          g1<0>F          0x0121004a
                            math MsgDesc: pow sat mlen 2 rlen 1             { align16 };
send(16) 1      g10<1>UW        g0<8,8,1>UW     0x02382004
                            sampler MsgDesc: (4, 0, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g10<1>UW        g0<8,8,1>UW     0x02382003
                            sampler MsgDesc: (3, 0, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g10<1>UW        g0<8,8,1>UW     0x02382002
                            sampler MsgDesc: (2, 0, 2, ) mlen 3 rlen 8      { align1 };
send(16) 1      g4<1>UW         g0<8,8,1>UW     0x02580002
                            sampler MsgDesc: (2, 0, 0, ) mlen 5 rlen 8      { align1 };
