#name: AVR, check link-relax flag is set final link (no inputs relaxable)
#as: -mmcu=avrxmega2
#ld: -relax -mavrxmega2
#source: relax-elf-flags-a.s -mno-link-relax
#source: relax-elf-flags-b.s -mno-link-relax
#readelf: -h
#target: avr-*-*

ELF Header:
#...
  Flags:                             0xe6, avr:102, link-relax
#...