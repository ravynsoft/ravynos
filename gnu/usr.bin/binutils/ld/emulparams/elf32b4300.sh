# If you change this file, please also look at files which source this one:
# elf32l4300.sh

EMBEDDED=yes
source_sh ${srcdir}/emulparams/elf32bmip.sh
TEXT_START_ADDR=0xa0020000
unset SHLIB_TEXT_START_ADDR
EXECUTABLE_SYMBOLS='_DYNAMIC_LINK = 0;'
DYNAMIC_LINK=false
