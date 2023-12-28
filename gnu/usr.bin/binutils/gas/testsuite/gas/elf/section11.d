#as: --no-pad-sections
#readelf: -S --wide
#name: Disabling section padding
# The RX port uses non standard section names.
#xfail: loongarch*-* rx-*-*
# LoongArch and RISC-V handles alignment via linker relaxation, so object files don't have
# the expected alignment.
#xfail: riscv*-*-*

#...
  \[ .\] .text[ 	]+PROGBITS[ 	]+0+00 0+[0-9a-f]+ 0+0(1|4|5) 00  AX  0   0 16
#...
  \[ .\] .data[ 	]+PROGBITS[ 	]+0+00 0+[0-9a-f]+ 0+01 00  WA  0   0 16
#...
  \[ .\] .bss[ 	]+NOBITS[ 	]+0+00 0+[0-9a-f]+ 0+01 00  WA  0   0 16
#pass
