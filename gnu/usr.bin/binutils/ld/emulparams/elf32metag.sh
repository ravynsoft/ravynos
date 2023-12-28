MACHINE=
SCRIPT_NAME=elf
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
OUTPUT_FORMAT="elf32-metag"
TEXT_START_ADDR=0x10005000
ARCH=metag
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ENTRY=__start
NOP=0xa0fffffe
EXTRA_EM_FILE=metagelf
USER_LABEL_PREFIX=_
test -n "${RELOCATING}" && OTHER_SECTIONS="
  .core_text 0x80000000	:
  {
    *(.core_text)
  }
  .core_data 0x82000000	:
  {
    *(.core_data)
    *(.core_rodata)
  }
  .internal_memory 0xe0200000 :
  {
    *(.internal_text)
    *(.internal_data)
    *(.internal_rodata)
  }
"
