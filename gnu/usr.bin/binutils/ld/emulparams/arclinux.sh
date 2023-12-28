source_sh ${srcdir}/emulparams/arc-endianness.sh
ARCH=arc
SCRIPT_NAME=arclinux
if [ "x${ARC_ENDIAN}" = "xbig" ]; then
  OUTPUT_FORMAT="elf32-bigarc"
else
  OUTPUT_FORMAT="elf32-littlearc"
fi
LITTLE_OUTPUT_FORMAT="elf32-littlearc"
BIG_OUTPUT_FORMAT="elf32-bigarc"
TEXT_START_ADDR=0x10000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ENTRY=__start
TEMPLATE_NAME=elf
EXTRA_EM_FILE=arclinux
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes

OTHER_SECTIONS="/DISCARD/ : { *(.__arc_profile_*) }"

# To support RELRO security feature.
NO_SMALL_DATA=yes
SEPARATE_GOTPLT=4
GENERATE_COMBRELOC_SCRIPT=yes
