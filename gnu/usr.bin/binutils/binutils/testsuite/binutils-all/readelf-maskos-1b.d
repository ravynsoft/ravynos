#name: -t (section details) for unknown SHF_MASKOS value in section
#source: readelf-maskos-unknown.s
#notarget: mips-*-* hppa-*-* score-*-* msp430-*-elf visium-*-elf
#readelf: -S -t --wide
# Only run this test for targets that are not ELFOSABI_STANDALONE, and do not
# set SHF_MASKOS bit 0x8000000.

#...
  \[[ 0-9]+\] .data.var
       PROGBITS +0+ +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +0 +0 +(1|2|4|8)
       \[0+0800003\]: WRITE, ALLOC, OS \(0+0800000\)
#pass
