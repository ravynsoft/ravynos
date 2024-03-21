#objdump: --dwarf=frames
#as: --gdwarf-cie-version=3
#source: cie-rtn-col.s

.*:     file format elf.*-.*riscv

Contents of the .* section:


00000000 [a-zA-Z0-9]+ [a-zA-Z0-9]+ CIE
  Version:               3
  Augmentation:          .*
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: 4929
  Augmentation data:     .*
#...
