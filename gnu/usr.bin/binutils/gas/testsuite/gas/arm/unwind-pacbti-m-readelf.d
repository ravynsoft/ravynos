#readelf: -u
#source: unwind-pacbti-m.s
#name: Unwind table information for Armv8.1-M.Mainline PACBTI extension
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince
# VxWorks needs a special variant of this file.
#skip: *-*-vxworks*

Unwind section '.ARM.exidx' at offset 0x60 contains 1 entry:

0x0 <foo>: @0x0
  Compact model index: 1
  0xb4      pop {ra_auth_code}
  0x84 0x00 pop {r14}
  0xa3      pop {r4, r5, r6, r7}
  0xb4      pop {ra_auth_code}
  0x84 0x00 pop {r14}
  0xb4      pop {ra_auth_code}
  0xa8      pop {r4, r14}
  0xb0      finish
