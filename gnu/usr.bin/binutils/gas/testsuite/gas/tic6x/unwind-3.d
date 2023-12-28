#readelf: -u
#name: C6X unwinding directives 3 (segment change)
#source: unwind-3.s

Unwind section '.c6xabi.exidx.text.bar' .*

0x0: 0x830e2807
  Compact model index: 3
  Stack increment 56
  Registers restored: B11, B13
  Return register: B3

Unwind section '.c6xabi.exidx' .*

0x0: 0x80008021
  Compact model index: 0
  0x00      sp = sp \+ 8
  0x80 0x21 pop {A10, B3}
