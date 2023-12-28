#name: CSDB
#source: csdb.s
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
.*> f3af 8014 ?	csdb
.*> f3bf 8f40 ?	ssbb
.*> f3bf 8f44 ?	pssbb
.*> e320f014 ?	csdb
.*> f57ff040 ?	ssbb
.*> f57ff044 ?	pssbb
