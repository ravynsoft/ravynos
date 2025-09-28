#as:
#ld: -shared -z combreloc
#objdump: -R

.*: +file format .*

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
[[:xdigit:]]+ R_LARCH_IRELATIVE +\*ABS\*\+0x[[:xdigit:]]+
[[:xdigit:]]+ R_LARCH_64 +test
