#name: AVR, check link-relax flag is set on partial link
#as: -mmcu=avrxmega2
#ld: -r -mavrxmega2
#source: relax-elf-flags-a.s -mlink-relax
#source: relax-elf-flags-b.s -mlink-relax
#readelf: -h
#target: avr-*-*

ELF Header:
#...
  Flags:                             0xe6, avr:102, link-relax
#...