SCRIPT_NAME=mmo
TARGET_PAGE_SIZE=256

# Default to 0 as mmixal does.
TEXT_START_ADDR='DEFINED (__.MMIX.start..text) ? __.MMIX.start..text : 0'
DATA_ADDR='DEFINED (__.MMIX.start..data) ? __.MMIX.start..data : 0x2000000000000000'
OUTPUT_FORMAT=mmo
RELOCATEABLE_OUTPUT_FORMAT=elf64-mmix
ARCH=mmix
EXTRA_EM_FILE=mmo
