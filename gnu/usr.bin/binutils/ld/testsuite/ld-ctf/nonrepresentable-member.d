#as:
#source: nonrepresentable-member.c
#objdump: --ctf
#ld: -shared
#name: Nonrepresentable members

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct blah .*
        *\[0x0\] boring: ID 0x[0-9a-f]*: \(kind 1\) int .*
        *\[0x[0-9a-f]*\] foo: .* \(.*represent.*\)
        *\[0x[0-9a-f]*\] bar: .* \(.*represent.*\)
        *\[0x[0-9a-f]*\] this_is_printed: ID 0x[0-9a-f]*: \(kind 1\) int .*
#...

  Strings:
#...
