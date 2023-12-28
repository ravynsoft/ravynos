#name: MIPS eh-frame 1, n64
#source: eh-frame1.s
#source: eh-frame1.s
#as: --defsym alignment=3 --defsym fill=0x40
#readelf: --relocs -wf
#ld: -shared --eh-frame-hdr -Teh-frame1.ld
#warning: FDE encoding in.*prevents \.eh_frame_hdr table being created.

Relocation section '\.rel\.dyn' .*:
 *Offset .*
0+00+000  [0-9a-f]+ R_MIPS_NONE *
 *Type2: R_MIPS_NONE *
 *Type3: R_MIPS_NONE *
# Initial PCs for the FDEs attached to CIE 0x120
0+00030140  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030160  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030300  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030320  [0-9a-f]+ R_MIPS_REL32 *
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+000300c7  [0-9a-f]+ R_MIPS_REL32      0+00+00+00 foo
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+00030130  [0-9a-f]+ R_MIPS_REL32      0+00+00+00 foo
 *Type2: R_MIPS_64 *
 *Type3: R_MIPS_NONE *
0+0003018a  [0-9a-f]+ R_MIPS_REL32      0+00+00+00 foo
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

  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
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

0+00b4 0+0020 0+0000 CIE
  Version:               1
  Augmentation:          "zRP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     1c 00 00 00 00 00 00 00 00 00

  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_nop
  DW_CFA_nop

0+00d8 0+001c 0+0028 FDE cie=0+00b4 pc=0+0200f0..0+020100
  DW_CFA_advance_loc: 0 to 0+0200f0
  DW_CFA_advance_loc: 0 to 0+0200f0
  DW_CFA_advance_loc: 0 to 0+0200f0
  DW_CFA_advance_loc: 0 to 0+0200f0
  DW_CFA_advance_loc: 0 to 0+0200f0
  DW_CFA_advance_loc: 0 to 0+0200f0
  DW_CFA_advance_loc: 0 to 0+0200f0

0+00f8 0+001c 0+0048 FDE cie=0+00b4 pc=0+020100..0+020120
  DW_CFA_advance_loc: 0 to 0+020100
  DW_CFA_advance_loc: 0 to 0+020100
  DW_CFA_advance_loc: 0 to 0+020100
  DW_CFA_advance_loc: 0 to 0+020100
  DW_CFA_advance_loc: 0 to 0+020100
  DW_CFA_advance_loc: 0 to 0+020100
  DW_CFA_advance_loc: 0 to 0+020100

0+0118 0+001c 0+0000 CIE
  Version:               1
  Augmentation:          "zP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00


0+0138 0+001c 0+0024 FDE cie=0+0118 pc=0+020120..0+020130
  DW_CFA_advance_loc: 0 to 0+020120
  DW_CFA_advance_loc: 0 to 0+020120
  DW_CFA_advance_loc: 0 to 0+020120
  DW_CFA_advance_loc: 0 to 0+020120
  DW_CFA_advance_loc: 0 to 0+020120
  DW_CFA_advance_loc: 0 to 0+020120
  DW_CFA_advance_loc: 0 to 0+020120

0+0158 0+001c 0+0044 FDE cie=0+0118 pc=0+020130..0+020150
  DW_CFA_advance_loc: 0 to 0+020130
  DW_CFA_advance_loc: 0 to 0+020130
  DW_CFA_advance_loc: 0 to 0+020130
  DW_CFA_advance_loc: 0 to 0+020130
  DW_CFA_advance_loc: 0 to 0+020130
  DW_CFA_advance_loc: 0 to 0+020130
  DW_CFA_advance_loc: 0 to 0+020130

0+0178 0+001c 0+0000 CIE
  Version:               1
  Augmentation:          "zPR"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     00 00 00 00 00 00 00 00 00 1c

  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000
  DW_CFA_advance_loc: 0 to 0+0000

0+0198 0+001c 0+0024 FDE cie=0+0178 pc=0+020150..0+020160
  DW_CFA_advance_loc: 0 to 0+020150
  DW_CFA_advance_loc: 0 to 0+020150
  DW_CFA_advance_loc: 0 to 0+020150
  DW_CFA_advance_loc: 0 to 0+020150
  DW_CFA_advance_loc: 0 to 0+020150
  DW_CFA_advance_loc: 0 to 0+020150
  DW_CFA_advance_loc: 0 to 0+020150

# FDE for .discard removed
# zPR2 removed
0+01b8 0+001c 0+0044 FDE cie=0+0178 pc=0+020160..0+020190
  DW_CFA_advance_loc: 0 to 0+020160
  DW_CFA_advance_loc: 0 to 0+020160
  DW_CFA_advance_loc: 0 to 0+020160
  DW_CFA_advance_loc: 0 to 0+020160
  DW_CFA_advance_loc: 0 to 0+020160
  DW_CFA_advance_loc: 0 to 0+020160
  DW_CFA_advance_loc: 0 to 0+020160

0+01d8 0+001c 0+0064 FDE cie=0+0178 pc=0+020190..0+0201d0
  DW_CFA_advance_loc: 0 to 0+020190
  DW_CFA_advance_loc: 0 to 0+020190
  DW_CFA_advance_loc: 0 to 0+020190
  DW_CFA_advance_loc: 0 to 0+020190
  DW_CFA_advance_loc: 0 to 0+020190
  DW_CFA_advance_loc: 0 to 0+020190
  DW_CFA_advance_loc: 0 to 0+020190

0+01f8 0+001c 0+01fc FDE cie=0+0000 pc=0+0201d0..0+0201e0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic1 removed, followed by repeat of above
0+0218 0+001c 0+021c FDE cie=0+0000 pc=0+0201e0..0+0201f0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0238 0+001c 0+023c FDE cie=0+0000 pc=0+0201f0..0+020210
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0258 0+001c 0+025c FDE cie=0+0000 pc=0+020210..0+020240
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0278 0+001c 0+027c FDE cie=0+0000 pc=0+020240..0+020280
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0298 0+001c 0+029c FDE cie=0+0000 pc=0+020280..0+0202d0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+02b8 0+001c 0+0208 FDE cie=0+00b4 pc=0+0202d0..0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202d0
  DW_CFA_advance_loc: 0 to 0+0202d0
  DW_CFA_advance_loc: 0 to 0+0202d0
  DW_CFA_advance_loc: 0 to 0+0202d0
  DW_CFA_advance_loc: 0 to 0+0202d0
  DW_CFA_advance_loc: 0 to 0+0202d0
  DW_CFA_advance_loc: 0 to 0+0202d0

0+02d8 0+001c 0+0228 FDE cie=0+00b4 pc=0+0202e0..0+020300
  DW_CFA_advance_loc: 0 to 0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202e0
  DW_CFA_advance_loc: 0 to 0+0202e0

0+02f8 0+001c 0+01e4 FDE cie=0+0118 pc=0+020300..0+020310
  DW_CFA_advance_loc: 0 to 0+020300
  DW_CFA_advance_loc: 0 to 0+020300
  DW_CFA_advance_loc: 0 to 0+020300
  DW_CFA_advance_loc: 0 to 0+020300
  DW_CFA_advance_loc: 0 to 0+020300
  DW_CFA_advance_loc: 0 to 0+020300
  DW_CFA_advance_loc: 0 to 0+020300

0+0318 0+001c 0+0204 FDE cie=0+0118 pc=0+020310..0+020330
  DW_CFA_advance_loc: 0 to 0+020310
  DW_CFA_advance_loc: 0 to 0+020310
  DW_CFA_advance_loc: 0 to 0+020310
  DW_CFA_advance_loc: 0 to 0+020310
  DW_CFA_advance_loc: 0 to 0+020310
  DW_CFA_advance_loc: 0 to 0+020310
  DW_CFA_advance_loc: 0 to 0+020310

0+0338 0+001c 0+01c4 FDE cie=0+0178 pc=0+020330..0+020340
  DW_CFA_advance_loc: 0 to 0+020330
  DW_CFA_advance_loc: 0 to 0+020330
  DW_CFA_advance_loc: 0 to 0+020330
  DW_CFA_advance_loc: 0 to 0+020330
  DW_CFA_advance_loc: 0 to 0+020330
  DW_CFA_advance_loc: 0 to 0+020330
  DW_CFA_advance_loc: 0 to 0+020330

0+0358 0+001c 0+01e4 FDE cie=0+0178 pc=0+020340..0+020370
  DW_CFA_advance_loc: 0 to 0+020340
  DW_CFA_advance_loc: 0 to 0+020340
  DW_CFA_advance_loc: 0 to 0+020340
  DW_CFA_advance_loc: 0 to 0+020340
  DW_CFA_advance_loc: 0 to 0+020340
  DW_CFA_advance_loc: 0 to 0+020340
  DW_CFA_advance_loc: 0 to 0+020340

0+0378 0+001c 0+0204 FDE cie=0+0178 pc=0+020370..0+0203b0
  DW_CFA_advance_loc: 0 to 0+020370
  DW_CFA_advance_loc: 0 to 0+020370
  DW_CFA_advance_loc: 0 to 0+020370
  DW_CFA_advance_loc: 0 to 0+020370
  DW_CFA_advance_loc: 0 to 0+020370
  DW_CFA_advance_loc: 0 to 0+020370
  DW_CFA_advance_loc: 0 to 0+020370

0+0398 0+0018 0+039c FDE cie=0+0000 pc=0+0203b0..0+0203c0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

