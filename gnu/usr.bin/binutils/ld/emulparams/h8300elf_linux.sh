# If you change this file, please also look at files which source this one:
# h8300helf.sh h8300self.sh

SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-h8300-linux"
NO_REL_RELOCS=yes
TEXT_START_ADDR=0x100
MAXPAGESIZE=2
TARGET_PAGE_SIZE=128
ARCH=h8300
TEMPLATE_NAME=elf
EMBEDDED=yes
STACK_ADDR=0xfefc
TINY_READONLY_SECTION=".tinyrodata :
  {
	*(.tinyrodata)
  } =0"
TINY_DATA_SECTION=".tinydata	${RELOCATING+0xff8000} :
  {
	*(.tinydata)
	${RELOCATING+ _tinydata = .; }
  }"
TINY_BSS_SECTION=".tinybss	: ${RELOCATING+AT (_tinydata)}
  {
	*(.tinybss)
  }"
