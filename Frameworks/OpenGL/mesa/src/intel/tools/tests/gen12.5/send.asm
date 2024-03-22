(+f0.0.any8h) send(1) g57UD     g58UD           nullUD          0x6210c500                0x02000000
                            ugm MsgDesc: ( load, a32, d32, V8, transpose, L1STATE_L3MOCS dst_len = 1, src0_len = 1, src1_len = 0 bti )  BTI 2  base_offset 0  { align1 WE_all 1N $5 };
(+f0.0.any8h) send(1) g28UD     g29UD           nullUD          0x6210c500                0x02000000
                            ugm MsgDesc: ( load, a32, d32, V8, transpose, L1STATE_L3MOCS dst_len = 1, src0_len = 1, src1_len = 0 bti )  BTI 2  base_offset 0  { align1 WE_all 1N $2 };
(+f0.0.any32h) send(1) g57UD    g58UD           nullUD          0x6210c500                0x02000000
                            ugm MsgDesc: ( load, a32, d32, V8, transpose, L1STATE_L3MOCS dst_len = 1, src0_len = 1, src1_len = 0 bti )  BTI 2  base_offset 0  { align1 WE_all 1N $0 };
send(8)         nullUD          g79UD           g10UD           0x6200f506                0x04000100
                            ugm MsgDesc: ( store_cmask, a32, d32, xyzw, L1STATE_L3MOCS dst_len = 0, src0_len = 1, src1_len = 4 bti )  BTI 4  base_offset 0  { align1 1Q $0 };
send(16)        nullUD          g9UD            g7UD            0x44000504                a0.1<0>UD
                            ugm MsgDesc: ( store, a32, d32, V1, L1STATE_L3MOCS dst_len = 0, src0_len = 2, src1_len = 0 ss )  surface_state_index 0  { align1 1H @1 $0 };
send(1)         g4UD            g0UD            nullUD          0x0210151f                0x00000000
                            tgm MsgDesc: ( fence, a32, tile, evict, normal_routing dst_len = 1, src0_len = 1, src1_len = 0 flat )  base_offset 0  { align1 WE_all 1N $3 };
send(8)         nullUD          g36UD           g37UD           0x02000b04                0x00000040
                            slm MsgDesc: ( store, a32, d16u32, V1, L1STATE_L3MOCS dst_len = 0, src0_len = 1, src1_len = 1 flat )  base_offset 0  { align1 1Q $1 };
send(8)         nullUD          g34UD           g35UD           0x02000b04                0x00000040
                            slm MsgDesc: ( store, a32, d16u32, V1, L1STATE_L3MOCS dst_len = 0, src0_len = 1, src1_len = 1 flat )  base_offset 0  { align1 1Q $0 };
send(8)         nullUD          g6UD            g7UD            0x0200f506                0x00000100
                            slm MsgDesc: ( store_cmask, a32, d32, xyzw, L1STATE_L3MOCS dst_len = 0, src0_len = 1, src1_len = 4 flat )  base_offset 0  { align1 1Q $6 };
send(16)        nullUD          g82UD           g91UD           0x04040519                0x00000080
                            slm MsgDesc: ( atomic_or, a32, d32, V1, L1UC_L3WB dst_len = 0, src0_len = 2, src1_len = 2 flat )  base_offset 0  { align1 2H $0 };
send(1)         g10UD           g0UD            nullUD          0x0210011f                0x00000000
                            slm MsgDesc: ( fence, a32, threadgroup, none, normal_routing dst_len = 1, src0_len = 1, src1_len = 0 flat )  base_offset 0  { align1 WE_all 1N $1 };
send(1)         g23UD           g117UD          nullUD          0x2210c500                a0.1<0>UD
                            ugm MsgDesc: ( load, a32, d32, V8, transpose, L1STATE_L3MOCS dst_len = 1, src0_len = 1, bss )  src1_len = 0 ex_bso surface_state_index 0  { align1 WE_all 1N @1 $10 };
send(8)         nullUD          g14UD           g24UD           0x040350fc                a0.1<0>UD
                            dp data 1 MsgDesc: (DC typed surface write, Surface = 252, SIMD16, Mask = 0x0)  src1_len = 4 ex_bso mlen 2 rlen 0 { align1 1Q @1 $5 };
send(8)         nullUD          g51UD           g52UD           0x02000000                0x00000040
                            rt accel MsgDesc: SIMD8,  mlen 1 ex_mlen 1 rlen 0 { align1 1Q $2 };
send(16)        nullUD          g88UD           g98UD           0x02000100                0x00000080
                            rt accel MsgDesc: SIMD16,  mlen 1 ex_mlen 2 rlen 0 { align1 1H $6 };
