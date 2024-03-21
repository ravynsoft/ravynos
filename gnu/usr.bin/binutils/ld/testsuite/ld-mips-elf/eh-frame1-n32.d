#name: MIPS eh-frame 1, n32
#source: eh-frame1.s
#source: eh-frame1.s
#as: --defsym alignment=2 --defsym fill=0x40
#readelf: --relocs -wf
#ld: -shared --eh-frame-hdr -Teh-frame1.ld
#warning: FDE encoding in.*prevents \.eh_frame_hdr table being created.

Relocation section '\.rel\.dyn' .*:
 *Offset .*
00000000  [0-9a-f]+ R_MIPS_NONE *
# Initial PCs for the FDEs attached to CIE 0xbc
000300dc  [0-9a-f]+ R_MIPS_REL32 *
000300f0  [0-9a-f]+ R_MIPS_REL32 *
000301f8  [0-9a-f]+ R_MIPS_REL32 *
0003020c  [0-9a-f]+ R_MIPS_REL32 *
0003008b  [0-9a-f]+ R_MIPS_REL32      00000000   foo
000300d0  [0-9a-f]+ R_MIPS_REL32      00000000   foo
0003010e  [0-9a-f]+ R_MIPS_REL32      00000000   foo
Contents of the \.eh_frame section:

00000000 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     1b

  DW_CFA_advance_loc: 0 to 00000000
  DW_CFA_advance_loc: 0 to 00000000
  DW_CFA_advance_loc: 0 to 00000000

00000014 00000010 00000018 FDE cie=00000000 pc=00020000..00020010
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000028 00000010 0000002c FDE cie=00000000 pc=00020010..00020030
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic2 removed
0000003c 00000010 00000040 FDE cie=00000000 pc=00020030..00020060
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic3 removed
00000050 00000010 00000054 FDE cie=00000000 pc=00020060..000200a0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic4 removed
00000064 00000010 00000068 FDE cie=00000000 pc=000200a0..000200f0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000078 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zRP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     1b 00 00 00 00 00

  DW_CFA_advance_loc: 0 to 00000000
  DW_CFA_advance_loc: 0 to 00000000
  DW_CFA_advance_loc: 0 to 00000000
  DW_CFA_nop
  DW_CFA_nop

00000094 00000010 00000020 FDE cie=00000078 pc=000200f0..00020100
  DW_CFA_advance_loc: 0 to 000200f0
  DW_CFA_advance_loc: 0 to 000200f0
  DW_CFA_advance_loc: 0 to 000200f0

000000a8 00000010 00000034 FDE cie=00000078 pc=00020100..00020120
  DW_CFA_advance_loc: 0 to 00020100
  DW_CFA_advance_loc: 0 to 00020100
  DW_CFA_advance_loc: 0 to 00020100

000000bc 00000014 00000000 CIE
  Version:               1
  Augmentation:          "zP"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     50 00 00 00 00 00 00 00


000000d4 00000010 0000001c FDE cie=000000bc pc=00020120..00020130
  DW_CFA_advance_loc: 0 to 00020120
  DW_CFA_advance_loc: 0 to 00020120
  DW_CFA_advance_loc: 0 to 00020120

000000e8 00000010 00000030 FDE cie=000000bc pc=00020130..00020150
  DW_CFA_advance_loc: 0 to 00020130
  DW_CFA_advance_loc: 0 to 00020130
  DW_CFA_advance_loc: 0 to 00020130

000000fc 00000014 00000000 CIE
  Version:               1
  Augmentation:          "zPR"
  Code alignment factor: 1
  Data alignment factor: 4
  Return address column: 31
  Augmentation data:     00 00 00 00 00 1b

  DW_CFA_advance_loc: 0 to 00000000

00000114 00000010 0000001c FDE cie=000000fc pc=00020150..00020160
  DW_CFA_advance_loc: 0 to 00020150
  DW_CFA_advance_loc: 0 to 00020150
  DW_CFA_advance_loc: 0 to 00020150

# FDE for .discard removed
# zPR2 removed
00000128 00000010 00000030 FDE cie=000000fc pc=00020160..00020190
  DW_CFA_advance_loc: 0 to 00020160
  DW_CFA_advance_loc: 0 to 00020160
  DW_CFA_advance_loc: 0 to 00020160

0000013c 00000010 00000044 FDE cie=000000fc pc=00020190..000201d0
  DW_CFA_advance_loc: 0 to 00020190
  DW_CFA_advance_loc: 0 to 00020190
  DW_CFA_advance_loc: 0 to 00020190

00000150 00000010 00000154 FDE cie=00000000 pc=000201d0..000201e0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

# basic1 removed, followed by repeat of above
00000164 00000010 00000168 FDE cie=00000000 pc=000201e0..000201f0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000178 00000010 0000017c FDE cie=00000000 pc=000201f0..00020210
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0000018c 00000010 00000190 FDE cie=00000000 pc=00020210..00020240
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000001a0 00000010 000001a4 FDE cie=00000000 pc=00020240..00020280
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000001b4 00000010 000001b8 FDE cie=00000000 pc=00020280..000202d0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000001c8 00000010 00000154 FDE cie=00000078 pc=000202d0..000202e0
  DW_CFA_advance_loc: 0 to 000202d0
  DW_CFA_advance_loc: 0 to 000202d0
  DW_CFA_advance_loc: 0 to 000202d0

000001dc 00000010 00000168 FDE cie=00000078 pc=000202e0..00020300
  DW_CFA_advance_loc: 0 to 000202e0
  DW_CFA_advance_loc: 0 to 000202e0
  DW_CFA_advance_loc: 0 to 000202e0

000001f0 00000010 00000138 FDE cie=000000bc pc=00020300..00020310
  DW_CFA_advance_loc: 0 to 00020300
  DW_CFA_advance_loc: 0 to 00020300
  DW_CFA_advance_loc: 0 to 00020300

00000204 00000010 0000014c FDE cie=000000bc pc=00020310..00020330
  DW_CFA_advance_loc: 0 to 00020310
  DW_CFA_advance_loc: 0 to 00020310
  DW_CFA_advance_loc: 0 to 00020310

00000218 00000010 00000120 FDE cie=000000fc pc=00020330..00020340
  DW_CFA_advance_loc: 0 to 00020330
  DW_CFA_advance_loc: 0 to 00020330
  DW_CFA_advance_loc: 0 to 00020330

0000022c 00000010 00000134 FDE cie=000000fc pc=00020340..00020370
  DW_CFA_advance_loc: 0 to 00020340
  DW_CFA_advance_loc: 0 to 00020340
  DW_CFA_advance_loc: 0 to 00020340

00000240 00000010 00000148 FDE cie=000000fc pc=00020370..000203b0
  DW_CFA_advance_loc: 0 to 00020370
  DW_CFA_advance_loc: 0 to 00020370
  DW_CFA_advance_loc: 0 to 00020370

00000254 00000010 00000258 FDE cie=00000000 pc=000203b0..000203c0
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

