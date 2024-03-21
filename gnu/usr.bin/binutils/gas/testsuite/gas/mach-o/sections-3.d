#objdump: -P section
#notarget: x86_64-*-darwin*
.*: +file format mach-o.*
#...
 Section: __symbol_stub    __TEXT.*\(bfdname: .symbol_stub\)
  addr: (00000000)?00000000 size: (00000000)?00000000 offset: (00000000)?00000000
  align: 0  nreloc: 0  reloff: (00000000)?00000000
  flags: 80000008 \(type: symbol_stubs attr: pure_instructions\)
  first indirect sym: 0 \(0 entries\)  stub size: (16|20)  reserved3: 0x0
 Section: __la_symbol_ptr  __DATA.*\(bfdname: .lazy_symbol_pointer\)
  addr: (00000000)?00000000 size: (00000000)?00000000 offset: (00000000)?00000000
  align: 2  nreloc: 0  reloff: (00000000)?00000000
  flags: 00000007 \(type: lazy_symbol_pointers attr: -\)
  first indirect sym: 0 \(0 entries\)  reserved2: 0x0  reserved3: 0x0
 Section: __nl_symbol_ptr  __DATA.*\(bfdname: .non_lazy_symbol_pointer\)
  addr: (00000000)?00000000 size: (00000000)?00000000 offset: (00000000)?00000000
  align: 2  nreloc: 0  reloff: (00000000)?00000000
  flags: 00000006 \(type: non_lazy_symbol_pointers attr: -\)
  first indirect sym: 0 \(0 entries\)  reserved2: 0x0  reserved3: 0x0
