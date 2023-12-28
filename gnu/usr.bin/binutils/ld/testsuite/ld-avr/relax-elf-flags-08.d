#name: AVR, check link-relax flag is clear final link (both inputs relaxable)
#as: -mmcu=avrxmega2
#ld: -mavrxmega2
#source: relax-elf-flags-a.s -mlink-relax
#source: relax-elf-flags-b.s -mlink-relax
#readelf: -h
#target: avr-*-*

ELF Header:
#...
  Flags:                             0x66, avr:102
#...