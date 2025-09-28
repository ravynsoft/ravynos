  /* EVEX_W_0F5B_P_0 */
  {
    { "%XEvcvtdq2ps",	{ XM, EXx, EXxEVexR }, 0 },
    { "vcvtqq2ps%XY",	{ XMxmmq, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0F62 */
  {
    { "%XEvpunpckldq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F66 */
  {
    { "vpcmpgtd",	{ MaskG, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F6A */
  {
    { "%XEvpunpckhdq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F6B */
  {
    { "%XEvpackssdw",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F6C */
  {
    { Bad_Opcode },
    { "%XEvpunpcklqdq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F6D */
  {
    { Bad_Opcode },
    { "%XEvpunpckhqdq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F6F_P_1 */
  {
    { "vmovdqu32",	{ XM, EXEvexXNoBcst }, 0 },
    { "vmovdqu64",	{ XM, EXEvexXNoBcst }, 0 },
  },
  /* EVEX_W_0F6F_P_2 */
  {
    { "vmovdqa32",	{ XM, EXEvexXNoBcst }, 0 },
    { "vmovdqa64",	{ XM, EXEvexXNoBcst }, 0 },
  },
  /* EVEX_W_0F6F_P_3 */
  {
    { "vmovdqu8",	{ XM, EXx }, 0 },
    { "vmovdqu16",	{ XM, EXx }, 0 },
  },
  /* EVEX_W_0F70_P_2 */
  {
    { "%XEvpshufd",	{ XM, EXx, Ib }, 0 },
  },
  /* EVEX_W_0F72_R_2 */
  {
    { "%XEvpsrld",	{ Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F72_R_6 */
  {
    { "%XEvpslld",	{ Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F73_R_2 */
  {
    { Bad_Opcode },
    { "%XEvpsrlq",	{ Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F73_R_6 */
  {
    { Bad_Opcode },
    { "%XEvpsllq",	{ Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F76 */
  {
    { "vpcmpeqd",	{ MaskG, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F78_P_0 */
  {
    { "vcvttps2udq",	{ XM, EXx, EXxEVexS }, 0 },
    { "vcvttpd2udq%XY",	{ XMxmmq, EXx, EXxEVexS }, 0 },
  },
  /* EVEX_W_0F78_P_2 */
  {
    { "vcvttps2uqq",	{ XM, EXEvexHalfBcstXmmq, EXxEVexS }, 0 },
    { "vcvttpd2uqq",	{ XM, EXx, EXxEVexS }, 0 },
  },
  /* EVEX_W_0F79_P_0 */
  {
    { "vcvtps2udq",	{ XM, EXx, EXxEVexR }, 0 },
    { "vcvtpd2udq%XY",	{ XMxmmq, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0F79_P_2 */
  {
    { "vcvtps2uqq",	{ XM, EXEvexHalfBcstXmmq, EXxEVexR }, 0 },
    { "vcvtpd2uqq",	{ XM, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0F7A_P_1 */
  {
    { "vcvtudq2pd",	{ XM, EXEvexHalfBcstXmmq }, 0 },
    { "vcvtuqq2pd",	{ XM, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0F7A_P_2 */
  {
    { "vcvttps2qq",	{ XM, EXEvexHalfBcstXmmq, EXxEVexS }, 0 },
    { "vcvttpd2qq",	{ XM, EXx, EXxEVexS }, 0 },
  },
  /* EVEX_W_0F7A_P_3 */
  {
    { "vcvtudq2ps",	{ XM, EXx, EXxEVexR }, 0 },
    { "vcvtuqq2ps%XY",	{ XMxmmq, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0F7B_P_2 */
  {
    { "vcvtps2qq",	{ XM, EXEvexHalfBcstXmmq, EXxEVexR }, 0 },
    { "vcvtpd2qq",	{ XM, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0F7E_P_1 */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F7E_P_1) },
  },
  /* EVEX_W_0F7F_P_1 */
  {
    { "vmovdqu32",	{ EXxS, XM }, 0 },
    { "vmovdqu64",	{ EXxS, XM }, 0 },
  },
  /* EVEX_W_0F7F_P_2 */
  {
    { "vmovdqa32",	{ EXxS, XM }, 0 },
    { "vmovdqa64",	{ EXxS, XM }, 0 },
  },
  /* EVEX_W_0F7F_P_3 */
  {
    { "vmovdqu8",	{ EXxS, XM }, 0 },
    { "vmovdqu16",	{ EXxS, XM }, 0 },
  },
  /* EVEX_W_0FD2 */
  {
    { "%XEvpsrld",	{ XM, Vex, EXxmm }, PREFIX_DATA },
  },
  /* EVEX_W_0FD3 */
  {
    { Bad_Opcode },
    { "%XEvpsrlq",	{ XM, Vex, EXxmm }, PREFIX_DATA },
  },
  /* EVEX_W_0FD4 */
  {
    { Bad_Opcode },
    { "%XEvpaddq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0FD6 */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FD6) },
  },
  /* EVEX_W_0FE6_P_1 */
  {
    { "%XEvcvtdq2pd",	{ XM, EXEvexHalfBcstXmmq }, 0 },
    { "vcvtqq2pd",	{ XM, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_0FE7 */
  {
    { "%XEvmovntdq",	{ EXEvexXNoBcst, XM }, PREFIX_DATA },
  },
  /* EVEX_W_0FF2 */
  {
    { "%XEvpslld",		{ XM, Vex, EXxmm }, PREFIX_DATA },
  },
  /* EVEX_W_0FF3 */
  {
    { Bad_Opcode },
    { "%XEvpsllq",		{ XM, Vex, EXxmm }, PREFIX_DATA },
  },
  /* EVEX_W_0FF4 */
  {
    { Bad_Opcode },
    { "%XEvpmuludq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0FFA */
  {
    { "%XEvpsubd",		{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0FFB */
  {
    { Bad_Opcode },
    { "%XEvpsubq",		{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0FFE */
  {
    { "%XEvpaddd",		{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F3810_P_1 */
  {
    { "vpmovuswb",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3810_P_2 */
  {
    { Bad_Opcode },
    { "vpsrlvw",	{ XM, Vex, EXx }, 0 },
  },
  /* EVEX_W_0F3811_P_1 */
  {
    { "vpmovusdb",	{ EXxmmqd, XM }, 0 },
  },
  /* EVEX_W_0F3811_P_2 */
  {
    { Bad_Opcode },
    { "vpsravw",	{ XM, Vex, EXx }, 0 },
  },
  /* EVEX_W_0F3812_P_1 */
  {
    { "vpmovusqb",	{ EXxmmdw, XM }, 0 },
  },
  /* EVEX_W_0F3812_P_2 */
  {
    { Bad_Opcode },
    { "vpsllvw",	{ XM, Vex, EXx }, 0 },
  },
  /* EVEX_W_0F3813_P_1 */
  {
    { "vpmovusdw",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3814_P_1 */
  {
    { "vpmovusqw",	{ EXxmmqd, XM }, 0 },
  },
  /* EVEX_W_0F3815_P_1 */
  {
    { "vpmovusqd",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3819_L_n */
  {
    { "vbroadcastf32x2",	{ XM, EXq }, PREFIX_DATA },
    { "%XEvbroadcastsd",	{ XM, EXq }, PREFIX_DATA },
  },
  /* EVEX_W_0F381A_M_0_L_n */
  {
    { "vbroadcastf32x4",	{ XM, EXxmm }, PREFIX_DATA },
    { "vbroadcastf64x2",	{ XM, EXxmm }, PREFIX_DATA },
  },
  /* EVEX_W_0F381B_M_0_L_2 */
  {
    { "vbroadcastf32x8",	{ XM, EXymm }, PREFIX_DATA },
    { "vbroadcastf64x4",	{ XM, EXymm }, PREFIX_DATA },
  },
  /* EVEX_W_0F381E */
  {
    { "%XEvpabsd",	{ XM, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F381F */
  {
    { Bad_Opcode },
    { "vpabsq",	{ XM, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F3820_P_1 */
  {
    { "vpmovswb",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3821_P_1 */
  {
    { "vpmovsdb",	{ EXxmmqd, XM }, 0 },
  },
  /* EVEX_W_0F3822_P_1 */
  {
    { "vpmovsqb",	{ EXxmmdw, XM }, 0 },
  },
  /* EVEX_W_0F3823_P_1 */
  {
    { "vpmovsdw",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3824_P_1 */
  {
    { "vpmovsqw",	{ EXxmmqd, XM }, 0 },
  },
  /* EVEX_W_0F3825_P_1 */
  {
    { "vpmovsqd",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3825_P_2 */
  {
    { "%XEvpmovsxdq",	{ XM, EXxmmq }, 0 },
  },
  /* EVEX_W_0F3828_P_2 */
  {
    { Bad_Opcode },
    { "%XEvpmuldq",	{ XM, Vex, EXx }, 0 },
  },
  /* EVEX_W_0F3829_P_2 */
  {
    { Bad_Opcode },
    { "vpcmpeqq",	{ MaskG, Vex, EXx }, 0 },
  },
  /* EVEX_W_0F382A_P_1 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_EVEX_0F382A_P_1_W_1) },
  },
  /* EVEX_W_0F382A_P_2 */
  {
    { "%XEvmovntdqa",	{ XM, EXEvexXNoBcst }, 0 },
  },
  /* EVEX_W_0F382B */
  {
    { "%XEvpackusdw",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F3830_P_1 */
  {
    { "vpmovwb",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3831_P_1 */
  {
    { "vpmovdb",	{ EXxmmqd, XM }, 0 },
  },
  /* EVEX_W_0F3832_P_1 */
  {
    { "vpmovqb",	{ EXxmmdw, XM }, 0 },
  },
  /* EVEX_W_0F3833_P_1 */
  {
    { "vpmovdw",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3834_P_1 */
  {
    { "vpmovqw",	{ EXxmmqd, XM }, 0 },
  },
  /* EVEX_W_0F3835_P_1 */
  {
    { "vpmovqd",	{ EXxmmq, XM }, 0 },
  },
  /* EVEX_W_0F3835_P_2 */
  {
    { "%XEvpmovzxdq",	{ XM, EXxmmq }, 0 },
  },
  /* EVEX_W_0F3837 */
  {
    { Bad_Opcode },
    { "vpcmpgtq",	{ MaskG, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F383A_P_1 */
  {
    { MOD_TABLE (MOD_EVEX_0F383A_P_1_W_0) },
  },
  /* EVEX_W_0F3859 */
  {
    { "vbroadcasti32x2",	{ XM, EXq }, PREFIX_DATA },
    { "%XEvpbroadcastq",	{ XM, EXq }, PREFIX_DATA },
  },
  /* EVEX_W_0F385A_M_0_L_n */
  {
    { "vbroadcasti32x4",	{ XM, EXxmm }, PREFIX_DATA },
    { "vbroadcasti64x2",	{ XM, EXxmm }, PREFIX_DATA },
  },
  /* EVEX_W_0F385B_M_0_L_2 */
  {
    { "vbroadcasti32x8",	{ XM, EXymm }, PREFIX_DATA },
    { "vbroadcasti64x4",	{ XM, EXymm }, PREFIX_DATA },
  },
  /* EVEX_W_0F3870 */
  {
    { Bad_Opcode },
    { "vpshldvw",  { XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F3872_P_2 */
  {
    { Bad_Opcode },
    { "vpshrdvw",  { XM, Vex, EXx }, 0 },
  },
  /* EVEX_W_0F387A */
  {
    { MOD_TABLE (MOD_EVEX_0F387A_W_0) },
  },
  /* EVEX_W_0F387B */
  {
    { MOD_TABLE (MOD_EVEX_0F387B_W_0) },
  },
  /* EVEX_W_0F3883 */
  {
    { Bad_Opcode },
    { "vpmultishiftqb",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A18_L_n */
  {
    { "vinsertf32x4",	{ XM, Vex, EXxmm, Ib }, PREFIX_DATA },
    { "vinsertf64x2",	{ XM, Vex, EXxmm, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A19_L_n */
  {
    { "vextractf32x4",	{ EXxmm, XM, Ib }, PREFIX_DATA },
    { "vextractf64x2",	{ EXxmm, XM, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A1A_L_2 */
  {
    { "vinsertf32x8",	{ XM, Vex, EXymm, Ib }, PREFIX_DATA },
    { "vinsertf64x4",	{ XM, Vex, EXymm, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A1B_L_2 */
  {
    { "vextractf32x8",	{ EXymm, XM, Ib }, PREFIX_DATA },
    { "vextractf64x4",	{ EXymm, XM, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A21 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0F3A21) },
  },
  /* EVEX_W_0F3A23_L_n */
  {
    { "vshuff32x4",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { "vshuff64x2",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A38_L_n */
  {
    { "vinserti32x4",	{ XM, Vex, EXxmm, Ib }, PREFIX_DATA },
    { "vinserti64x2",	{ XM, Vex, EXxmm, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A39_L_n */
  {
    { "vextracti32x4",	{ EXxmm, XM, Ib }, PREFIX_DATA },
    { "vextracti64x2",	{ EXxmm, XM, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A3A_L_2 */
  {
    { "vinserti32x8",	{ XM, Vex, EXymm, Ib }, PREFIX_DATA },
    { "vinserti64x4",	{ XM, Vex, EXymm, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A3B_L_2 */
  {
    { "vextracti32x8",	{ EXymm, XM, Ib }, PREFIX_DATA },
    { "vextracti64x4",	{ EXymm, XM, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A42 */
  {
    { "vdbpsadbw",	{ XM, Vex, EXx, Ib }, 0 },
  },
  /* EVEX_W_0F3A43_L_n */
  {
    { "vshufi32x4",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { "vshufi64x2",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* EVEX_W_0F3A70 */
  {
    { Bad_Opcode },
    { "vpshldw",   { XM, Vex, EXx, Ib }, 0 },
  },
  /* EVEX_W_0F3A72 */
  {
    { Bad_Opcode },
    { "vpshrdw",   { XM, Vex, EXx, Ib }, 0 },
  },
  /* EVEX_W_MAP5_5B_P_0 */
  {
    { "vcvtdq2ph%XY",	{ XMxmmq, EXx, EXxEVexR }, 0 },
    { "vcvtqq2ph%XZ",	{ XMM, EXx, EXxEVexR }, 0 },
  },
  /* EVEX_W_MAP5_7A_P_3 */
  {
    { "vcvtudq2ph%XY",	{ XMxmmq, EXx, EXxEVexR }, 0 },
    { "vcvtuqq2ph%XZ",	{ XMM, EXx, EXxEVexR }, 0 },
  },
