#ifndef S12Z_H
#define S12Z_H

/* This byte is used to prefix instructions in "page 2" of the opcode
   space.  */
#define PAGE2_PREBYTE (0x1b)

struct reg
{
  char      *name;   /* The canonical name of the register.  */
  int       bytes;   /* its size, in bytes.  */
};


/* How many registers do we have.  Actually there are only 13,
   because CCL and CCH are the low and high bytes of CCW.  But
   for assemnbly / disassembly purposes they are considered
   distinct registers.  */
#define S12Z_N_REGISTERS 15

extern const struct reg registers[S12Z_N_REGISTERS];

/* Solaris defines REG_Y in sys/regset.h; undef it here to avoid
   breaking compilation when this target is enabled.  */
#undef REG_Y

enum
  {
    REG_D2 = 0,
    REG_D3,
    REG_D4,
    REG_D5,
    REG_D0,
    REG_D1,
    REG_D6,
    REG_D7,
    REG_X,
    REG_Y,
    REG_S,
    REG_P,
    REG_CCH,
    REG_CCL,
    REG_CCW
  };

/* Any of the registers d0, d1, ... d7.  */
#define REG_BIT_Dn \
((0x1U << REG_D2) | \
 (0x1U << REG_D3) | \
 (0x1U << REG_D4) | \
 (0x1U << REG_D5) | \
 (0x1U << REG_D6) | \
 (0x1U << REG_D7) | \
 (0x1U << REG_D0) | \
 (0x1U << REG_D1))

/* Any of the registers x, y or z.  */
#define REG_BIT_XYS \
((0x1U << REG_X) | \
 (0x1U << REG_Y) | \
 (0x1U << REG_S))

/* Any of the registers x, y, z or p.  */
#define REG_BIT_XYSP \
((0x1U << REG_X)  | \
 (0x1U << REG_Y)  | \
 (0x1U << REG_S)  | \
 (0x1U << REG_P))

/* The x register or the y register.  */
#define REG_BIT_XY \
((0x1U << REG_X) | \
 (0x1U << REG_Y))

#endif
