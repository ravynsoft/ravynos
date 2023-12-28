# Check that types with the same names in distinct TUs show up as
# conflicting.
#as:
#source: cross-tu-cyclic-1.c
#source: cross-tu-cyclic-2.c
#objdump: --ctf
#ld: -shared
#name: cross-TU-cyclic-conflicting

.*:     file format .*

Contents of CTF section \.ctf:

#...
  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct B .*
#...
    0x[0-9a-f]*: \(kind 1\) int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
#...
    0x[0-9a-f]*: \(kind 1\) long int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
#...
    0x[0-9a-f]*: \(kind 9\) struct A
#...
    0x[0-9a-f]*: \(kind 6\) struct C .*
#...

  Strings:
#...

CTF archive member: .*/ld/testsuite/ld-ctf/cross-tu-cyclic-1\.c:
#...
  Types:
    0x80[0-9a-f]*: \(kind 6\) struct A .*
        *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 1\) long int .*
        *\[0x[0-9a-f]*\] foo: ID 0x[0-9a-f]*\: \(kind 3\) struct B \* .*

  Strings:
#...

CTF archive member: .*/ld/testsuite/ld-ctf/cross-tu-cyclic-2\.c:
#...
  Types:
    0x80[0-9a-f]*: \(kind 6\) struct A .*
        *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 1\) long int .*
        *\[0x[0-9a-f]*\] foo: ID 0x[0-9a-f]*: \(kind 3\) struct B \* .*
        *\[0x[0-9a-f]*\] bar: ID 0x[0-9a-f]*: \(kind 3\) struct C \* .*

  Strings:
#...
