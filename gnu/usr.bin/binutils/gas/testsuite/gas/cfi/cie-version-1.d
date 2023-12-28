#objdump: --dwarf=frames
#name: CIE Version 1
#as: --gdwarf-cie-version=1
#source: cie-version.s
#...
.*:     file format .*

Contents of the .eh_frame section:

00000000 0+[0-9a-f]+ 0+000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01][abc]
#...