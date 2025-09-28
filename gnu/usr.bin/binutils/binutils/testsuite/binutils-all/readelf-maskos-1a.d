#name: Unknown SHF_MASKOS value in section
#source: readelf-maskos-unknown.s
#notarget: mips-*-* hppa-*-* score-*-* msp430-*-elf visium-*-elf
#readelf: -S --wide
# Only run this test for targets that are not ELFOSABI_STANDALONE, and do not
# set SHF_MASKOS bit 0x8000000.

#...
  \[[ 0-9]+\] .data.var.*WAo.*
#pass
