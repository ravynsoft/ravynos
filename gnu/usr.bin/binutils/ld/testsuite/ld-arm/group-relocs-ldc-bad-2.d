#name: LDC group relocations failure test
#source: group-relocs-ldc-bad-2.s
#ld: -Ttext 0x8000 --section-start foo=0x118400
#error: overflow whilst splitting 0x123456 for group relocation
