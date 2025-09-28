#as: -mabi=lp64
#readelf: -S
#name: AArch64 section tail padding
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

There are .* section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  \[ 0\]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  \[ 1\] \.text             PROGBITS         0000000000000000  00000040
       0000000000000000  0000000000000000  AX       0     0     1
  \[ 2\] \.data             PROGBITS         0000000000000000  00000040
       0000000000000008  0000000000000000  WA       0     0     64
  \[ 3\] \.bss              NOBITS           0000000000000000  00000080
       000000000000000c  0000000000000000  WA       0     0     64
#...
