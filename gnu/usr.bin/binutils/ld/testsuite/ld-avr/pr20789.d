#name: AVR Account for relaxation in negative label differences
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: pr20789.s
#objdump: -s
#target: avr-*-*

.*:     file format elf32-avr

Contents of section .text:
 0000 ffcf                                 .*
Contents of section .data:
 802000 feff                               .*

