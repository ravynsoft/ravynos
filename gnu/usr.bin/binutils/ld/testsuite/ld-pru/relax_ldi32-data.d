#name: PRU LDI32 relaxation data
#source: relax_ldi32.s
#source: relax_ldi32_symbol.s
#as: --mlink-relax
#ld: --relax
#objdump: -s

# Note: default linker script should put a guard at DRAM address 0

.*: +file format elf32-pru

Contents of section .text:
 [0-9a-f]+ d0adde24 90efbe24 0d00f630 d0adde24  .*
 [0-9a-f]+ 90efbe24 f0cace24 e0cace24 d0010024  .*
 [0-9a-f]+ 90cace24 d0acde24 90efbe24 d0341224  .*
 [0-9a-f]+ 90785624 f0785624 f0100024 f300007e  .*
Contents of section .data:
 0000 00000000 40000020 38000000 10003800  .*
 0010 0e000000 f2ffffff 0e0038aa           .*
