#name: ALU group relocations failure test
#source: group-relocs-alu-bad-2.s
#ld: -Ttext 0x8000 --section-start foo=0x1208000
#error: overflow whilst splitting 0x1234 for group relocation
