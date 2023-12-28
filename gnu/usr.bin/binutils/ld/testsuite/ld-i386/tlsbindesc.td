#source: tlsbindesc.s
#source: tlsbin.s
#as: --32
#ld: -melf_i386 tmpdir/libtlslib.so --no-ld-generated-unwind-info
#objdump: -sj.tdata
#target: i?86-*-*

.*:     file format elf32-i386.*

Contents of section \.tdata:
 [0-9a-f]+000 11000000 12000000 13000000 14000000  .*
 [0-9a-f]+010 15000000 16000000 17000000 18000000  .*
 [0-9a-f]+020 41000000 42000000 43000000 44000000  .*
 [0-9a-f]+030 45000000 46000000 47000000 48000000  .*
 [0-9a-f]+040 01010000 02010000 03010000 04010000  .*
 [0-9a-f]+050 05010000 06010000 07010000 08010000  .*
