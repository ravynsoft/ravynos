source_sh ${srcdir}/emulparams/plt_unwind.sh
source_sh ${srcdir}/emulparams/extern_protected_data.sh
source_sh ${srcdir}/emulparams/dynamic_undefined_weak.sh
source_sh ${srcdir}/emulparams/call_nop.sh
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-i386"
EXTRA_EM_FILE="elf-x86"
NO_RELA_RELOCS=yes
TEXT_START_ADDR=0x08048000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ARCH=i386
MACHINE=
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
ELF_INTERPRETER_NAME=\"/usr/lib/ld.so.1\"
NO_SMALL_DATA=yes
