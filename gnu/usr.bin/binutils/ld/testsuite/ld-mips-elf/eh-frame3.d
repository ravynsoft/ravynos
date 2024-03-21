#name: MIPS eh-frame 3
#source: eh-frame1.s
#source: eh-frame1.s
#as: -EB -mips3 -mabi=eabi --defsym alignment=3 --defsym fill=0
#readelf: -wf
#ld: -EB -Teh-frame1.ld --defsym foo=0x50607080
#
# This test is for the official LP64 version of EABI64, which uses a
# combination of 32-bit objects and 64-bit FDE addresses.
#

Contents of the \.eh_frame section:

0+0000 0+000c 0+0000 CIE
  Version:               1
  Augmentation:          ""
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0010 0+0014 0+0014 FDE cie=0+0000 pc=0+020000..0+020010

0+0028 0+0014 0+002c FDE cie=0+0000 pc=0+020010..0+020030

# basic2 removed
0+0040 0+0014 0+0044 FDE cie=0+0000 pc=0+020030..0+020060

# basic3 removed
0+0058 0+0014 0+005c FDE cie=0+0000 pc=0+020060..0+0200a0

# basic4 removed
0+0070 0+0014 0+0074 FDE cie=0+0000 pc=0+0200a0..0+0200f0

0+0088 0+001c 0+0000 CIE
  Version:               1
  Augmentation:          "zP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     00 00 00 00 00 50 60 70 80

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00a8 0+001c 0+0024 FDE cie=0+0088 pc=0+0200f0..0+020100
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00c8 0+001c 0+0044 FDE cie=0+0088 pc=0+020100..0+020120
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00e8 0+001c 0+0000 CIE
  Version:               1
  Augmentation:          "zP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     50 00 00 00 00 00 00 00 00 00 00 00 50 60 70 80


0+0108 0+001c 0+0024 FDE cie=0+00e8 pc=0+020120..0+020130
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0128 0+0018 0+0044 FDE cie=0+00e8 pc=0+020130..0+020150
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0144 0+0018 0+0000 CIE
  Version:               1
  Augmentation:          "zPR"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     00 00 00 00 00 50 60 70 80 00

  DW_CFA_nop

0+0160 0+001c 0+0020 FDE cie=0+0144 pc=0+020150..0+020160
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# FDE for .discard removed
# zPR2 removed
0+0180 0+001c 0+0040 FDE cie=0+0144 pc=0+020160..0+020190
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01a0 0+001c 0+0060 FDE cie=0+0144 pc=0+020190..0+0201d0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01c0 0+0014 0+01c4 FDE cie=0+0000 pc=0+0201d0..0+0201e0

# basic1 removed, followed by repeat of above
0+01d8 0+0014 0+01dc FDE cie=0+0000 pc=0+0201e0..0+0201f0

0+01f0 0+0014 0+01f4 FDE cie=0+0000 pc=0+0201f0..0+020210

0+0208 0+0014 0+020c FDE cie=0+0000 pc=0+020210..0+020240

0+0220 0+0014 0+0224 FDE cie=0+0000 pc=0+020240..0+020280

0+0238 0+0014 0+023c FDE cie=0+0000 pc=0+020280..0+0202d0

0+0250 0+001c 0+01cc FDE cie=0+0088 pc=0+0202d0..0+0202e0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0270 0+001c 0+01ec FDE cie=0+0088 pc=0+0202e0..0+020300
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0290 0+001c 0+01ac FDE cie=0+00e8 pc=0+020300..0+020310
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+02b0 0+001c 0+01cc FDE cie=0+00e8 pc=0+020310..0+020330
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+02d0 0+001c 0+0190 FDE cie=0+0144 pc=0+020330..0+020340
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# FDE for .discard removed
# zPR2 removed
0+02f0 0+001c 0+01b0 FDE cie=0+0144 pc=0+020340..0+020370
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0310 0+001c 0+01d0 FDE cie=0+0144 pc=0+020370..0+0203b0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0330 0+0014 0+0334 FDE cie=0+0000 pc=0+0203b0..0+0203c0
