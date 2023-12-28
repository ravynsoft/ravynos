#objdump: -rs
#name: i386 secidx reloc

.*: +file format pe-i386

RELOCATION RECORDS FOR \[\.data\]:
OFFSET[ 	]+TYPE[ 	]+VALUE
0+24 secidx            \.text
0+27 secidx            \.text
0+2a secidx            \.text
0+2d secidx            \.text
0+3c secidx            \.data
0+3f secidx            \.data
0+42 secidx            \.data
0+45 secidx            \.data
0+54 secidx            \.rdata
0+57 secidx            \.rdata
0+5a secidx            \.rdata
0+5d secidx            \.rdata
0+6c secidx            ext24
0+6f secidx            ext2d
0+72 secidx            ext36
0+75 secidx            ext3f

Contents of section \.text:
 0000 3e3e3e3e 3c3c3c3c 3e3e3e3e 3e3c3c3c  >>>><<<<>>>>><<<
 0010 3e3e3e3e 3e3e3c3c 3e3e3e3e 3e3e3e3c  >>>>>><<>>>>>>><
Contents of section \.data:
 0000 3e3e3e3e 3c3c3c3c 3e3e3e3e 3e3c3c3c  >>>><<<<>>>>><<<
 0010 3e3e3e3e 3e3e3c3c 3e3e3e3e 3e3e3e3c  >>>>>><<>>>>>>><
 0020 3e3e3e3e 00001100 00110000 11000011  >>>>............
 0030 3c3c3c3c 3c3c3c3c 3e3e3e3e 00001100  <<<<<<<<>>>>....
 0040 00110000 11000011 3c3c3c3c 3c3c3c3c  ........<<<<<<<<
 0050 3e3e3e3e 00001100 00110000 11000011  >>>>............
 0060 3c3c3c3c 3c3c3c3c 3e3e3e3e 00001100  <<<<<<<<>>>>....
 0070 00110000 11000011 3c3c3c3c 3c3c3c3c  ........<<<<<<<<
Contents of section \.rdata:
 0000 3e3e3e3e 3c3c3c3c 3e3e3e3e 3e3c3c3c  >>>><<<<>>>>><<<
 0010 3e3e3e3e 3e3e3c3c 3e3e3e3e 3e3e3e3c  >>>>>><<>>>>>>><
 0020 3e3e3e3e 00000000 00000000 00000000  >>>>............
