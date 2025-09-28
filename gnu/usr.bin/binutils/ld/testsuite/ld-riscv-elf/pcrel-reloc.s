.text
.global _start
_start:
    auipc t0, %pcrel_hi(sym)
    lw t0, %pcrel_lo(_start)(t0)
