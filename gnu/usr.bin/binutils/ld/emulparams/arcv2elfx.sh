source_sh ${srcdir}/emulparams/arc-endianness.sh
MACHINE=
SCRIPT_NAME=elfarcv2
if [ "x${ARC_ENDIAN}" = "xbig" ]; then
  OUTPUT_FORMAT="elf32-bigarc"
else
  OUTPUT_FORMAT="elf32-littlearc"
fi
STARTUP_MEMORY=startup
TEXT_MEMORY=text
DATA_MEMORY=data
SDATA_MEMORY=sdata
ARCH=arc
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
GENERIC_BOARD=yes
TEMPLATE_NAME=elf
LITTLE_OUTPUT_FORMAT="elf32-littlearc"
BIG_OUTPUT_FORMAT="elf32-bigarc"
TEXT_START_ADDR=0x100
ENTRY=__start
SDATA_START_SYMBOLS='__SDATA_BEGIN__ = . + 0x100;'
JLI_START_TABLE='__JLI_TABLE__ = .;'
OTHER_SECTIONS="/DISCARD/ : { *(.__arc_profile_*) }"
EMBEDDED=yes
