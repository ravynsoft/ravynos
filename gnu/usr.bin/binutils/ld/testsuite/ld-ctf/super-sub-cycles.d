#as:
#source: super-sub-cycles.c
#objdump: --ctf
#ld: -shared
#name: Super- and sub-cycles

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Compilation unit name: .*super-sub-cycles.c
#...
    Type section:	.*\(0x108 bytes\)
#...
  Types:
#...
    0x[0-9a-f]: \(kind 6\) struct A \(.*
         \[0x0\] b: ID 0x[0-9a-f]*: \(kind 6\) struct B \(.*
             \[0x0\] c: ID 0x[0-9a-f]*: \(kind 6\) struct C \(.*
                 \[0x0\] a: ID 0x[0-9a-f]*: \(kind 3\) struct A \* \(.*
                 \[0x[0-9a-f]*\] d: ID 0x[0-9a-f]*: \(kind 6\) struct D \(.*
                     \[0x[0-9a-f]*\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(.*
             \[0x[0-9a-f]*\] d: ID 0x[0-9a-f]*: \(kind 6\) struct D \(.*
                 \[0x[0-9a-f]*\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(.*
         \[0x[0-9a-f]*\] x: ID 0x[0-9a-f]*: \(kind 6\) struct X \(.*
             \[0x[0-9a-f]*\] y: ID 0x[0-9a-f]*: \(kind 6\) struct Y \(.*
                 \[0x[0-9a-f]*\] z: ID 0x[0-9a-f]*: \(kind 6\) struct Z \(.*
                     \[0x[0-9a-f]*\] y: ID 0x[0-9a-f]*: \(kind 3\) struct Y \* \(.*
                     \[0x[0-9a-f]*\] d: ID 0x[0-9a-f]*: \(kind 3\) struct D \* \(.*
#...
