#as: --EL
#objdump: -s -j .data
#name: eBPF data directives

.*: +file format .*bpf.*

Contents of section \.data:
 0000 0ff0efbe adde8877 66554433 2211 .*
