SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-tilegx-le"
BIG_OUTPUT_FORMAT="elf32-tilegx-be"
LITTLE_OUTPUT_FORMAT="elf32-tilegx-le"
TEXT_START_ADDR=0x10000
NO_REL_RELOCS=yes
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
# See also `include/elf/tilegx.h'
ARCH=tilegx
ALIGNMENT=64
MACHINE=
NOP=0
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
GENERATE_COMBRELOC_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
NO_SMALL_DATA=yes
SEPARATE_GOTPLT="SIZEOF (.got.plt) >= 8 ? 8 : 0"
# Look for 32 bit target libraries in /lib32, /usr/lib32 etc., first.
LIBPATH_SUFFIX=32
OTHER_SECTIONS="
  /* TILE architecture interrupt vector areas */
  .intrpt0 0xfc000000 : { KEEP(*(.intrpt0)) }
  .intrpt1 0xfd000000 : { KEEP(*(.intrpt1)) }
  .intrpt2 0xfe000000 : { KEEP(*(.intrpt2)) }
  .intrpt3 0xff000000 : { KEEP(*(.intrpt3)) }
"
