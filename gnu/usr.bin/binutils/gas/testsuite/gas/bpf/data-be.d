#as: --EB
#source: data.s
#objdump: -s -j .data
#name: eBPF data directives, big endian

.*: +file format .*bpf.*

Contents of section \.data:
 0000 f00fdead beef1122 33445566 7788 .*
