#name: PE-COFF Long section names in objects (default)
#ld:  -r
#objdump: -h
#source: longsecn.s

.*:     file format .*

Sections:
Idx Name          Size      VMA +LMA +File off  Algn
  0 \.(text|bss )         [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  (CONTENTS, ALLOC, LOAD, (READONLY, )?CODE|ALLOC)
  1 \.text\.very\.long\.section\.name [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, READONLY, CODE, DATA
  2 \.data         [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  3 \.data\$1       [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  4 \.rodata\$1     [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  5 \.data\$123     [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  6 \.rodata\$123   [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  7 \.data\$123456789 [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  8 \.rodata\$123456789 [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
  9 \.data\.very\.long\.section [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
 10 \.rodata\.very\.long\.section [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
 11 \.data\.very\.long\.section\$1 [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
 12 \.rodata\.very\.long\.section\$1 [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
 13 \.data\.very\.long\.section\$1234 [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
 14 \.rodata\.very\.long\.section\$1234 [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  [0-9a-fA-F]+  2\*\*[0-9]
                  CONTENTS, ALLOC, LOAD, DATA
#...
