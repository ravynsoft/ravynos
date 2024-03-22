#define NV_MME_PRED_MODE_UUUU                0
#define NV_MME_PRED_MODE_TTTT                1
#define NV_MME_PRED_MODE_FFFF                2
#define NV_MME_PRED_MODE_TTUU                3
#define NV_MME_PRED_MODE_FFUU                4
#define NV_MME_PRED_MODE_TFUU                5
#define NV_MME_PRED_MODE_TUUU                6
#define NV_MME_PRED_MODE_FUUU                7
#define NV_MME_PRED_MODE_UUTT                8
#define NV_MME_PRED_MODE_UUTF                9
#define NV_MME_PRED_MODE_UUTU                10
#define NV_MME_PRED_MODE_UUFT                11
#define NV_MME_PRED_MODE_UUFF                12
#define NV_MME_PRED_MODE_UUFU                13
#define NV_MME_PRED_MODE_UUUT                14
#define NV_MME_PRED_MODE_UUUF                15

#define NV_MME_REG_R0                       0
#define NV_MME_REG_R1                       1
#define NV_MME_REG_R2                       2
#define NV_MME_REG_R3                       3
#define NV_MME_REG_R4                       4
#define NV_MME_REG_R5                       5
#define NV_MME_REG_R6                       6
#define NV_MME_REG_R7                       7
#define NV_MME_REG_R8                       8
#define NV_MME_REG_R9                       9
#define NV_MME_REG_R10                      10
#define NV_MME_REG_R11                      11
#define NV_MME_REG_R12                      12
#define NV_MME_REG_R13                      13
#define NV_MME_REG_R14                      14
#define NV_MME_REG_R15                      15
#define NV_MME_REG_R16                      16
#define NV_MME_REG_R17                      17
#define NV_MME_REG_R18                      18
#define NV_MME_REG_R19                      19
#define NV_MME_REG_R20                      20
#define NV_MME_REG_R21                      21
#define NV_MME_REG_R22                      22
#define NV_MME_REG_R23                      23
#define NV_MME_REG_ZERO                     24
#define NV_MME_REG_IMMED                    25
#define NV_MME_REG_IMMEDPAIR                26
#define NV_MME_REG_IMMED32                  27
#define NV_MME_REG_LOAD0                    28
#define NV_MME_REG_LOAD1                    29

#define NV_MME_ALU_ADD                    0
#define NV_MME_ALU_ADDC                   1
#define NV_MME_ALU_SUB                    2
#define NV_MME_ALU_SUBB                   3
#define NV_MME_ALU_MUL                    4
#define NV_MME_ALU_MULH                   5
#define NV_MME_ALU_MULU                   6
#define NV_MME_ALU_EXTENDED               7
#define NV_MME_ALU_CLZ                    8
#define NV_MME_ALU_SLL                    9
#define NV_MME_ALU_SRL                    10
#define NV_MME_ALU_SRA                    11
#define NV_MME_ALU_AND                    12
#define NV_MME_ALU_NAND                   13
#define NV_MME_ALU_OR                     14
#define NV_MME_ALU_XOR                    15
#define NV_MME_ALU_MERGE                  16
#define NV_MME_ALU_SLT                    17
#define NV_MME_ALU_SLTU                   18
#define NV_MME_ALU_SLE                    19
#define NV_MME_ALU_SLEU                   20
#define NV_MME_ALU_SEQ                    21
#define NV_MME_ALU_STATE                  22
#define NV_MME_ALU_LOOP                   23
#define NV_MME_ALU_JAL                    24
#define NV_MME_ALU_BLT                    25
#define NV_MME_ALU_BLTU                   26
#define NV_MME_ALU_BLE                    27
#define NV_MME_ALU_BLEU                   28
#define NV_MME_ALU_BEQ                    29
#define NV_MME_ALU_DREAD                  30
#define NV_MME_ALU_DWRITE                 31

#define NV_MME_OUT_NONE                 0
#define NV_MME_OUT_ALU0                 1
#define NV_MME_OUT_ALU1                 2
#define NV_MME_OUT_LOAD0                3
#define NV_MME_OUT_LOAD1                4
#define NV_MME_OUT_IMMED0               5
#define NV_MME_OUT_IMMED1               6
#define NV_MME_OUT_RESERVED             7
#define NV_MME_OUT_IMMEDHIGH0           8
#define NV_MME_OUT_IMMEDHIGH1           9
#define NV_MME_OUT_IMMED32_0            10

#define MME_BITS(en,pm,pr,o0,d0,a0,b0,i0,o1,d1,a1,b1,i1,m0,e0,m1,e1)           \
   ((e1) << (92 - 64) | (m1) << (89 - 64) |                                    \
    (e0) << (85 - 64) | (m0) << (82 - 64) |                                    \
    (i1) << (66 - 64) | (b1) >> (64 - 61)),                                    \
   (((b1) & 7)  << (61 - 32) | (a1) << (56 - 32) |                             \
    (d1) << (51 - 32) | (o1) << (46 - 32) |                                    \
    (i0) >> (32 - 30)),                                                        \
   (((i0) & 3) << 30 | (b0) << 25 | (a0) << 20 | (d0) << 15 | (o0) << 10 |     \
    (pr) << 5 | (pm) << 1 | (en))

#define MME_INSN(en,o0,d0,a0,b0,i0,m0,e0,o1,d1,a1,b1,i1,m1,e1)                 \
   MME_BITS((en), NV_MME_PRED_MODE_UUUU, NV_MME_REG_ZERO,                      \
            NV_MME_ALU_##o0, NV_MME_REG_##d0,                               \
            NV_MME_REG_##a0, NV_MME_REG_##b0, (i0),                            \
            NV_MME_ALU_##o1, NV_MME_REG_##d1,                               \
            NV_MME_REG_##a1, NV_MME_REG_##b1, (i1),                            \
            NV_MME_OUT_##m0, NV_MME_OUT_##e0,                                  \
            NV_MME_OUT_##m1, NV_MME_OUT_##e1)

const uint32_t mmec597_per_instance_bf[] = {
// r1 = load();      // count
// r3 = load();      // mask
// mthd(0x1880, 1);  // VERTEX_ARRAY_PER_INSTANCE[0]
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,  (1<<12)|0x1880/4, IMMED0,   NONE,
                 ADD,   R3, LOAD1,  ZERO,                 0,   NONE,   NONE),
// while (HW_LOOP_COUNT < r1) {
//    send(r3 & 1);
//    r3 >>= 1;
// }
   MME_INSN(0,  LOOP, ZERO,    R1,  ZERO,            0x0003,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   AND, ZERO,    R3, IMMED,                 1,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   SRL,   R3,    R3, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_vertex_array_select[] = {
// r1 = load();            // array
// r2 = load();            // limit hi
// r3 = load();            // limit lo
// r4 = load();            // start hi
// r5 = load();            // start lo
// r6 = (r1 & 0x1f) << 2;
// r7 = (r1 & 0x1f) << 1;
// mthd(0x1c04 + r6, 1);   // VERTEX_ARRAY_START_HIGH[]
// send(r4);
// send(r5);
// mthd(0x0600 + r7, 1);   // VERTEX_ARRAY_LIMIT_HIGH[]
// send(r2);
// send(r3);
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD,   R2, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R3, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD,   R4, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R5, LOAD0,  ZERO,                 0,   NONE,   NONE,
               MERGE,   R6,  ZERO,    R1,  (2<<10)|(5<<5)|0,   NONE,   NONE),
   MME_INSN(0, MERGE,   R7,  ZERO,    R1,  (1<<10)|(5<<5)|0,   ALU1,   NONE,
                 ADD, ZERO,    R6, IMMED,  (1<<12)|0x1c04/4,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,    R5,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(1,   ADD, ZERO,    R7, IMMED,  (1<<12)|0x0600/4,   ALU0,   ALU1,
                 ADD, ZERO,    R2,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R3,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_blend_enables[] = {
// r1 = load();         // enable mask
// mthd(0x1360, 1);     // NVC0_3D_BLEND_ENABLE[]
// send((r1 >> 0) & 1);
// send((r1 >> 1) & 1);
// send((r1 >> 2) & 1);
// send((r1 >> 3) & 1);
// send((r1 >> 4) & 1);
// send((r1 >> 5) & 1);
// send((r1 >> 6) & 1);
// send((r1 >> 7) & 1);
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                 0, IMMED1,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,  (1<<12)|0x1360/4,   NONE,   NONE),
   MME_INSN(0, MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|0,   NONE,   ALU0,
               MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|1,   NONE,   ALU1),
   MME_INSN(0, MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|2,   NONE,   ALU0,
               MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|3,   NONE,   ALU1),
   MME_INSN(1, MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|4,   NONE,   ALU0,
               MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|5,   NONE,   ALU1),
   MME_INSN(0, MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|6,   NONE,   ALU0,
               MERGE, ZERO,  ZERO,    R1,  (0<<10)|(1<<5)|7,   NONE,   ALU1),
};

const uint32_t mmec597_poly_mode_front[] = {
// r1 = load();
// mthd(0x0dac,0);      // POLYGON_MODE_FRONT
// send(r1);
// r2 = read(0x0db0);   // POLYGON_MODE_BACK
// r3 = read(0x20c0);   // SP_SELECT[3]
// r7 = r1 | r2;
// r4 = read(0x2100);   // SP_SELECT[4]
// r6 = 0x60;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,  (0<<12)|0x0dac/4, IMMED0,   ALU0,
               STATE,   R2, IMMED,  ZERO,          0x0db0/4,   NONE,   NONE),
   MME_INSN(0, STATE,   R3, IMMED,  ZERO,          0x20c0/4,   NONE,   NONE,
                  OR,   R7,    R1,    R2,                 0,   NONE,   NONE),
   MME_INSN(0, STATE,   R4, IMMED,  ZERO,          0x2100/4,   NONE,   NONE,
                 ADD,   R6, IMMED,  ZERO,              0x60,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0x200;
   MME_INSN(0,   ADD,   R6, IMMED,  ZERO,             0x200,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// r7 = r3 | r4;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,    OR,   R7,    R3,    R4,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0;
   MME_INSN(0,   ADD,   R6,  ZERO,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x02ec, 0);
// send(r6);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,  (0<<12)|0x02ec/4, IMMED0,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_poly_mode_back[] = {
// r1 = load();
// mthd(0x0db0,0);      // POLYGON_MODE_BACK
// send(r1);
// r2 = read(0x0dac);   // POLYGON_MODE_FRONT
// r3 = read(0x20c0);   // SP_SELECT[3]
// r7 = r1 | r2;
// r4 = read(0x2100);   // SP_SELECT[4]
// r6 = 0x60;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,  (0<<12)|0x0db0/4, IMMED0,   ALU0,
               STATE,   R2, IMMED,  ZERO,          0x0dac/4,   NONE,   NONE),
   MME_INSN(0, STATE,   R3, IMMED,  ZERO,          0x20c0/4,   NONE,   NONE,
                  OR,   R7,    R1,    R2,                 0,   NONE,   NONE),
   MME_INSN(0, STATE,   R4, IMMED,  ZERO,          0x2100/4,   NONE,   NONE,
                 ADD,   R6, IMMED,  ZERO,              0x60,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0x200;
   MME_INSN(0,   ADD,   R6, IMMED,  ZERO,             0x200,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// r7 = r3 | r4;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,    OR,   R7,    R3,    R4,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0;
   MME_INSN(0,   ADD,   R6,  ZERO,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x02ec, 0);
// send(r6);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,  (0<<12)|0x02ec/4, IMMED0,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_gp_select[] = {
// r1 = load();
// mthd(0x2100,0);      // SP_SELECT[4]
// send(r1);
// r2 = read(0x0dac);   // POLYGON_MODE_FRONT
// r3 = read(0x0db0);   // POLYGON_MODE_BACK
// r7 = r2 | r3;
// r4 = read(0x20c0);   // SP_SELECT[3]
// r6 = 0x60;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,  (0<<12)|0x2100/4, IMMED0,   ALU0,
               STATE,   R2, IMMED,  ZERO,          0x0dac/4,   NONE,   NONE),
   MME_INSN(0, STATE,   R3, IMMED,  ZERO,          0x0db0/4,   NONE,   NONE,
                  OR,   R7,    R2,    R3,                 0,   NONE,   NONE),
   MME_INSN(0, STATE,   R4, IMMED,  ZERO,          0x20c0/4,   NONE,   NONE,
                 ADD,   R6, IMMED,  ZERO,              0x60,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0x200;
   MME_INSN(0,   ADD,   R6, IMMED,  ZERO,             0x200,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// r7 = r1 | r4;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,    OR,   R7,    R1,    R4,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0;
   MME_INSN(0,   ADD,   R6,  ZERO,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x02ec, 0);
// send(r6);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,  (0<<12)|0x02ec/4, IMMED0,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_tep_select[] = {
// r1 = load();
// mthd(0x20c0,0);      // SP_SELECT[3]
// send(r1);
// r2 = read(0x0dac);   // POLYGON_MODE_FRONT
// r3 = read(0x0db0);   // POLYGON_MODE_BACK
// r7 = r2 | r3;
// r4 = read(0x2100);   // SP_SELECT[4]
// r6 = 0x60;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,  (0<<12)|0x20c0/4, IMMED0,   ALU0,
               STATE,   R2, IMMED,  ZERO,          0x0dac/4,   NONE,   NONE),
   MME_INSN(0, STATE,   R3, IMMED,  ZERO,          0x0db0/4,   NONE,   NONE,
                  OR,   R7,    R2,    R3,                 0,   NONE,   NONE),
   MME_INSN(0, STATE,   R4, IMMED,  ZERO,          0x2100/4,   NONE,   NONE,
                 ADD,   R6, IMMED,  ZERO,              0x60,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0x200;
   MME_INSN(0,   ADD,   R6, IMMED,  ZERO,             0x200,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// r7 = r1 | r4;
// r7 = r7 & 1;
// if (r7 != 0)
   MME_INSN(0,    OR,   R7,    R1,    R4,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   AND,   R7,    R7, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R7,  ZERO,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = 0;
   MME_INSN(0,   ADD,   R6,  ZERO,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x02ec, 0);
// send(r6);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,  (0<<12)|0x02ec/4, IMMED0,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_draw_arrays_indirect[] = {
// r1 = load();         // mode
// r5 = read(0x1438);   // VB_INSTANCE_BASE
// r6 = load();         // start_drawid
// r7 = load();         // numparams
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                0,   NONE,   NONE,
                 ADD,   R6, LOAD1,  ZERO,                0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R7, LOAD0,  ZERO,                0,   NONE,   NONE,
               STATE,   R5, IMMED,  ZERO,         0x1438/4,   NONE,   NONE),
// while (HW_LOOP_COUNT < r7) {
//    r2 = load();      // count
//    r3 = load();      // instance_count
//    mthd(0x0d74, 0);  // VERTEX_BUFFER_FIRST
//    send(load());     // start
//    r4 = load();      // start_instance
//    if (r3) {
   MME_INSN(0,  LOOP, ZERO,    R7,  ZERO,            0x000c,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R2, LOAD0,  ZERO,          0x0d74/4, IMMED0,   NONE,
                 ADD,   R3, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   ALU0,
                 ADD,   R4, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R3,  ZERO,    (2<<14)|0x0008,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//       mthd(0x238c, 1);     // CB_POS
//       send(256 + 160);
//       send(0);             // base_vertex
//       send(r4);            // start_instance
//       send(r6);            // draw id
//       mthd(0x1438, 0);     // VB_INSTANCE_BASE
//       send(r4);
//       r1 = r1 & ~(1<<26);  // clear INSTANCE_NEXT
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,  (1<<12)|0x238c/4, IMMED0, IMMED1,
                 ADD, ZERO,  ZERO,  ZERO,         256 + 160,   NONE,   ALU0),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,          0x1438/4, IMMED0,   ALU0,
               MERGE,   R1,    R1,  ZERO, (26<<10)|(1<<5)|0,   NONE,   NONE),
//       do {
//          mthd(0x1618, 0);  // VERTEX_BEGIN_GL
//          send(r1);         // mode
//          mthd(0x0d78, 0);  // VERTEX_BUFFER_COUNT
//          send(r2);         // count
//          mthd(0x1614, 0);  // VERTEX_END_GL
//          send(0);
//          r1 |= (1<<26);    // set INSTANCE_NEXT
//       } while(--r3);
//    }
   MME_INSN(0,   ADD, ZERO,    R1,  ZERO,          0x1618/4, IMMED0,   ALU0,
                 ADD, ZERO,    R2,  ZERO,          0x0d78/4, IMMED1,   ALU1),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,          0x1614/4, IMMED0,   ALU0,
                 ADD,   R4, IMMED,  ZERO,                 1,   NONE,   NONE),
   MME_INSN(0, MERGE,   R1,    R1,    R4, (26<<10)|(1<<5)|0,   NONE,   NONE,
                 SUB,   R3,    R3, IMMED,                 1,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R3,  ZERO,    (1<<14)|0x3ffd,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r6 = r6 + 1;
// };
   MME_INSN(0,   ADD,   R6,    R6, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x1438, 0);  // restore VB_INSTANCE_BASE
// send(r5);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,          0x1438/4, IMMED0,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R5,  ZERO,                 0,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
};

const uint32_t mmec597_draw_elts_indirect[] = {
// r1 = load();         // mode
// r8 = read(0x1434);   // VB_ELEMENT_BASE
// r9 = read(0x1438);   // VB_INSTANCE_BASE
// r6 = load();         // start_drawid
// r7 = load();         // numparams
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                 0,   NONE,   NONE,
               STATE,   R8, IMMED,  ZERO,          0x1434/4,   NONE,   NONE),
   MME_INSN(0, STATE,   R9, IMMED,  ZERO,          0x1438/4,   NONE,   NONE,
                 ADD,   R6, LOAD0,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R7, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// while (HW_LOOP_COUNT < r7) {
//    r3 = load();      // count
//    r2 = load();      // instance_count
//    mthd(0x17dc, 0);  // INDEX_BATCH_FIRST
//    send(load());     // start
//    r4 = load();      // index_bias
//    mthd(0x238c, 1);  // CB_POS
//    send(256 + 160);
//    send(r4);         // index_bias
//    r5 = load();      // start_instance
//    if (r2) {
   MME_INSN(0,  LOOP, ZERO,    R7,  ZERO,            0x000d,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R3, LOAD0,  ZERO,          0x17dc/4, IMMED0,   NONE,
                 ADD,   R2, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   ALU0,
                 ADD,   R4, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,  (1<<12)|0x238c/4, IMMED0, IMMED1,
                 ADD, ZERO,    R4,  ZERO,         256 + 160,   NONE,   ALU1),
   MME_INSN(0,   BEQ, ZERO,    R2,  ZERO,    (2<<14)|0x0008,   NONE,   NONE,
                 ADD,   R5, LOAD0,  ZERO,                 0,   NONE,   NONE),
//       send(r5);         // start_instance
//       send(r6);         // draw_id
//       mthd(0x1434, 1);  // VB_ELEMENT_BASE
//       send(r4);         // index_bias
//       send(r5);         // start_instance
//       mthd(0x1118, 0);  // VERTEX_ID_BASE
//       send(r4);         // index_bias
//       r1 &= ~(1 << 26); // clear INSTANCE_NEXT
   MME_INSN(0,   ADD, ZERO,    R5,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,  (1<<12)|0x1434/4, IMMED0,   ALU0,
                 ADD, ZERO,    R5,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,          0x1118/4, IMMED0,   ALU0,
               MERGE,   R1,    R1,  ZERO, (26<<10)|(1<<5)|0,   NONE,   NONE),
//       do {
//          mthd(0x1618, 0);  // VERTEX_BEGIN_GL
//          send(r1);         // mode
//          mthd(0x17e0, 0);  // INDEX_BATCH_COUNT
//          send(r3);         // count
//          mthd(0x1614, 0);  // VERTEX_END_GL
//          send(0);
//          r1 |= (1 << 26);  // set INSTANCE_NEXT
//       } while (--r2);
//    }
   MME_INSN(0,   ADD, ZERO,    R1,  ZERO,          0x1618/4, IMMED0,   ALU0,
                 ADD, ZERO,    R3,  ZERO,          0x17e0/4, IMMED1,   ALU1),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,          0x1614/4, IMMED0,   ALU0,
                 ADD,   R4, IMMED,  ZERO,                 1,   NONE,   NONE),
   MME_INSN(0, MERGE,   R1,    R1,    R4, (26<<10)|(1<<5)|0,   NONE,   NONE,
                 SUB,   R2,    R2, IMMED,                 1,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R2,  ZERO,    (1<<14)|0x3ffd,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//   r6 = r6 + 1;
// };
   MME_INSN(0,   ADD,   R6,    R6, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x1434, 1);
// send(r8);         // restore VB_ELEMENT_BASE
// send(r9);         // restore VB_INSTANCE_BASE
// mthd(0x1118, 0);
// send(r8);         // restore VERTEX_ID_BASE
   MME_INSN(1,   ADD, ZERO,    R8,  ZERO,  (1<<12)|0x1434/4, IMMED0,   ALU0,
                 ADD, ZERO,    R9,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R8,  ZERO,          0x1118/4, IMMED0,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_draw_arrays_indirect_count[] = {
// r1 = load();         // mode
// r6 = load();         // start_drawid
// r7 = load();         // numparams
// r5 = load();         // totaldraws
// r8 = read(0x1438);   // VB_INSTANCE_BASE
// r5 = r5 - r6;        // remaining draws
// if (r5 > r7)
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD,   R6, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R7, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD,   R5, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0, STATE,   R8, IMMED,  ZERO,          0x1438/4,   NONE,   NONE,
                 SUB,   R5,    R5,    R6,                 0,   NONE,   NONE),
   MME_INSN(0,   BLE, ZERO,    R5,    R7,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r5 = r7;
   MME_INSN(0,   ADD,   R5,    R7,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// if (r5 >= 0) {
   MME_INSN(0,   BLT, ZERO,    R5,  ZERO,    (2<<14)|0x000e,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    while (HW_LOOP_COUNT < r5) {
//       r2 = load();      // count
//       r3 = load();      // instance_count
//       mthd(0x0d74, 0);  // VERTEX_BUFFER_FIRST
//       send(load());     // start
//       r4 = load();      // start_instance
//       if (r3) {
   MME_INSN(0,  LOOP, ZERO,    R5,  ZERO,            0x000c,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R2, LOAD0,  ZERO,          0x0d74/4, IMMED0,   NONE,
                 ADD,   R3, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   ALU0,
                 ADD,   R4, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R3,  ZERO,    (2<<14)|0x0008,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//          mthd(0x238c, 1);  // CB_POS
//          send(256 + 160);
//          send(0);          // base_vertex
//          send(r4);         // start_instance
//          send(r6);         // draw_id
//          mthd(0x1438, 0);  // VB_INSTANCE_BASE
//          send(r4);
//          r1 &= ~(1 << 26); // clear INSTANCE_NEXT
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,  (1<<12)|0x238c/4, IMMED0, IMMED1,
                 ADD, ZERO,  ZERO,  ZERO,           256+160,   NONE,   ALU0),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,          0x1438/4, IMMED0,   ALU0,
               MERGE,   R1,    R1,  ZERO, (26<<10)|(1<<5)|0,   NONE,   NONE),
//          do {
//             mthd(0x1618, 0);  // VERTEX_BEGIN_GL
//             send(r1);         // mode
//             mthd(0x0d78, 0);  // VERTEX_BUFFER_COUNT
//             send(r2);
//             mthd(0x1614, 0);  // VERTEX_END_GL
//             send(0);
//             r1 |= (1 << 26);  // set INSTANCE_NEXT
//          } while (--r3);
//       }
   MME_INSN(0,   ADD, ZERO,    R1,  ZERO,          0x1618/4, IMMED0,   ALU0,
                 ADD, ZERO,    R2,  ZERO,          0x0d78/4, IMMED1,   ALU1),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,          0x1614/4, IMMED0,   ALU0,
                 ADD,   R4, IMMED,  ZERO,                 1,   NONE,   NONE),
   MME_INSN(0, MERGE,   R1,    R1,    R4, (26<<10)|(1<<5)|0,   NONE,   NONE,
                 SUB,   R3,    R3, IMMED,                 1,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R3,  ZERO,    (1<<14)|0x3ffd,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//       r6 = r6 + 1;   // draw_id++
//    }
   MME_INSN(0,   ADD,   R6,    R6, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r7 = r7 - r5;  // unneeded params
// }
   MME_INSN(0,   SUB,   R7,    R7,    R5,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// while (HW_LOOP_COUNT < r7) {
//    load();
//    load();
//    load();
//    load();
// }
   MME_INSN(0,  LOOP, ZERO,    R7,  ZERO,            0x0003,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO, LOAD1,  ZERO,                 0,   NONE,   NONE),
// exit mthd(0x1438, 0);   // VB_INSTANCE_BASE
// send(r8);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,          0x1438/4, IMMED0,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO,    R8,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_draw_elts_indirect_count[] = {
// r8 = read(0x1434);
// r1 = load();
// r9 = read(0x1438);
// r6 = load();
// r7 = load();
// r5 = load();
// r5 = r5 - r6;
// if (r5 > r7)
   MME_INSN(0, STATE,   R8, IMMED,  ZERO,          0x1434/4,   NONE,   NONE,
                 ADD,   R1, LOAD0,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0, STATE,   R9, IMMED,  ZERO,          0x1438/4,   NONE,   NONE,
                 ADD,   R6, LOAD0,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R7, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD,   R5, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   SUB,   R5,    R5,    R6,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BLE, ZERO,    R5,    R7,    (2<<14)|0x0002,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r5 = r7;
   MME_INSN(0,   ADD,   R5,    R7,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// if (r5 >= 0) {
   MME_INSN(0,   BLT, ZERO,    R5,  ZERO,    (2<<14)|0x000f,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    while (HW_LOOP_COUNT < r5) {
//       r3 = load();
//       r2 = load();
//       mthd(0x17dc, 0);
//       send(load());
//       r4 = load();
//       mthd(0x238c, 1);
//       send(256 + 160);
//       send(r4);
//       r10 = load();
//       if (r2) {
   MME_INSN(0,  LOOP, ZERO,    R5,  ZERO,            0x000d,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD,   R3, LOAD0,  ZERO,  (0<<12)|0x17dc/4, IMMED0,   NONE,
                 ADD,   R2, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,  (1<<12)|0x238c/4,   NONE,   ALU0,
                 ADD,   R4, LOAD1,  ZERO,         256 + 160, IMMED0, IMMED1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,                 0,   NONE,   ALU0,
                 ADD,  R10, LOAD0,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R2,  ZERO,    (2<<14)|0x0008,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//          send(r10);
//          send(r6);
//          mthd(0x1434, 1);
//          send(r4);
//          send(r10);
//          mthd(0x1118, 0);
//          send(r4);
//          r1 &= ~(1 << 26);
   MME_INSN(0,   ADD, ZERO,   R10,  ZERO,                 0,   NONE,   ALU0,
                 ADD, ZERO,    R6,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,  (1<<12)|0x1434/4, IMMED0,   ALU0,
                 ADD, ZERO,   R10,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R4,  ZERO,  (0<<12)|0x1118/4, IMMED0,   ALU0,
               MERGE,   R1,    R1,  ZERO, (26<<10)|(1<<5)|0,   NONE,   NONE),
//          do {
//             mthd(0x1618, 0);
//             send(r1);
//             mthd(0x17e0, 0);
//             send(r3);
//             mthd(0x1614, 0);
//             send(0);
//             r1 |= (1 << 26);
//          } while (--r2);
//       }
   MME_INSN(0,   ADD, ZERO,    R1,  ZERO,          0x1618/4, IMMED0,   ALU0,
                 ADD, ZERO,    R3,  ZERO,          0x17e0/4, IMMED1,   ALU1),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,          0x1614/4, IMMED0,   ALU0,
                 ADD,   R4, IMMED,  ZERO,                 1,   NONE,   NONE),
   MME_INSN(0, MERGE,   R1,    R1,    R4, (26<<10)|(1<<5)|0,   NONE,   NONE,
                 SUB,   R2,    R2, IMMED,                 1,   NONE,   NONE),
   MME_INSN(0,   BEQ, ZERO,    R2,  ZERO,    (1<<14)|0x3ffd,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//       r6 = r6 + 1;
//    }
   MME_INSN(0,   ADD,   R6,    R6, IMMED,                 1,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
//    r7 = r7 - r5; // unneeded params
// }
   MME_INSN(0,   SUB,   R7,    R7,    R5,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// while (HW_LOOP_COUNT < r7) {
//    r2 = load();
//    r2 = load();
//    r2 = load();
//    r2 = load();
//    r2 = load();
// }
   MME_INSN(0,  LOOP, ZERO,    R7,  ZERO,            0x0004,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO, LOAD1,  ZERO,                 0,   NONE,   NONE),
   MME_INSN(0,   ADD, ZERO, LOAD0,  ZERO,                 0,   NONE,   NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
// mthd(0x1434, 1);
// send(r8);
// send(r9);
// exit mthd(0x1118, 0);
// send(r8);
   MME_INSN(1,   ADD, ZERO,    R8,  ZERO,  (1<<12)|0x1434/4, IMMED0,   ALU0,
                 ADD, ZERO,    R9,  ZERO,                 0,   NONE,   ALU1),
   MME_INSN(0,   ADD, ZERO,    R8,  ZERO,  (0<<12)|0x1118/4, IMMED0,   ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,   NONE),
};

const uint32_t mmec597_query_buffer_write[] = {
// r1 = load();   // clamp value
// r2 = load();   // end value (lo)
// r3 = load();   // end value (hi)
// r4 = load();   // start value (lo)
// r5 = load();   // start value (hi)
// r8 = load();   // desired sequence
// r9 = load();   // actual sequence
// r7 = load();   // query address (hi)
// r6 = load();   // query address (lo)
// if (r9 >= r8) {
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD,   R2, LOAD1,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R3, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD,   R4, LOAD1,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R5, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD,   R8, LOAD1,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R9, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD,   R7, LOAD1,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R6, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   BLT, ZERO,    R9,    R8,    (2<<14)|0x000e,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
//    [r3,r2] = [r3,r2] - [r5,r4];
//    if (r1) {
   MME_INSN(0,   SUB,   R2,    R2,    R4,                 0,   NONE,      NONE,
                SUBB,   R3,    R3,    R5,                 0,   NONE,      NONE),
   MME_INSN(0,   BEQ, ZERO,    R1,  ZERO,    (2<<14)|0x0004,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
//       if (r3 != 0 || r1 < r2)
//          r2 = r1;
//    }
   MME_INSN(0,   BEQ, ZERO,    R3,  ZERO,    (1<<14)|0x0002,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,  BLTU, ZERO,    R1,    R2,    (1<<14)|0x0002,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R2,    R1,  ZERO,                 0,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
//    mthd(0x1b00, 1);
//    send(r7);
//    send(r6);
//    send(r2)
//    send(0x10000000);
//    if (!r1) {
   MME_INSN(0,   ADD, ZERO,    R7,  ZERO,  (1<<12)|0x1b00/4, IMMED0,      ALU0,
                 ADD, ZERO,    R6,  ZERO,                 0,   NONE,      ALU1),
   MME_INSN(0,   ADD, ZERO,    R2,  ZERO,                 0,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,            0x1000,   NONE, IMMED32_0,
                 ADD, ZERO,  ZERO,  ZERO,            0x0000,   NONE,      NONE),
   MME_INSN(0,   BEQ, ZERO,    R1,  ZERO,    (1<<14)|0x0004,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
//       [r7,r6] = [r7,r6] + 4;
//       mthd(0x1b00, 1);
//       send(r7);
//       send(r6);
//       send(r3);
//       send(0x10000000);
//    }
   MME_INSN(0,   ADD, ZERO,    R6, IMMED,                 4, IMMED1,      ALU1,
                ADDC, ZERO,    R7,  ZERO,  (1<<12)|0x1b00/4,   NONE,      ALU0),
   MME_INSN(0,   ADD, ZERO,    R3,  ZERO,                 0,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,            0x1000,   NONE, IMMED32_0,
                 ADD, ZERO,  ZERO,  ZERO,            0x0000,   NONE,      NONE),
//    mthd(0x0110, 0);
//    send(0);
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,  (0<<12)|0x0110/4, IMMED0,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
// }
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
};

const uint32_t mmec597_conservative_raster_state[] = {
// r1 = load();
// mthd(0x3400, 1);
// send(0);
// send(((r1 >> 8) & 7) << 23);
// send(0x03800000);
// mthd(0x2310, 1);
// send(0x00418800);
// r2 = r1 & 0xf;
// r3 = 16;
// r2 = r2 | (((r1 >> 4) & 0xf) << 8);
// mthd(0x0a1c, 8);
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,  (1<<12)|0x3400/4, IMMED0,    IMMED1,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0, MERGE, ZERO,  ZERO,    R1, (23<<10)|(3<<5)|8,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,            0x0380,   NONE, IMMED32_0,
                 ADD, ZERO,  ZERO,  ZERO,            0x0000,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,  (1<<12)|0x2310/4, IMMED0,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,            0x0000,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,            0x0041,   NONE, IMMED32_0,
                 ADD, ZERO,  ZERO,  ZERO,            0x8800,   NONE,      NONE),
   MME_INSN(0,   AND,   R2,    R1, IMMED,               0xf,   NONE,      NONE,
                 ADD,   R3,  ZERO, IMMED,                16,   NONE,      NONE),
   MME_INSN(0, MERGE,   R2,    R2,    R1,  (8<<10)|(4<<5)|4, IMMED1,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,  (8<<12)|0x0a1c/4,   NONE,      NONE),
// while (HW_LOOP_COUNT < r3)
//    send(r2);
   MME_INSN(0,  LOOP, ZERO,    R3,  ZERO,            0x0002,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,    R2,  ZERO,                 0,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
// mthd(0x1148, 0);
// send(1);
   MME_INSN(1,   ADD, ZERO,  ZERO,  ZERO,  (0<<12)|0x1148/4, IMMED0,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,                 1,   NONE,    IMMED1,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
};

const uint32_t mmec597_set_priv_reg[] = {
// r0 = load();
// mthd(WAIT_FOR_IDLE, 0);
// send(0);
// mthd(SET_MME_SHADOW_SCRATCH(0), 1);
// send(0);
   MME_INSN(0,   ADD,   R0,     LOAD0,      ZERO, (0<<12)|0x0110/4,     IMMED0, IMMEDHIGH0,
                 ADD, ZERO,      ZERO,      ZERO, (1<<12)|0x3400/4,     IMMED1, IMMEDHIGH0),
// send(load());
   MME_INSN(0,   ADD, ZERO,      ZERO,      ZERO,                0,       NONE,      LOAD0,
                 ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE),
// alu0 = r0;
// r0 = read(NVC597_SET_MME_SHADOW_SCRATCH(26));
// send(load());
// mthd(SET_FALCON04, 0);
// send(alu0);
   MME_INSN(0,   ADD, ZERO,        R0,      ZERO, (0<<12)|0x2310/4,       NONE,      LOAD0,
               STATE,   R0,     IMMED,      ZERO,         0x3468/4,     IMMED0,       ALU0),
// r0 &= 0xffff;
   MME_INSN(0, MERGE,   R0,      ZERO,        R0, (0<<10)|(8<<5)|0,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE),
// if (r0 == 2) {
   MME_INSN(0,   BEQ, ZERO,        R0, IMMEDPAIR,   (2<<14)|0x0004,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,                2,       NONE,       NONE),
//    do {
//       r0 = read(NVC597_SET_MME_SHADOW_SCRATCH(0));
//       mthd(NO_OPERATION);
//       send(0);
   MME_INSN(0, STATE,   R0,     IMMED,      ZERO,         0x3400/4,     IMMED1, IMMEDHIGH1,
                 ADD, ZERO,      ZERO,      ZERO,         0x0100/4,       NONE,       NONE),
//    } while(r0 != 1);
   MME_INSN(1,   BEQ, ZERO,        R0, IMMEDPAIR,   (1<<14)|0x1fff,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,                1,       NONE,       NONE),
// } else {
   MME_INSN(0,   JAL, ZERO,      ZERO,      ZERO,   (1<<15)|0x0003,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE),
//    while (HW_LOOP_COUNT < 10) {
   MME_INSN(0,  LOOP, ZERO, IMMEDPAIR,      ZERO,                2,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,               10,       NONE,       NONE),
//       mthd(NO_OPERATION, 0);
//       send(0);
//    }
// }
   MME_INSN(0,   ADD, ZERO,      ZERO,      ZERO, (0<<12)|0x0100/4,     IMMED0,     IMMED1,
                 ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE),
// nop
   MME_INSN(1,   ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE),
// nop
   MME_INSN(0,   ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE,
                 ADD, ZERO,      ZERO,      ZERO,                0,       NONE,       NONE),
};

const uint32_t mmec597_compute_counter[] = {
// r0 = load();
// r1 = 1;
// r2 = 0;
// while (HW_LOOP_COUNT < r2) {
   MME_INSN(0,   ADD,   R0, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD,   R1, IMMED,  ZERO,                 1,   NONE,      NONE),
   MME_INSN(0,  LOOP, ZERO,    R0,  ZERO,            0x0003,   NONE,      NONE,
                 ADD,   R2,  ZERO,  ZERO,                 0,   NONE,      NONE),
//    r3 = load();
//    [r1,r0] *= r3;
// }
   MME_INSN(0,   ADD,   R3, LOAD0,  ZERO,                 0,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,  MULU,   R1,    R1,    R3,                 0,   NONE,      NONE,
                MULH,   R2,  ZERO,  ZERO,                 0,   NONE,      NONE),
// r3 = read(0x3410);
// r4 = read(0x3414);
// [r4,r3] += [r2,r1];
// mthd(0x3410, 1);
// send(r3);
// send(r4);
   MME_INSN(0, STATE, ZERO,  ZERO,  ZERO,          0x3410/4,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(1, STATE, ZERO,  ZERO,  ZERO,          0x3414/4,   NONE,      NONE,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R3,    R3,    R1,  (1<<12)|0x3410/4, IMMED0,      ALU0,
                ADDC,   R4,    R4,    R2,                 0,   NONE,      ALU1),
};

const uint32_t mmec597_compute_counter_to_query[] = {
// r1 = load();
// r3 = read(0x3410);
// r2 = load();
// r4 = read(0x3414);
// [r2,r1] = [r2,r1] + [r4,r3];
// mthd(0x1b00, 1);
// r3 = load();
// send(r3);
// r4 = load();
// send(r4);
// send(r1);
// send(0x10000000);
   MME_INSN(0,   ADD,   R1, LOAD0,  ZERO,                 0,   NONE,      NONE,
               STATE,   R3, IMMED,  ZERO,          0x3410/4,   NONE,      NONE),
   MME_INSN(0,   ADD,   R2, LOAD0,  ZERO,                 0,   NONE,      NONE,
               STATE,   R4, IMMED,  ZERO,          0x3414/4,   NONE,      NONE),
   MME_INSN(0,   ADD,   R1,    R1,    R3,  (1<<12)|0x1b00/4, IMMED0,      NONE,
                ADDC,   R2,    R2,    R4,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD,   R3, LOAD0,  ZERO,                 0,   NONE,      ALU0,
                 ADD,   R4, LOAD1,  ZERO,                 0,   NONE,      ALU1),
   MME_INSN(0,   ADD, ZERO,    R1,  ZERO,                 0,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,            0x1000,   NONE, IMMED32_0,
                 ADD, ZERO,  ZERO,  ZERO,            0x0000,   NONE,      NONE),
// [r3,r4] = [r3,r4] + 4;
// mthd(0x1b00, 1);
// send(r3);
// send(r4);
// send(r2);
// send(0x10000000);
   MME_INSN(0,   ADD, ZERO,    R4, IMMED,                 4, IMMED1,      ALU1,
                ADDC, ZERO,    R3,  ZERO,  (1<<12)|0x1b00/4,   NONE,      ALU0),
   MME_INSN(1,   ADD, ZERO,    R2,  ZERO,                 0,   NONE,      ALU0,
                 ADD, ZERO,  ZERO,  ZERO,                 0,   NONE,      NONE),
   MME_INSN(0,   ADD, ZERO,  ZERO,  ZERO,            0x1000,   NONE, IMMED32_0,
                 ADD, ZERO,  ZERO,  ZERO,            0x0000,   NONE,      NONE),
};
