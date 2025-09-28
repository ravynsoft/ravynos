# Check that noncyclic cross-TU matching-up works.
# We can guarantee the order of emitted structures, too.
#as:
#source: cross-tu-1.c
#source: cross-tu-2.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: cross-TU-noncyclic

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	.* \(0x74 bytes\)
    String section:	.*

  Labels:

  Data objects:
#...
  Function objects:

  Variables:
#...

  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct B .*
       *\[0x0\] foo: ID 0x[0-9a-f]*: \(kind 1\) int .*
#...
    0x[0-9a-f]*: \(kind 6\) struct A .*
       *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 1\) long int .*
       *\[0x[0-9a-f]*\] foo: ID 0x[0-9a-f]*: \(kind 3\) struct B \* .*
#...
    0x[0-9a-f]*: \(kind 3\) struct B \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]*: \(kind 6\) struct B .*
#...
    0x[0-9a-f]*: \(kind 3\) struct A \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]*: \(kind 6\) struct A .*
#...
