#name: PRU LDI32 disabled-relaxation data
#source: relax_ldi32.s
#source: relax_ldi32_symbol.s
#as: --mlink-relax
#ld: --no-relax
#objdump: -s

# Note: default linker script should put a guard at DRAM address 0

.*: +file format elf32-pru

Contents of section .text:
 [0-9a-f]+ d0adde24 90efbe24 0f00f630 d0adde24  .*
 [0-9a-f]+ 90efbe24 d0000024 90cace24 e0cace24  .*
 [0-9a-f]+ d0010024 90cace24 d0acde24 90efbe24  .*
 [0-9a-f]+ d0341224 90785624 d0000024 90785624  .*
 [0-9a-f]+ f0120024 f100007e .*
Contents of section .data:
 0000 00000000 48000020 40000000 12004000  .*
 0010 10000000 f0ffffff 100040aa           .*
