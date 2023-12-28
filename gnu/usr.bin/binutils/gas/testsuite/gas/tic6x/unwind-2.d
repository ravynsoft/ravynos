#readelf: -u
#name: C6X unwinding directives 2 (big endian)
#as: -mbig-endian
#source: unwind-2.s

Unwind section '.c6xabi.exidx' .*

0x0: 0x83020227
  Compact model index: 3
  Stack increment 8
  Registers restored: A11, B3
  Return register: B3

0x100: 0x808003e7
  Compact model index: 0
  0x80 0x03 pop {A10, A11}
  0xe7      RETURN

0x200: 0x81008863
  Compact model index: 1
  0x88 0x63 pop {A10, A11, B3, B10, B15}

0x300: 0x83020227
  Compact model index: 3
  Stack increment 8
  Registers restored: A11, B3
  Return register: B3

0x400: 0x84000227
  Compact model index: 4
  Stack increment 0
  Registers restored:  \(compact\) A11, B3
  Return register: B3

0x500: 0x80a022e7
  Compact model index: 0
  0xa0 0x22 pop compact {A11, B3}
  0xe7      RETURN

0x600: 0x84000227
  Compact model index: 4
  Stack increment 0
  Registers restored:  \(compact\) A11, B3
  Return register: B3

0x700: 0x84000637
  Compact model index: 4
  Stack increment 0
  Registers restored:  \(compact\) A10, A11, B3, B10
  Return register: B3

0x800: 0x840002d7
  Compact model index: 4
  Stack increment 0
  Registers restored:  \(compact\) A10, A12, A13, B3
  Return register: B3

0x900: 0x84000c07
  Compact model index: 4
  Stack increment 0
  Registers restored:  \(compact\) B10, B11
  Return register: B3

0xa00: 0x83ff0027
  Compact model index: 3
  Restore stack from frame pointer
  Registers restored: A11, A15
  Return register: B3

0xb00: 0x84ff0027
  Compact model index: 4
  Restore stack from frame pointer
  Registers restored:  \(compact\) A11, A15
  Return register: B3

0xc00: 0x8001c1f7
  Compact model index: 0
  0x01      sp = sp \+ 16
  0xc1 0xf7 pop frame {B3, \[pad\]}

0xd00: @0x.*
  Compact model index: 1
  0x01      sp = sp \+ 16
  0xc2 0xf7 0xbf pop frame {\[pad\], A11, B3, \[pad\]}
  0xe7      RETURN
  0xe7      RETURN

0xe00: @0x.*
  Compact model index: 1
  0x01      sp = sp \+ 16
  0xc2 0xf7 0xfb pop frame {A11, \[pad\], B3, \[pad\]}
  0xe7      RETURN
  0xe7      RETURN

0xf00: @0x.*
  Compact model index: 1
  0x02      sp = sp \+ 24
  0xc2 0x7f 0xff 0xfb pop frame {A11, \[pad\], \[pad\], \[pad\], \[pad\], B3}
  0xe7      RETURN

