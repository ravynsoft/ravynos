#as:
#cc: -O2
#source: data-func-1.c
#source: data-func-2.c
#objdump: --ctf
#ld: -shared -s
#name: Conflicted data syms, partially indexed, stripped

.*: +file format .*

Contents of CTF section \.ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Data object section:	.* \(0x[1-9a-f][0-9a-f]* bytes\)
    Function info section:	.* \(0x[1-9a-f][0-9a-f]* bytes\)
    Object index section:	.* \(0xc bytes\)
    Type section:	.* \(0x118 bytes\)
    String section:	.*
#...
  Data objects:
    bar -> 0x[0-9a-f]*: \(kind 6\) struct var_3 \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    var_1 -> 0x[0-9a-f]*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_666 -> 0x[0-9a-f]*: \(kind 3\) foo_t \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*

  Function objects:
    func_[0-9]* -> 0x[0-9a-f]*: \(kind 5\) void \*\(\*\) \(const char \*restrict, int \(\*\)\(\*\) \(const char \*\)\) \(aligned at 0x[0-9a-f]*\)
#...
  Types:
#...
    .*: \(kind 6\) struct var_3 .*
#...
CTF archive member: .*/data-func-1\.c:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Parent name: \.ctf
    Compilation unit name: .*/data-func-1\.c
    Data object section:	.* \(0x[1-9a-f][0-9a-f]* bytes\)
    Type section:	.* \(0xc bytes\)
    String section:	.*

  Labels:

  Data objects:
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
    var_[0-9]* -> 0x80000001*: \(kind 10\) foo_t \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> .*
#...
  Function objects:

  Variables:

  Types:
    0x80000001: \(kind 10\) foo_t .* -> .* int .*
#...
