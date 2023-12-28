# Check that cross-TU chasing to a type in a cycle works.
# In particular, there should be only one copy of 'struct A *'.
# (Not entirely reliable: only fails if the two are emitted adjacently.
# Needs a proper objdump-CTF parser.)
#as:
#source: cross-tu-2.c
#source: cross-tu-cyclic-1.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: cross-TU-cyclic-nonconflicting

.*:     file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...

  Labels:

  Data objects:
#...
  Function objects:

  Variables:
#...

  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct A \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 1\) long int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x[0-9a-f]*\] foo: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    0x[0-9a-f]*: \(kind 1\) long int .*
    0x[0-9a-f]*: \(kind 6\) struct B \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x0\] foo: ID 0x[0-9a-f]*: \(kind 1\) int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x[0-9a-f]*\] bar: ID 0x[0-9a-f]*: \(kind 3\) struct A \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    0x[0-9a-f]*: \(kind 3\) struct B \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]*: \(kind 6\) struct B \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    0x[0-9a-f]*: \(kind 3\) struct A \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]*: \(kind 6\) struct A \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    0x[0-9a-f]*: \(kind 1\) int .*

  Strings:
#...
