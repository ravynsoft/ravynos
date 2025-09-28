#as: -k
#objdump: -sr
#name: VAX ELF relocations

# Test VAX ELF relocations

.*:     file format elf32-vax

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
00000015 R_VAX_PLT32       text_vax_plt32
00000028 R_VAX_GOT32       text_vax_got32
0000003b R_VAX_GOT32       text_vax_got32\+0x00000020
00000005 R_VAX_PC8         text_vax_pc8
00000009 R_VAX_PC16        text_vax_pc16
0000000e R_VAX_PC32        text_vax_pc32
0000001b R_VAX_PC8         text_vax_pc8
0000001e R_VAX_PC16        text_vax_pc16
00000022 R_VAX_PC32        text_vax_pc32
0000002e R_VAX_PC8         text_vax_pc8\+0x00000008
00000031 R_VAX_PC16        text_vax_pc16\+0x00000010
00000035 R_VAX_PC32        text_vax_pc32\+0x00000020


RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
00000000 R_VAX_8           data_vax_8
00000001 R_VAX_16          data_vax_16
00000003 R_VAX_32          data_vax_32
00000007 R_VAX_8           data_vax_8\+0x00000008
00000008 R_VAX_16          data_vax_16\+0x00000010
0000000a R_VAX_32          data_vax_32\+0x00000020


Contents of section \.text:
 0000 0000fb00 af00fb00 cf0000fb 00ef0000  .*
 0010 0000fb00 ef000000 00d5af00 d5cf0000  .*
 0020 d5ef0000 0000d5ef 00000000 d5af00d5  .*
 0030 cf0000d5 ef000000 00d5ef00 00000004  .*
Contents of section \.data:
 0000 00000000 00000000 00000000 0000      .*
#pass
