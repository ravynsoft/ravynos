#name: Unwind Stack Frame information for Armv8.1-M.Mainline PAC extension
#source: ehabi-pacbti-m.s
#as: -march=armv8.1-m.main+mve+pacbti
#readelf: -u
#target: [is_elf_format]

Unwind section '.ARM.exidx' at offset 0x5c contains 1 entry:

0x0: @0x0
  Compact model index: 1
  0xb1 0x08 pop {r3}
  0x80 0x08 pop {r7}
  0xb4      pop {ra_auth_code}
  0x84 0x00 pop {r14}
  0xb1 0x0f pop {r0, r1, r2, r3}
  0xb5      vsp as modifier for PAC validation
