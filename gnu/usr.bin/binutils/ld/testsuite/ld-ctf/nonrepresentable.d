#as:
#source: nonrepresentable-1.c
#source: nonrepresentable-2.c
#objdump: --ctf
#ld: -shared
#name: Nonrepresentable types

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
  Function objects:
#...
    foo -> 0x[0-9]*: \(kind 5\) int \(\*\) \(\(nonrepresentable type.*\)\) \(aligned at .*\)
#...
  Types:
#...
    0x[0-9a-f]*: \(kind 0\) \(nonrepresentable type.*\)
#...

  Strings:
#...
