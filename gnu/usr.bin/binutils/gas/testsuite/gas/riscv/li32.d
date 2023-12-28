#as: -march=rv32ic -mabi=ilp32
#objdump: -dr

.*:     file format elf32-(little|big)riscv


Disassembly of section .text:

0+000 <target>:
[^:]+:[ 	]+6521[ 	]+lui[ 	]+a0,0x8
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1 # .*
[^:]+:[ 	]+6509[ 	]+lui[ 	]+a0,0x2
[^:]+:[ 	]+f0150513[ 	]+add[ 	]+a0,a0,-255 # .*
[^:]+:[ 	]+12345537[ 	]+lui[ 	]+a0,0x12345
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1 # .*
[^:]+:[ 	]+f2345537[ 	]+lui[ 	]+a0,0xf2345
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1 # .*
