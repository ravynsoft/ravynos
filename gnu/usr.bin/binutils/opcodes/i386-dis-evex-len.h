static const struct dis386 evex_len_table[][3] = {
  /* EVEX_LEN_0F3816 */
  {
    { Bad_Opcode },
    { "%XEvpermp%XW",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpermp%XW",	{ XM, Vex, EXx }, PREFIX_DATA },
  },

  /* EVEX_LEN_0F3819 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3819_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3819_L_n) },
  },

  /* EVEX_LEN_0F381A_M_0 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F381A_M_0_L_n) },
    { VEX_W_TABLE (EVEX_W_0F381A_M_0_L_n) },
  },

  /* EVEX_LEN_0F381B_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F381B_M_0_L_2) },
  },

  /* EVEX_LEN_0F3836 */
  {
    { Bad_Opcode },
    { "%XEvperm%DQ",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vperm%DQ",	{ XM, Vex, EXx }, PREFIX_DATA },
  },

  /* EVEX_LEN_0F385A_M_0 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F385A_M_0_L_n) },
    { VEX_W_TABLE (EVEX_W_0F385A_M_0_L_n) },
  },

  /* EVEX_LEN_0F385B_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F385B_M_0_L_2) },
  },

  /* EVEX_LEN_0F38C6_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { REG_TABLE (REG_EVEX_0F38C6_M_0_L_2) },
  },

  /* EVEX_LEN_0F38C7_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { REG_TABLE (REG_EVEX_0F38C7_M_0_L_2) },
  },

  /* EVEX_LEN_0F3A00 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A00_L_1) },
    { VEX_W_TABLE (VEX_W_0F3A00_L_1) },
  },

  /* EVEX_LEN_0F3A01 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A01_L_1) },
    { VEX_W_TABLE (VEX_W_0F3A01_L_1) },
  },

  /* EVEX_LEN_0F3A18 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A18_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3A18_L_n) },
  },

  /* EVEX_LEN_0F3A19 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A19_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3A19_L_n) },
  },

  /* EVEX_LEN_0F3A1A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A1A_L_2) },
  },

  /* EVEX_LEN_0F3A1B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A1B_L_2) },
  },

  /* EVEX_LEN_0F3A23 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A23_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3A23_L_n) },
  },

  /* EVEX_LEN_0F3A38 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A38_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3A38_L_n) },
  },

  /* EVEX_LEN_0F3A39 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A39_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3A39_L_n) },
  },

  /* EVEX_LEN_0F3A3A */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A3A_L_2) },
  },

  /* EVEX_LEN_0F3A3B */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A3B_L_2) },
  },

  /* EVEX_LEN_0F3A43 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (EVEX_W_0F3A43_L_n) },
    { VEX_W_TABLE (EVEX_W_0F3A43_L_n) },
  },
};
