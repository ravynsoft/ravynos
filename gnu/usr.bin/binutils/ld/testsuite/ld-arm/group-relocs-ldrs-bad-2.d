#name: LDRS group relocations failure test
#source: group-relocs-ldrs-bad-2.s
#ld: -Ttext 0x8000 --section-start foo=0x8000100
#error: overflow whilst splitting 0x7ff8100 for group relocation
