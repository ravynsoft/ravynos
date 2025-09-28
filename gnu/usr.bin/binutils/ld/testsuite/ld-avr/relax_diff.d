#name: AVR Account for relaxation in label differences
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: relax.s
#objdump: -s
#target: avr-*-*

.*:     file format elf32-avr

Contents of section .text:
 0000 ffcf                                 .*
Contents of section .data:
 802000 0200                               .*

