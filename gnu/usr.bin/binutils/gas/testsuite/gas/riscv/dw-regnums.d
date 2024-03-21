#as: -march=rv32iv
#objdump: --dwarf=frames


.*:     file format elf.*-.*riscv

Contents of the .* section:


00000000 [a-zA-Z0-9]+ [a-zA-Z0-9]+ CIE
  Version:               .*
  Augmentation:          .*
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     .*
#...
[a-zA-Z0-9]+ [a-zA-Z0-9]+ [a-zA-Z0-9]+ FDE cie=00000000 pc=[a-zA-Z0-9]+\.\.[a-zA-Z0-9]+
  DW_CFA_advance_loc: 4 to 0+0000020
  DW_CFA_offset_extended_sf: r0 \(zero\) at cfa\+4
  DW_CFA_offset_extended_sf: r1 \(ra\) at cfa\+8
  DW_CFA_offset_extended_sf: r2 \(sp\) at cfa\+12
  DW_CFA_offset_extended_sf: r3 \(gp\) at cfa\+16
  DW_CFA_offset_extended_sf: r4 \(tp\) at cfa\+20
  DW_CFA_offset_extended_sf: r5 \(t0\) at cfa\+24
  DW_CFA_offset_extended_sf: r6 \(t1\) at cfa\+28
  DW_CFA_offset_extended_sf: r7 \(t2\) at cfa\+32
  DW_CFA_offset_extended_sf: r8 \(s0\) at cfa\+36
  DW_CFA_offset_extended_sf: r9 \(s1\) at cfa\+40
  DW_CFA_offset_extended_sf: r10 \(a0\) at cfa\+44
  DW_CFA_offset_extended_sf: r11 \(a1\) at cfa\+48
  DW_CFA_offset_extended_sf: r12 \(a2\) at cfa\+52
  DW_CFA_offset_extended_sf: r13 \(a3\) at cfa\+56
  DW_CFA_offset_extended_sf: r14 \(a4\) at cfa\+60
  DW_CFA_offset_extended_sf: r15 \(a5\) at cfa\+64
  DW_CFA_offset_extended_sf: r16 \(a6\) at cfa\+68
  DW_CFA_offset_extended_sf: r17 \(a7\) at cfa\+72
  DW_CFA_offset_extended_sf: r18 \(s2\) at cfa\+76
  DW_CFA_offset_extended_sf: r19 \(s3\) at cfa\+80
  DW_CFA_offset_extended_sf: r20 \(s4\) at cfa\+84
  DW_CFA_offset_extended_sf: r21 \(s5\) at cfa\+88
  DW_CFA_offset_extended_sf: r22 \(s6\) at cfa\+92
  DW_CFA_offset_extended_sf: r23 \(s7\) at cfa\+96
  DW_CFA_offset_extended_sf: r24 \(s8\) at cfa\+100
  DW_CFA_offset_extended_sf: r25 \(s9\) at cfa\+104
  DW_CFA_offset_extended_sf: r26 \(s10\) at cfa\+108
  DW_CFA_offset_extended_sf: r27 \(s11\) at cfa\+112
  DW_CFA_offset_extended_sf: r28 \(t3\) at cfa\+116
  DW_CFA_offset_extended_sf: r29 \(t4\) at cfa\+120
  DW_CFA_offset_extended_sf: r30 \(t5\) at cfa\+124
  DW_CFA_offset_extended_sf: r31 \(t6\) at cfa\+128
  DW_CFA_offset_extended_sf: r8 \(s0\) at cfa\+36
  DW_CFA_offset_extended_sf: r0 \(zero\) at cfa\+4
  DW_CFA_offset_extended_sf: r1 \(ra\) at cfa\+8
  DW_CFA_offset_extended_sf: r2 \(sp\) at cfa\+12
  DW_CFA_offset_extended_sf: r3 \(gp\) at cfa\+16
  DW_CFA_offset_extended_sf: r4 \(tp\) at cfa\+20
  DW_CFA_offset_extended_sf: r5 \(t0\) at cfa\+24
  DW_CFA_offset_extended_sf: r6 \(t1\) at cfa\+28
  DW_CFA_offset_extended_sf: r7 \(t2\) at cfa\+32
  DW_CFA_offset_extended_sf: r8 \(s0\) at cfa\+36
  DW_CFA_offset_extended_sf: r9 \(s1\) at cfa\+40
  DW_CFA_offset_extended_sf: r10 \(a0\) at cfa\+44
  DW_CFA_offset_extended_sf: r11 \(a1\) at cfa\+48
  DW_CFA_offset_extended_sf: r12 \(a2\) at cfa\+52
  DW_CFA_offset_extended_sf: r13 \(a3\) at cfa\+56
  DW_CFA_offset_extended_sf: r14 \(a4\) at cfa\+60
  DW_CFA_offset_extended_sf: r15 \(a5\) at cfa\+64
  DW_CFA_offset_extended_sf: r16 \(a6\) at cfa\+68
  DW_CFA_offset_extended_sf: r17 \(a7\) at cfa\+72
  DW_CFA_offset_extended_sf: r18 \(s2\) at cfa\+76
  DW_CFA_offset_extended_sf: r19 \(s3\) at cfa\+80
  DW_CFA_offset_extended_sf: r20 \(s4\) at cfa\+84
  DW_CFA_offset_extended_sf: r21 \(s5\) at cfa\+88
  DW_CFA_offset_extended_sf: r22 \(s6\) at cfa\+92
  DW_CFA_offset_extended_sf: r23 \(s7\) at cfa\+96
  DW_CFA_offset_extended_sf: r24 \(s8\) at cfa\+100
  DW_CFA_offset_extended_sf: r25 \(s9\) at cfa\+104
  DW_CFA_offset_extended_sf: r26 \(s10\) at cfa\+108
  DW_CFA_offset_extended_sf: r27 \(s11\) at cfa\+112
  DW_CFA_offset_extended_sf: r28 \(t3\) at cfa\+116
  DW_CFA_offset_extended_sf: r29 \(t4\) at cfa\+120
  DW_CFA_offset_extended_sf: r30 \(t5\) at cfa\+124
  DW_CFA_offset_extended_sf: r31 \(t6\) at cfa\+128
  DW_CFA_offset_extended_sf: r32 \(ft0\) at cfa\+132
  DW_CFA_offset_extended_sf: r33 \(ft1\) at cfa\+136
  DW_CFA_offset_extended_sf: r34 \(ft2\) at cfa\+140
  DW_CFA_offset_extended_sf: r35 \(ft3\) at cfa\+144
  DW_CFA_offset_extended_sf: r36 \(ft4\) at cfa\+148
  DW_CFA_offset_extended_sf: r37 \(ft5\) at cfa\+152
  DW_CFA_offset_extended_sf: r38 \(ft6\) at cfa\+156
  DW_CFA_offset_extended_sf: r39 \(ft7\) at cfa\+160
  DW_CFA_offset_extended_sf: r40 \(fs0\) at cfa\+164
  DW_CFA_offset_extended_sf: r41 \(fs1\) at cfa\+168
  DW_CFA_offset_extended_sf: r42 \(fa0\) at cfa\+172
  DW_CFA_offset_extended_sf: r43 \(fa1\) at cfa\+176
  DW_CFA_offset_extended_sf: r44 \(fa2\) at cfa\+180
  DW_CFA_offset_extended_sf: r45 \(fa3\) at cfa\+184
  DW_CFA_offset_extended_sf: r46 \(fa4\) at cfa\+188
  DW_CFA_offset_extended_sf: r47 \(fa5\) at cfa\+192
  DW_CFA_offset_extended_sf: r48 \(fa6\) at cfa\+196
  DW_CFA_offset_extended_sf: r49 \(fa7\) at cfa\+200
  DW_CFA_offset_extended_sf: r50 \(fs2\) at cfa\+204
  DW_CFA_offset_extended_sf: r51 \(fs3\) at cfa\+208
  DW_CFA_offset_extended_sf: r52 \(fs4\) at cfa\+212
  DW_CFA_offset_extended_sf: r53 \(fs5\) at cfa\+216
  DW_CFA_offset_extended_sf: r54 \(fs6\) at cfa\+220
  DW_CFA_offset_extended_sf: r55 \(fs7\) at cfa\+224
  DW_CFA_offset_extended_sf: r56 \(fs8\) at cfa\+228
  DW_CFA_offset_extended_sf: r57 \(fs9\) at cfa\+232
  DW_CFA_offset_extended_sf: r58 \(fs10\) at cfa\+236
  DW_CFA_offset_extended_sf: r59 \(fs11\) at cfa\+240
  DW_CFA_offset_extended_sf: r60 \(ft8\) at cfa\+244
  DW_CFA_offset_extended_sf: r61 \(ft9\) at cfa\+248
  DW_CFA_offset_extended_sf: r62 \(ft10\) at cfa\+252
  DW_CFA_offset_extended_sf: r63 \(ft11\) at cfa\+256
  DW_CFA_offset_extended_sf: r32 \(ft0\) at cfa\+132
  DW_CFA_offset_extended_sf: r33 \(ft1\) at cfa\+136
  DW_CFA_offset_extended_sf: r34 \(ft2\) at cfa\+140
  DW_CFA_offset_extended_sf: r35 \(ft3\) at cfa\+144
  DW_CFA_offset_extended_sf: r36 \(ft4\) at cfa\+148
  DW_CFA_offset_extended_sf: r37 \(ft5\) at cfa\+152
  DW_CFA_offset_extended_sf: r38 \(ft6\) at cfa\+156
  DW_CFA_offset_extended_sf: r39 \(ft7\) at cfa\+160
  DW_CFA_offset_extended_sf: r40 \(fs0\) at cfa\+164
  DW_CFA_offset_extended_sf: r41 \(fs1\) at cfa\+168
  DW_CFA_offset_extended_sf: r42 \(fa0\) at cfa\+172
  DW_CFA_offset_extended_sf: r43 \(fa1\) at cfa\+176
  DW_CFA_offset_extended_sf: r44 \(fa2\) at cfa\+180
  DW_CFA_offset_extended_sf: r45 \(fa3\) at cfa\+184
  DW_CFA_offset_extended_sf: r46 \(fa4\) at cfa\+188
  DW_CFA_offset_extended_sf: r47 \(fa5\) at cfa\+192
  DW_CFA_offset_extended_sf: r48 \(fa6\) at cfa\+196
  DW_CFA_offset_extended_sf: r49 \(fa7\) at cfa\+200
  DW_CFA_offset_extended_sf: r50 \(fs2\) at cfa\+204
  DW_CFA_offset_extended_sf: r51 \(fs3\) at cfa\+208
  DW_CFA_offset_extended_sf: r52 \(fs4\) at cfa\+212
  DW_CFA_offset_extended_sf: r53 \(fs5\) at cfa\+216
  DW_CFA_offset_extended_sf: r54 \(fs6\) at cfa\+220
  DW_CFA_offset_extended_sf: r55 \(fs7\) at cfa\+224
  DW_CFA_offset_extended_sf: r56 \(fs8\) at cfa\+228
  DW_CFA_offset_extended_sf: r57 \(fs9\) at cfa\+232
  DW_CFA_offset_extended_sf: r58 \(fs10\) at cfa\+236
  DW_CFA_offset_extended_sf: r59 \(fs11\) at cfa\+240
  DW_CFA_offset_extended_sf: r60 \(ft8\) at cfa\+244
  DW_CFA_offset_extended_sf: r61 \(ft9\) at cfa\+248
  DW_CFA_offset_extended_sf: r62 \(ft10\) at cfa\+252
  DW_CFA_offset_extended_sf: r63 \(ft11\) at cfa\+256
  DW_CFA_offset_extended_sf: r96 \(v0\) at cfa\+388
  DW_CFA_offset_extended_sf: r97 \(v1\) at cfa\+392
  DW_CFA_offset_extended_sf: r98 \(v2\) at cfa\+396
  DW_CFA_offset_extended_sf: r99 \(v3\) at cfa\+400
  DW_CFA_offset_extended_sf: r100 \(v4\) at cfa\+404
  DW_CFA_offset_extended_sf: r101 \(v5\) at cfa\+408
  DW_CFA_offset_extended_sf: r102 \(v6\) at cfa\+412
  DW_CFA_offset_extended_sf: r103 \(v7\) at cfa\+416
  DW_CFA_offset_extended_sf: r104 \(v8\) at cfa\+420
  DW_CFA_offset_extended_sf: r105 \(v9\) at cfa\+424
  DW_CFA_offset_extended_sf: r106 \(v10\) at cfa\+428
  DW_CFA_offset_extended_sf: r107 \(v11\) at cfa\+432
  DW_CFA_offset_extended_sf: r108 \(v12\) at cfa\+436
  DW_CFA_offset_extended_sf: r109 \(v13\) at cfa\+440
  DW_CFA_offset_extended_sf: r110 \(v14\) at cfa\+444
  DW_CFA_offset_extended_sf: r111 \(v15\) at cfa\+448
  DW_CFA_offset_extended_sf: r112 \(v16\) at cfa\+452
  DW_CFA_offset_extended_sf: r113 \(v17\) at cfa\+456
  DW_CFA_offset_extended_sf: r114 \(v18\) at cfa\+460
  DW_CFA_offset_extended_sf: r115 \(v19\) at cfa\+464
  DW_CFA_offset_extended_sf: r116 \(v20\) at cfa\+468
  DW_CFA_offset_extended_sf: r117 \(v21\) at cfa\+472
  DW_CFA_offset_extended_sf: r118 \(v22\) at cfa\+476
  DW_CFA_offset_extended_sf: r119 \(v23\) at cfa\+480
  DW_CFA_offset_extended_sf: r120 \(v24\) at cfa\+484
  DW_CFA_offset_extended_sf: r121 \(v25\) at cfa\+488
  DW_CFA_offset_extended_sf: r122 \(v26\) at cfa\+492
  DW_CFA_offset_extended_sf: r123 \(v27\) at cfa\+496
  DW_CFA_offset_extended_sf: r124 \(v28\) at cfa\+500
  DW_CFA_offset_extended_sf: r125 \(v29\) at cfa\+504
  DW_CFA_offset_extended_sf: r126 \(v30\) at cfa\+508
  DW_CFA_offset_extended_sf: r127 \(v31\) at cfa\+512
#...
