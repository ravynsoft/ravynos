LD_FEATURE("SANE_EXPR")
e1 = 0x100;
e2 = 0x80;
e3 = e1 * e2;
SECTIONS
{
  . = e3;
  .data :
  {
    d1 = 4;
    . += d1 + 5 << 2;
    d2 = .;
    s_diff = d2 - d1;
    s_sum_neg = d2 + -d1;
    s_sum = d2 + d1;
    s_prod = d2 * d1;
    s1 = d1 - 2;
    s2 = d2 % 5;
    s3 = d2 / 5;
    s4 = ABSOLUTE (d1) - 2;
    s5 = ABSOLUTE (d2) % 5;
    s6 = ABSOLUTE (d2) / 5;
    *(.data .rw)
  }
  .text : { *(.text) }
  .bss : { *(.bss) }
  /DISCARD/ : {*(*)}

diff = d2 - d1;
sum_neg = d2 + -d1;
sum = d2 + d1;
prod = d2 * d1;
x1 = d1 - 2;
x2 = d2 % 5;
x3 = d2 / 5;
x4 = ABSOLUTE (d1) - 2;
x5 = ABSOLUTE (d2) % 5;
x6 = ABSOLUTE (d2) / 5;
}
