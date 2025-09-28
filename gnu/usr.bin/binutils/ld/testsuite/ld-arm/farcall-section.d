#name: ARM-ARM farcall to symbol of type STT_SECTION
#source: farcall-section.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x2001014
#error: .*\(.text\+0x0\): relocation truncated to fit: R_ARM_CALL against `.foo'
