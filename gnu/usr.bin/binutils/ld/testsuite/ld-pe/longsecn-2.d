#name: PE-COFF Long section names (enabled)
#ld: --enable-long-section-names
#objdump: -h
#source: longsecn.s

.*:     file format .*

Sections:
Idx Name          Size      VMA +LMA +File off  Algn
  0 \.text         [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, READONLY, CODE, DATA
  1 \.data         [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  2 \.rodata       [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  3 \.data\.very\.long\.section [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  4 \.rodata\.very.long\.section [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  5 \.idata        [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
#...
