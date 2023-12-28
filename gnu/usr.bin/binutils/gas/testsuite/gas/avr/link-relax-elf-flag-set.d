#name: AVR, check elf link-relax header flag is set.
#as: -mlink-relax -mmcu=avrxmega2
#readelf: -h
#source: link-relax-elf-flag.s
#target: avr-*-*

ELF Header:
#...
  Flags:                             0xe6, avr:102, link-relax
#...