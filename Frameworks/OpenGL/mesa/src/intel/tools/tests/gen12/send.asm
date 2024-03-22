send(16)        g113UD          g12UD           nullUD          a0<0>UD         0x00000000
                            dp data 1 MsgDesc: indirect ex_mlen 0           { align1 1H @1 $6 };
(+f1.0) send(16) nullUD         g15UD           g17UD           a0<0>UD         0x00000080
                            dp data 1 MsgDesc: indirect ex_mlen 2           { align1 1H @1 $4 };
send(8)         g104UD          g119UD          nullUD          0x04116e13                0x00000000
                            dp data 1 MsgDesc: (DC typed surface read, Surface = 19, SIMD8, Mask = 0xe)  mlen 2 ex_mlen 0 rlen 1 { align1 2Q $8 };
send(8)         nullUD          g92UD           g117UD          0x020350fc                a0.1<0>UD
                            dp data 1 MsgDesc: (DC typed surface write, Surface = 252, SIMD16, Mask = 0x0)  mlen 1 rlen 0 { align1 1Q @1 $8 };
(+f0.0.any8h) send(8) g55UD     g118UD          nullUD          0x02184201                0x00000000
                            data MsgDesc: (DC unaligned OWORD block read, bti 1, 2)  mlen 1 ex_mlen 0 rlen 1 { align1 WE_all 1Q @3 $9 };
send(8)         nullUD          g126UD          nullUD          0x02000000                0x00000000
                            thread_spawner MsgDesc:  mlen 1 ex_mlen 0 rlen 0 { align1 WE_all 1Q @1 EOT };
send(8)         g18UD           g24UD           nullUD          0x04115e10                0x00000000
                            dp data 1 MsgDesc: (DC typed surface read, Surface = 16, SIMD16, Mask = 0xe)  mlen 2 ex_mlen 0 rlen 1 { align1 1Q $1 };
send(8)         g19UD           g28UD           nullUD          0x04116e10                0x00000000
                            dp data 1 MsgDesc: (DC typed surface read, Surface = 16, SIMD8, Mask = 0xe)  mlen 2 ex_mlen 0 rlen 1 { align1 2Q @7 $2 };
send(16)        g50UD           g36UD           nullUD          a0<0>UD         0x00000000
                            sampler MsgDesc: indirect ex_mlen 0             { align1 1H @1 $3 };
send(8)         nullUD          g25UD           g21UD           0x02035001                0x00000100
                            dp data 1 MsgDesc: (DC typed surface write, Surface = 1, SIMD16, Mask = 0x0)  mlen 1 ex_mlen 4 rlen 0 { align1 1Q $9 };
send(8)         g5UD            g25UD           nullUD          0x02415001                0x00000000
                            dp data 1 MsgDesc: (DC typed surface read, Surface = 1, SIMD16, Mask = 0x0)  mlen 1 ex_mlen 0 rlen 4 { align1 1Q $10 };
send(8)         g27UD           g35UD           nullUD          0x04146efd                0x00000000
                            dp data 1 MsgDesc: (DC A64 untyped surface read, Surface = 253, SIMD8, Mask = 0xe)  mlen 2 ex_mlen 0 rlen 1 { align1 1Q @1 $0 };
send(8)         nullUD          g36UD           g38UD           0x04035001                0x00000100
                            dp data 1 MsgDesc: (DC typed surface write, Surface = 1, SIMD16, Mask = 0x0)  mlen 2 ex_mlen 4 rlen 0 { align1 1Q @1 $1 };
send(8)         nullUD          g126UD          g118UD          0x02080007                0x00000200
                            urb MsgDesc: offset 0 SIMD8 write  mlen 1 ex_mlen 8 rlen 0 { align1 1Q @1 EOT };
send(8)         g14UD           g37UD           nullUD          0x02110401                0x00000000
                            data MsgDesc: (DC byte scattered read, bti 1, 4)  mlen 1 ex_mlen 0 rlen 1 { align1 1Q @1 $0 };
send(1)         g100UD          g0UD            nullUD          0x0219e000                0x00000000
                            data MsgDesc: (DC mfence, bti 0, 32)  mlen 1 ex_mlen 0 rlen 1 { align1 WE_all 1N $1 };
send(1)         g15UD           g0UD            nullUD          0x0219e000                0x00000000
                            data MsgDesc: (DC mfence, bti 0, 32)  mlen 1 ex_mlen 0 rlen 1 { align1 WE_all 1N $5 };

sendc(16)       nullUD          g119UD          nullUD          0x10031000                0x00000000
                            render MsgDesc: RT write SIMD16 LastRT Surface = 0  mlen 8 ex_mlen 0 rlen 0 { align1 1H @1 EOT };
sendc(8)        nullUD          g125UD          g123UD          0x04031400                0x00000080
                            render MsgDesc: RT write SIMD8 LastRT Surface = 0  mlen 2 ex_mlen 2 rlen 0 { align1 1Q @1 EOT };
sendc(16)       nullUD          g119UD          nullUD          0x10031000                0x00000000
                            render MsgDesc: RT write SIMD16 LastRT Surface = 0  mlen 8 ex_mlen 0 rlen 0 { align1 1H @1 EOT };
sendc(16)       nullUD          g123UD          g119UD          0x08031000                0x00000100
                            render MsgDesc: RT write SIMD16 LastRT Surface = 0  mlen 4 ex_mlen 4 rlen 0 { align1 1H @1 EOT };
