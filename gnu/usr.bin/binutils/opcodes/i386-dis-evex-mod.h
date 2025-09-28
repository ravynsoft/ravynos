  /* MOD_EVEX_0F381A */
  {
    { EVEX_LEN_TABLE (EVEX_LEN_0F381A_M_0) },
  },
  /* MOD_EVEX_0F381B */
  {
    { EVEX_LEN_TABLE (EVEX_LEN_0F381B_M_0) },
  },
  /* MOD_EVEX_0F3828_P_1 */
  {
    { Bad_Opcode },
    { "vpmovm2%BW",	{ XM, MaskE }, 0 },
  },
  /* MOD_EVEX_0F382A_P_1_W_1 */
  {
    { Bad_Opcode },
    { "vpbroadcastmb2q",	{ XM, MaskE }, 0 },
  },
  /* MOD_EVEX_0F3838_P_1 */
  {
    { Bad_Opcode },
    { "vpmovm2%DQ",	{ XM, MaskE }, 0 },
  },
  /* MOD_EVEX_0F383A_P_1_W_0 */
  {
    { Bad_Opcode },
    { "vpbroadcastmw2d",	{ XM, MaskE }, 0 },
  },
  /* MOD_EVEX_0F385A */
  {
    { EVEX_LEN_TABLE (EVEX_LEN_0F385A_M_0) },
  },
  /* MOD_EVEX_0F385B */
  {
    { EVEX_LEN_TABLE (EVEX_LEN_0F385B_M_0) },
  },
  /* MOD_EVEX_0F387A_W_0 */
  {
    { Bad_Opcode },
    { "vpbroadcastb",	{ XM, Ed }, PREFIX_DATA },
  },
  /* MOD_EVEX_0F387B_W_0 */
  {
    { Bad_Opcode },
    { "vpbroadcastw",	{ XM, Ed }, PREFIX_DATA },
  },
  /* MOD_EVEX_0F387C */
  {
    { Bad_Opcode },
    { "vpbroadcastK",	{ XM, Edq }, PREFIX_DATA },
  },
  {
    /* MOD_EVEX_0F38C6 */
    { EVEX_LEN_TABLE (EVEX_LEN_0F38C6_M_0) },
  },
  {
    /* MOD_EVEX_0F38C7 */
    { EVEX_LEN_TABLE (EVEX_LEN_0F38C7_M_0) },
  },
