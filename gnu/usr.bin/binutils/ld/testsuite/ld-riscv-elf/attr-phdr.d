#name: PT_RISCV_ATTRIBUTES check
#source: attr-phdr.s
#as: -march=rv32ic
#ld: -m[riscv_choose_ilp32_emul]
#readelf: -l

Elf file type is EXEC \(Executable file\)
Entry point .*
There are .* program headers, starting at offset .*

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  RISCV_ATTRIBUT .*
  LOAD           .*

 Section to Segment mapping:
  Segment Sections...
   00     .riscv.attributes 
   01     .text 
