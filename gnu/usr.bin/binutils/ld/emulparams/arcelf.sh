source_sh ${srcdir}/emulparams/arc-endianness.sh
SCRIPT_NAME=elfarc
TEMPLATE_NAME=elf
if [ "x${ARC_ENDIAN}" = "xbig" ]; then
  OUTPUT_FORMAT="elf32-bigarc"
else
  OUTPUT_FORMAT="elf32-littlearc"
fi
LITTLE_OUTPUT_FORMAT="elf32-littlearc"
BIG_OUTPUT_FORMAT="elf32-bigarc"
# leave room for vector table, 32 vectors * 8 bytes
TEXT_START_ADDR=0x100
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
#NONPAGED_TEXT_START_ADDR=0x0
ARCH=arc
MACHINE=
ENTRY=__start
SDATA_START_SYMBOLS='__SDATA_BEGIN__ = . + 0x100;'
JLI_START_TABLE='__JLI_TABLE__ = .;'
OTHER_SECTIONS="/DISCARD/ : { *(.__arc_profile_*) }"
EMBEDDED=yes

GENERATE_SHLIB_SCRIPT=yes
