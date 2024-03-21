#name: MIPS eh-frame 2, n64
#source: eh-frame1.s
#source: eh-frame1.s
#as: --defsym alignment=3 --defsym fill=0
#readelf: --relocs -wf
#ld: -shared --eh-frame-hdr -Teh-frame1.ld
#warning: FDE encoding in.*prevents \.eh_frame_hdr table being created.

Relocation section '\.rel\.dyn' .*:
 *Offset .*
0+00000  [0-9a-f]+ R_MIPS_NONE *
 *Type2: R_MIPS_NONE *
 *Type3: R_MIPS_NONE *
# Initial PCs for the FDEs attached to CIE 0x118
0+00030138  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030158  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+000302f0  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030310  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+000300c7  [0-9a-f]+ R_MIPS_REL32      0+000 foo
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030128  [0-9a-f]+ R_MIPS_REL32      0+000 foo
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+0003017e  [0-9a-f]+ R_MIPS_REL32      0+000 foo
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
Contents of the \.eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     1c

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+001c 0+001c FDE cie=0+0000 pc=0+020000..0+020010
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0038 0+001c 0+003c FDE cie=0+0000 pc=0+020010..0+020030
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic2 removed
0+0058 0+001c 0+005c FDE cie=0+0000 pc=0+020030..0+020060
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic3 removed
0+0078 0+001c 0+007c FDE cie=0+0000 pc=0+020060..0+0200a0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic4 removed
0+0098 0+0018 0+009c FDE cie=0+0000 pc=0+0200a0..0+0200f0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00b4 0+0018 0+0000 CIE
  Version:               1
  Augmentation:          "zRP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     1c 00 00 00 00 00 00 00 00 00

  DW_CFA_nop

0+00d0 0+001c 0+0020 FDE cie=0+00b4 pc=0+0200f0..0+020100
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00f0 0+001c 0+0040 FDE cie=0+00b4 pc=0+020100..0+020120
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0110 0+001c 0+0000 CIE
  Version:               1
  Augmentation:          "zP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00


0+0130 0+001c 0+0024 FDE cie=0+0110 pc=0+020120..0+020130
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0150 0+0018 0+0044 FDE cie=0+0110 pc=0+020130..0+020150
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+016c 0+0018 0+0000 CIE
  Version:               1
  Augmentation:          "zPR"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     00 00 00 00 00 00 00 00 00 1c

  DW_CFA_nop

0+0188 0+001c 0+0020 FDE cie=0+016c pc=0+020150..0+020160
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# FDE for .discard removed
# zPR2 removed
0+01a8 0+001c 0+0040 FDE cie=0+016c pc=0+020160..0+020190
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01c8 0+001c 0+0060 FDE cie=0+016c pc=0+020190..0+0201d0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01e8 0+001c 0+01ec FDE cie=0+0000 pc=0+0201d0..0+0201e0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic1 removed, followed by repeat of above
0+0208 0+001c 0+020c FDE cie=0+0000 pc=0+0201e0..0+0201f0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0228 0+001c 0+022c FDE cie=0+0000 pc=0+0201f0..0+020210
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0248 0+001c 0+024c FDE cie=0+0000 pc=0+020210..0+020240
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0268 0+001c 0+026c FDE cie=0+0000 pc=0+020240..0+020280
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0288 0+001c 0+028c FDE cie=0+0000 pc=0+020280..0+0202d0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+02a8 0+001c 0+01f8 FDE cie=0+00b4 pc=0+0202d0..0+0202e0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+02c8 0+001c 0+0218 FDE cie=0+00b4 pc=0+0202e0..0+020300
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+02e8 0+001c 0+01dc FDE cie=0+0110 pc=0+020300..0+020310
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0308 0+001c 0+01fc FDE cie=0+0110 pc=0+020310..0+020330
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0328 0+001c 0+01c0 FDE cie=0+016c pc=0+020330..0+020340
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0348 0+001c 0+01e0 FDE cie=0+016c pc=0+020340..0+020370
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0368 0+001c 0+0200 FDE cie=0+016c pc=0+020370..0+0203b0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0388 0+0018 0+038c FDE cie=0+0000 pc=0+0203b0..0+0203c0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

