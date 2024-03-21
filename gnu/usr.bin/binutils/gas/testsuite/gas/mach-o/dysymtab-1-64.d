#objdump: -P dysymtab
#target: x86_64-*-darwin* powerpc64-*-darwin*
#source: symbols-base-64.s
.*: +file format mach-o.*
#...
Load command dysymtab:
              local symbols: idx:          0  num: 6.*\(nxtidx: 6\)
           external symbols: idx:          6  num: 18.*\(nxtidx: 24\)
          undefined symbols: idx:         24  num: 21.*\(nxtidx: 45\)
           table of content: off: 0x00000000  num: 0.*\(endoff: 0x00000000\)
               module table: off: 0x00000000  num: 0.*\(endoff: 0x00000000\)
   external reference table: off: 0x00000000  num: 0.*\(endoff: 0x00000000\)
      indirect symbol table: off: 0x00000000  num: 0.*\(endoff: 0x00000000\)
  external relocation table: off: 0x00000000  num: 0.*\(endoff: 0x00000000\)
     local relocation table: off: 0x00000000  num: 0.*\(endoff: 0x00000000\)
