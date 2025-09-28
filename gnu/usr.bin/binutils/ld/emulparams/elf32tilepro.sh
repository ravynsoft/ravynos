SCRIPT_NAME=elf
if [ -z "$OUTPUT_FORMAT" ]; then
    # Allow overriding externally to "elf32-tile64" if desired
    OUTPUT_FORMAT=elf32-tilepro
fi
TEXT_START_ADDR=0x10000
NO_REL_RELOCS=yes
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
# See also `include/elf/tilepro.h'
ARCH=tilepro
ALIGNMENT=64
MACHINE=
NOP=0
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
GENERATE_COMBRELOC_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
NO_SMALL_DATA=yes
SEPARATE_GOTPLT="SIZEOF (.got.plt) >= 8 ? 8 : 0"
OTHER_SECTIONS="
  /* TILEPRO architecture interrupt vector areas */
  .intrpt0 0xfc000000 : { KEEP(*(.intrpt0)) }
  .intrpt1 0xfd000000 : { KEEP(*(.intrpt1)) }
  .intrpt2 0xfe000000 : { KEEP(*(.intrpt2)) }
  .intrpt3 0xff000000 : { KEEP(*(.intrpt3)) }
"
