#name: objdump -p print 64-bit values
#source: dyn-sec64.s
#as: -mips3
#ld: -Tdyn-sec64.ld -shared
#objdump: -p

.*: .* file format .*

Program Header:
.* LOAD .*
.*
.* LOAD .*
.*
.* DYNAMIC .*
.*
.* NULL .*
.*

Dynamic Section:

  INIT .* 0x0001234000003000
  FINI .* 0x0001234000004000
  HASH .* 0x0001234000001000
  STRTAB .*
  SYMTAB .*
  STRSZ .*
  SYMENT .*
  PLTGOT .* 0x0001235000000000
  REL .* 0x0001234000002000
#pass
