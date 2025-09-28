/* Definitions for decoding the ft32 opcode table.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by FTDI (support@ftdichip.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
02110-1301, USA.  */

typedef struct ft32_opc_info_t
{
  const char *name;
  int dw;
  unsigned int mask;
  unsigned int bits;
  int fields;
} ft32_opc_info_t;

#define FT32_PAT_ALUOP    0x08
#define FT32_PAT_LDA      0x18
#define FT32_PAT_TOCI     0x01
#define FT32_PAT_CMPOP    0x0b
#define FT32_PAT_STA      0x17
#define FT32_PAT_EXA      0x07
#define FT32_PAT_LDK      0x0c
#define FT32_PAT_FFUOP    0x1e
#define FT32_PAT_LDI      0x15
#define FT32_PAT_STI      0x16
#define FT32_PAT_EXI      0x1d
#define FT32_PAT_POP      0x11
#define FT32_PAT_LPM      0x0d
#define FT32_PAT_LINK     0x12
#define FT32_PAT_TOC      0x00
#define FT32_PAT_PUSH     0x10
#define FT32_PAT_RETURN   0x14
#define FT32_PAT_UNLINK   0x13
#define FT32_PAT_LPMI     0x19

#define FT32_FLD_CBCRCV (1 << 0)
#define FT32_FLD_INT (1 << 1)
#define FT32_FLD_INT_BIT 26
#define FT32_FLD_INT_SIZ 1
#define FT32_FLD_DW (1 << 2)
#define FT32_FLD_DW_BIT 25
#define FT32_FLD_DW_SIZ 2
#define FT32_FLD_CB (1 << 3)
#define FT32_FLD_CB_BIT 22
#define FT32_FLD_CB_SIZ 5
#define FT32_FLD_R_D (1 << 4)
#define FT32_FLD_R_D_BIT 20
#define FT32_FLD_R_D_SIZ 5
#define FT32_FLD_CR (1 << 5)
#define FT32_FLD_CR_BIT 20
#define FT32_FLD_CR_SIZ 2
#define FT32_FLD_CV (1 << 6)
#define FT32_FLD_CV_BIT 19
#define FT32_FLD_CV_SIZ 1
#define FT32_FLD_BT (1 << 7)
#define FT32_FLD_BT_BIT 18
#define FT32_FLD_BT_SIZ 1
#define FT32_FLD_R_1 (1 << 8)
#define FT32_FLD_R_1_BIT 15
#define FT32_FLD_R_1_SIZ 5
#define FT32_FLD_RIMM (1 << 9)
#define FT32_FLD_RIMM_BIT 4
#define FT32_FLD_RIMM_SIZ 11
#define FT32_FLD_R_2 (1 << 10)
#define FT32_FLD_R_2_BIT 4
#define FT32_FLD_R_2_SIZ 5
#define FT32_FLD_K20 (1 << 11)
#define FT32_FLD_K20_BIT 0
#define FT32_FLD_K20_SIZ 20
#define FT32_FLD_PA (1 << 12)
#define FT32_FLD_PA_BIT 0
#define FT32_FLD_PA_SIZ 18
#define FT32_FLD_AA (1 << 13)
#define FT32_FLD_AA_BIT 0
#define FT32_FLD_AA_SIZ 17
#define FT32_FLD_K16 (1 << 14)
#define FT32_FLD_K16_BIT 0
#define FT32_FLD_K16_SIZ 16
#define FT32_FLD_K15 (1 << 15)
#define FT32_FLD_K15_BIT 0
#define FT32_FLD_K15_SIZ 15
#define FT32_FLD_AL (1 << 16)
#define FT32_FLD_AL_BIT 0
#define FT32_FLD_AL_SIZ 4

#define FT32_IS_CALL(inst)   (((inst) & 0xfffc0000) == 0x00340000)
#define FT32_IS_PUSH(inst)   (((inst) & 0xfff00000) == 0x84000000)
#define FT32_PUSH_REG(inst)  (((inst) >> 15) & 0x1f)
#define FT32_IS_LINK(inst)   (((inst) & 0xffff0000) == 0x95d00000)
#define FT32_LINK_SIZE(inst) ((inst) & 0xffff)

#define FT32_FLD_R_D_POST (1 << 17)
#define FT32_FLD_R_1_POST (1 << 18)

static const unsigned int sc_form_0[] = {
0x44000000, 0x44000002, 0x44000004, 0x44000005, 0x4400000b,
0x44000010, 0x44000012, 0x44000014, 0x44000015, 0x44000018,
0x4400001b, 0x44000020, 0x44000022, 0x44000024, 0x44000025,
0x4400002b, 0x44000030, 0x44000032, 0x44000034, 0x44000035,
0x4400003b, 0x44000040, 0x44000042, 0x44000044, 0x44000045,
0x4400004b, 0x44000050, 0x44000060, 0x44000062, 0x44000065,
0x44000070, 0x44000072, 0x44000075, 0x44000080, 0x44000082,
0x44000090, 0x44000092, 0x440000a0, 0x440000a2, 0x440000b0,
0x440000d0, 0x440000d2, 0x440000d5, 0x440000e0, 0x440000e2,
0x440000f0, 0x440000f2, 0x440000f5, 0x44000100, 0x44000102,
0x44000110, 0x44000120, 0x44000122, 0x44000130, 0x44000140,
0x44000170, 0x44000180, 0x44000190, 0x440001a0, 0x440001b0,
0x440001e0, 0x440001f0, 0x44004000, 0x44004003, 0x4400400c,
0x4400400d, 0x44004010, 0x44004011, 0x44004013, 0x44004014,
0x44004016, 0x44004019, 0x4400401a, 0x44004020, 0x44004021,
0x44004028, 0x4400402a, 0x44004034, 0x44004038, 0x44004039,
0x44004040, 0x44004048, 0x44004058, 0x44004064, 0x44004074,
0x44004080, 0x44004081, 0x44004088, 0x44004089, 0x44004098,
0x440040c0, 0x440040ca, 0x440040f4, 0x44004100, 0x44004108,
0x44004109, 0x4400410a, 0x4400410b, 0x44004180, 0x44004182,
0x440041c0, 0x440041c2, 0x440041f4, 0x44004200, 0x44004202,
0x4400420b, 0x4400421b, 0x4400422b, 0x44004240, 0x44004242,
0x4400424b, 0x4400425b, 0x4400426b, 0x4400427b, 0x44004280,
0x44004282, 0x440042ab, 0x440042bb, 0x440042c0, 0x440042c2,
0x44004300, 0x44004340, 0x440043c3, 0x4400440b, 0x44004543,
0x44004553, 0x440047f4, 0x44004800, 0x44004980, 0x44004a0b,
0x44004a80, 0x44004dc0, 0x44004ff4, 0x44005003, 0x44005353,
0x4400537b, 0x440053e3, 0x44005700, 0x4400594b, 0x4400620b,
0x4400621b, 0x4400622b, 0x4400623b, 0x4400624b, 0x4400625b,
0x4400626b, 0x4400627b, 0x4400628b, 0x4400629b, 0x440062bb,
0x440062fb, 0x4400633b, 0x4400637b, 0x44006383, 0x44007f00,
0x44007f80, 0x44007fc0, 0x44007fe0, 0x44007ff0, 0x44007ff3, 0x44007ff6
};
static const unsigned int sc_form_1[] = {
0x44000002, 0x44008002, 0x44010002, 0x44010008, 0x44018002,
0x44020002, 0x44030002, 0x44038002
};
static const unsigned int sc_form_2[] = {
0x59e04002, 0x59e04012, 0x59e04022, 0x59e04032, 0x59e04042,
0x59e04052, 0x59e04072, 0x59e04082, 0x59e07ff2, 0x5de00002,
0x5de00012, 0x5de00022, 0x5de00032, 0x5de00042, 0x5de00052,
0x5de00062, 0x5de00072, 0x5de00082, 0x5de000a2, 0x5de000d2,
0x5de000e2, 0x5de000f2, 0x5de00102, 0x5de00112, 0x5de00122,
0x5de00132, 0x5de00172, 0x5de04002, 0x5de04012, 0x5de04022,
0x5de04032, 0x5de04042, 0x5de04052, 0x5de04062, 0x5de04072,
0x5de04082, 0x5de04092, 0x5de040f2, 0x5de04102, 0x5de04112,
0x5de04142, 0x5de04162, 0x5de041b2, 0x5de041f2, 0x5de04202,
0x5de0420c, 0x5de0421c, 0x5de0422c, 0x5de0423c, 0x5de04242,
0x5de0424c, 0x5de0425c, 0x5de0426c, 0x5de0427c, 0x5de0428c,
0x5de0429c, 0x5de042ac, 0x5de042cc, 0x5de042dc, 0x5de042ec,
0x5de042fc, 0x5de0430c, 0x5de0431c, 0x5de0433c, 0x5de0436c,
0x5de0437c, 0x5de04382, 0x5de043ac, 0x5de043bc, 0x5de043cc,
0x5de043dc, 0x5de04ff2, 0x5de07ff2, 0x84000000
};
static const unsigned int sc_form_3[] = {
0x44000010, 0x44000024, 0x44000025, 0x44000030, 0x44000034,
0x44004000, 0x4400400c, 0x4400400d, 0x44004010, 0x44004014,
0x44004028, 0x44004038, 0x44004040, 0x440040ca, 0x44004109,
0x44004880, 0x44004ac0, 0x44004e00, 0x44004e80, 0x44004ff4,
0x4400500c, 0x44005200, 0x44005680, 0x44005700, 0x4400620b,
0x44007f80, 0x44008020, 0x44008024, 0x4400c000, 0x4400c00c,
0x4400c00d, 0x4400c010, 0x4400c028, 0x4400c038, 0x4400c1f9,
0x4400cff4, 0x4400d00c, 0x4400d173, 0x4400ff80, 0x4400fff0,
0x44010030, 0x44014000, 0x4401400c, 0x4401400d, 0x44014010,
0x44014028, 0x44014074, 0x44014080, 0x440143f3, 0x44014ff4,
0x4401500c, 0x44015743, 0x44017ff0, 0x44018030, 0x4401c000,
0x4401c00c, 0x4401c010, 0x4401cff4, 0x4401d00c, 0x44020100,
0x44024000, 0x44024014, 0x44027f80, 0x4402c000, 0x44030085,
0x44034000, 0x44034109, 0x4403c000, 0x44044000, 0x44044109,
0x4404410b, 0x4404c000, 0x440680e0, 0x4406c000, 0x4406c040,
0x4406c080, 0x4406c0c0, 0x4406c300, 0x4406c3c0, 0x4406c900,
0x4406cac0, 0x4406cb40, 0x4406cbc0, 0x4406ce80, 0x4406d5c0,
0x4406d680, 0x4406d700, 0x4406d740, 0x4406d780, 0x4406dfc0,
0x44074000, 0x44074010, 0x44074e80, 0x4407c000, 0x4407c00d,
0x4407c074, 0x44084000, 0x4408c000, 0x44094000, 0x4409c000,
0x440a4000, 0x440ac000, 0x440b4000, 0x440bc000, 0x440c4000,
0x440c4040, 0x440cc000, 0x440d4000, 0x440dc000, 0x440ef340,
0x440ef400, 0x440ef440, 0x440ef4c0, 0x440ef540, 0x440ef5c0,
0x440ef6b0, 0x440ef700, 0x440ef780, 0x440ef8c0, 0x440f420d,
0x440f421d, 0x440f426d, 0x440fc180, 0x440fc1a0, 0x440fc1b0,
0x440fc1c0, 0x440fc200, 0x440fc240, 0x440fc280, 0x440fc380,
0x440fc840, 0x440fc8c0, 0x440fc900, 0x440fc980, 0x440fcd80,
0x64000000, 0x64000001, 0x64000002, 0x64000003, 0x64000004,
0x64000005, 0x64000006, 0x64000007, 0x64000008, 0x64000009,
0x6400000a, 0x6400000b, 0x6400000c, 0x6400000d, 0x6400000e,
0x6400000f, 0x64000010, 0x64000011, 0x64000012, 0x64000013,
0x64000014, 0x64000015, 0x64000017, 0x64000018, 0x64000019,
0x6400001a, 0x6400001d, 0x64000020, 0x64000023, 0x64000024,
0x64000027, 0x6400002b, 0x6400002c, 0x6400002d, 0x64000030,
0x64000035, 0x6400003c, 0x64000040, 0x64000048, 0x64000064,
0x6400006c, 0x64000080, 0x640000ff, 0x64000100, 0x640001b0,
0x640001b8, 0x64000200, 0x64000218, 0x64000240, 0x6400024c,
0x64000250, 0x640003e8, 0x64000400, 0x64000409, 0x64000554,
0x64000600, 0x64000690, 0x64000730, 0x640007ff, 0x64000800,
0x64000900, 0x64000fff, 0x64001000, 0x6400182c, 0x64001b70,
0x64001c1c, 0x64001c24, 0x64001fff, 0x64002000, 0x64003598,
0x640036ec, 0x64003fff, 0x640052c0, 0x640054e4, 0x64005a3c,
0x64005fa4, 0x64006468, 0x64006718, 0x64008000, 0x6400c000,
0x6400ffff, 0x64010000, 0x64010008, 0x640102a0, 0x64014515,
0x64040000, 0x64050000, 0x6407c000, 0x640ff800, 0x640ffc00,
0x640ffc01, 0x640ffc02, 0x640fffff, 0x8c000000, 0x94000000,
0x94000018, 0x9400001c, 0x94000020, 0x98000000, 0xa0000000,
0xa8000000, 0xa8000001, 0xa8000002, 0xa8000003, 0xa8008000,
0xa8008002, 0xa8010000, 0xa8018000, 0xa8020000, 0xa8068000,
0xa8068005, 0xa8068006, 0xa8068007, 0xa806800f, 0xa8068040,
0xa8068043, 0xa8068054, 0xa8070000, 0xa8078000, 0xa8080000,
0xa8090000, 0xa80b0000, 0xa80f801b, 0xaa000000, 0xaa008000,
0xaa008002, 0xaa00800c, 0xaa010000, 0xaa068000, 0xaa068002,
0xaa068004, 0xaa068006, 0xaa068008, 0xaa06800a, 0xaa06800c,
0xaa068024, 0xaa070000, 0xaa070002, 0xaa07000c, 0xaa078000,
0xaa078002, 0xac000000, 0xac000004, 0xac000008, 0xac00000c,
0xac000010, 0xac000014, 0xac000018, 0xac00001c, 0xac000020,
0xac000024, 0xac000028, 0xac00002c, 0xac000030, 0xac000038,
0xac00003c, 0xac000040, 0xac000044, 0xac000058, 0xac00006c,
0xac008000, 0xac008004, 0xac008008, 0xac00800c, 0xac008010,
0xac008018, 0xac008020, 0xac008024, 0xac008028, 0xac008030,
0xac008034, 0xac008038, 0xac00803c, 0xac008044, 0xac008070,
0xac008078, 0xac010000, 0xac010004, 0xac010008, 0xac01000c,
0xac010010, 0xac01002c, 0xac018000, 0xac018004, 0xac018008,
0xac020000, 0xac020004, 0xac020008, 0xac028000, 0xac030000,
0xac038000, 0xac048000, 0xac050000, 0xac068000, 0xac068004,
0xac068008, 0xac06800c, 0xac068010, 0xac068014, 0xac068018,
0xac06801c, 0xac068020, 0xac068024, 0xac068028, 0xac06802c,
0xac068030, 0xac068034, 0xac068038, 0xac06803c, 0xac068040,
0xac068044, 0xac068048, 0xac06804c, 0xac068050, 0xac068058,
0xac070000, 0xac070004, 0xac070008, 0xac07000c, 0xac070010,
0xac070014, 0xac070018, 0xac07001c, 0xac070024, 0xac070028,
0xac07002c, 0xac070038, 0xac07003c, 0xac070040, 0xac070044,
0xac070048, 0xac07004c, 0xac070054, 0xac078000, 0xac078004,
0xac078008, 0xac07800c, 0xac078024, 0xac07803c, 0xac080000,
0xac080004, 0xac080008, 0xac08003c, 0xac088000, 0xac088004,
0xac088008, 0xac08800c, 0xac090000, 0xac098000, 0xac0a0000,
0xac0a0004, 0xac0a8000, 0xac0b0000, 0xac0c0000, 0xac0d8004,
0xac0d8008, 0xac0e8090, 0xac0e8094, 0xac0e80fc, 0xac0f8018,
0xac0f801c, 0xac0f8020, 0xac0f8024, 0xac0f8028, 0xac0f802c,
0xac0f8030, 0xac0f8034, 0xac0f8038, 0xac0f803c, 0xac0f8040,
0xac0f8044, 0xac0f8048, 0xac0f804c, 0xac0f8050, 0xac0f8054,
0xac0f8058, 0xb0000000, 0xb0000006, 0xb0000009, 0xb0000018,
0xb0000019, 0xb000001b, 0xb0008000, 0xb0010000, 0xb0018000,
0xb0018087, 0xb0020000, 0xb0030000, 0xb0070000, 0xb0078000,
0xb0080000, 0xb2000000, 0xb2000006, 0xb200000c, 0xb2008000,
0xb200800c, 0xb2010000, 0xb2018000, 0xb2020000, 0xb2078000,
0xb4000000, 0xb4000004, 0xb4000008, 0xb400000c, 0xb4000010,
0xb4000014, 0xb4000018, 0xb400001c, 0xb4000020, 0xb4000024,
0xb4000028, 0xb400002c, 0xb4000030, 0xb4000034, 0xb4000038,
0xb400003c, 0xb4000040, 0xb4000044, 0xb4000048, 0xb4000050,
0xb4000054, 0xb400006c, 0xb4008000, 0xb4008004, 0xb4008008,
0xb400800c, 0xb4008010, 0xb4008014, 0xb4008018, 0xb400801c,
0xb4008020, 0xb4008024, 0xb4008028, 0xb400802c, 0xb4008034,
0xb4008038, 0xb4008040, 0xb4008044, 0xb400806c, 0xb4008070,
0xb4010000, 0xb4010004, 0xb4010008, 0xb401000c, 0xb4010010,
0xb4010018, 0xb401001c, 0xb4010020, 0xb4010024, 0xb4010028,
0xb401002c, 0xb4018000, 0xb4018004, 0xb4018008, 0xb4018018,
0xb401801c, 0xb4020000, 0xb4020004, 0xb4020008, 0xb402000c,
0xb4020018, 0xb4028000, 0xb4028018, 0xb4030000, 0xb4030008,
0xb4030018, 0xb4038000, 0xb4068000, 0xb4068004, 0xb4068008,
0xb406800c, 0xb4068018, 0xb406801c, 0xb4068020, 0xb4068024,
0xb4068028, 0xb4070000, 0xb4070004, 0xb4070008, 0xb4070010,
0xb4070018, 0xb4070024, 0xb4070028, 0xb4078000, 0xb4078024,
0xb4080000, 0xb4080004, 0xb4090000, 0xb4098000, 0xb40c8000
};

#define N_SC_FORM0  (sizeof (sc_form_0) / sizeof (unsigned int))
#define N_SC_FORM1  (sizeof (sc_form_1) / sizeof (unsigned int))
#define N_SC_FORM2  (sizeof (sc_form_2) / sizeof (unsigned int))
#define N_SC_FORM3  (sizeof (sc_form_3) / sizeof (unsigned int))

static int
sc_compar (const void *va, const void *vb)
{
  const unsigned int *a = (unsigned int *) va;
  const unsigned int *b = (unsigned int *) vb;
  return (*a - *b);
}

static int ATTRIBUTE_UNUSED
ft32_shortcode (unsigned int op32, unsigned int *sc)
{
  unsigned int Rd_mask = 31 << 20;
  unsigned int R1_mask = 31 << 15;
  unsigned int R2_mask = 2047 << 4;
  unsigned int Rd = (op32 >> 20) & 31;
  unsigned int R1 = (op32 >> 15) & 31;
  unsigned int R2 = (op32 >> 4) & 2047;
  unsigned int lookup;
  unsigned int *find = NULL;
  unsigned int code = 0, r = 0;

  /* Form 0 */
  if (Rd == R1)
    {
      lookup = op32 & ~Rd_mask & ~R1_mask;
      find = (unsigned int *) bsearch (&lookup, sc_form_0, N_SC_FORM0,
				       sizeof (unsigned int), sc_compar);
      code = find - sc_form_0;
      r = Rd;
    }

  /* Form 1 */
  if ((find == NULL) && (Rd == R2))
    {
      lookup = op32 & ~Rd_mask & ~R2_mask;
      find = (unsigned int *) bsearch (&lookup, sc_form_1, N_SC_FORM1,
				       sizeof (unsigned int), sc_compar);
      code = find - sc_form_1 + N_SC_FORM0;
      r = Rd;
    }

  /* Form 2 */
  if (find == NULL)
    {
      lookup = op32 & ~R1_mask;
      find = (unsigned int *) bsearch (&lookup, sc_form_2, N_SC_FORM2,
				       sizeof (unsigned int), sc_compar);
      code = find - sc_form_2 + (N_SC_FORM0 + N_SC_FORM1);
      r = R1;
    }

  /* Form 3 */
  if (find == NULL)
    {
      lookup = op32 & ~Rd_mask;
      find = (unsigned int *) bsearch (&lookup, sc_form_3, N_SC_FORM3,
				       sizeof (unsigned int), sc_compar);
      code = find - sc_form_3 + (N_SC_FORM0 + N_SC_FORM1 + N_SC_FORM2);
      r = Rd;
    }

  *sc = (code << 5) | r;

  return (find != NULL);
}

static int ATTRIBUTE_UNUSED
ft32_split_shortcode (unsigned int op32, unsigned int code15[2])
{
  int code3;
  unsigned int code30;

  switch (op32 >> 27)
    {
    case 2:
      code3 = 0;
      break;
    case 3:
      code3 = 1;
      break;
    case 4:
      code3 = 2;
      break;
    case 5:
      code3 = 3;
      break;
    case 6:
      code3 = 4;
      break;
    case 9:
      code3 = 5;
      break;
    case 10:
      code3 = 6;
      break;
    case 14:
      code3 = 7;
      break;
    default:
      code3 = -1;
      break;
    }

  if (code3 == -1)
    {
      code15[0] = 0;
      code15[1] = 0;
      return 0;
    }
  else
    {
      code30 = ((op32 & 0x07ffffff) << 3) | code3;
      code15[0] = code30 & 0x7fff;
      code15[1] = (code30 >> 15) & 0x7fff;
      return 1;
    }
}

static unsigned int ATTRIBUTE_UNUSED
ft32_merge_shortcode (unsigned int code15[2])
{
  static const unsigned char pat3[] = { 2, 3, 4, 5, 6, 9, 10, 14 };

  unsigned int code30 = (code15[1] << 15) | code15[0];
  unsigned int code27 = code30 >> 3;
  unsigned int code3 = code30 & 7;
  unsigned int pattern = pat3[code3];
  return (pattern << 27) | code27;
}

static int ATTRIBUTE_UNUSED
ft32_decode_shortcode (unsigned int pc, unsigned int op32, unsigned int *sc)
{
  int code3;
  unsigned int code30;
  unsigned int code15[2];
  size_t i;

  switch (op32 >> 27)
    {
    case 2:
      code3 = 0;
      break;
    case 3:
      code3 = 1;
      break;
    case 4:
      code3 = 2;
      break;
    case 5:
      code3 = 3;
      break;
    case 6:
      code3 = 4;
      break;
    case 9:
      code3 = 5;
      break;
    case 10:
      code3 = 6;
      break;
    case 14:
      code3 = 7;
      break;
    default:
      code3 = -1;
      break;
    }

  if (code3 == -1)
    return 0;
  else
    {
      code30 = ((op32 & 0x07ffffff) << 3) | code3;
      code15[0] = code30 & 0x7fff;
      code15[1] = (code30 >> 15) & 0x7fff;
      for (i = 0; i < 2; i++)
	{
	  unsigned int code = code15[i] >> 5;
	  unsigned int r = code15[i] & 0x1f;

	  if (code < 768)
	    {
	      if (code < N_SC_FORM0)
		sc[i] = sc_form_0[code] | (r << 20) | (r << 15);
	      else if (code < (N_SC_FORM0 + N_SC_FORM1))
		sc[i] = sc_form_1[code - N_SC_FORM0] | (r << 20) | (r << 4);
	      else if (code < (N_SC_FORM0 + N_SC_FORM1 + N_SC_FORM2))
		sc[i] = sc_form_2[code - (N_SC_FORM0 + N_SC_FORM1)]
		  | (r << 15);
	      else
		sc[i] = sc_form_3[code -
				  (N_SC_FORM0 + N_SC_FORM1 + N_SC_FORM2)]
		  | (r << 20);
	    }
	  else
	    {
	      int jtype = (code15[i] >> 9) & 15;
	      int offset = code15[i] & 511;
	      if (offset & 256)
		offset -= 512;
	      if (jtype < 14)
		sc[i] =
		  0x00200000 | ((jtype >> 1) << 22) | ((jtype & 1) << 19);
	      else if (jtype == 14)
		sc[i] = 0x00300000;
	      else
		sc[i] = 0x00340000;
	      sc[i] |= ((pc >> 2) + offset);
	    }
	}
      return 1;
    }
}
