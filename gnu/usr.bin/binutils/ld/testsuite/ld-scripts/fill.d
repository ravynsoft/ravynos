#source: fill_0.s
#source: fill_1.s
#source: fill_2.s
#ld: -T fill.t
#objdump: -s -j .text
#notarget: [is_aout_format]
#skip: ia64-*-* mips*-*-freebsd* mips*-*-gnu* mips*-*-irix* mips*-*-kfreebsd*
#skip: mips*-*-linux* mips*-*-netbsd* mips*-*-openbsd* mips*-*-sysv4* sh-*-pe
#skip: tilegx*-*-* tilepro-*-* x86_64-*-cygwin x86_64-*-mingw* x86_64-*-pe*
#xfail: alpha*-*-*ecoff
#xfail: tic30-*-coff tic4x-*-* tic54x-*-* z8k-*-*
#
# See also fill16.d.  We use `skip' for configurations unsupported
# here that are covered there, and `xfail' for configurations that work
# in neither place.  See below for details as to why individual
# configurations are listed above.
#
# alpha-linuxecoff pads out code to 16 bytes.
# ia64 aligns code to minimum 16 bytes.
# mips aligns to minimum 16 bytes (except for bare-metal ELF and VxWorks).
# sh-pe pads out code sections to 16 bytes
# tic30-coff aligns to 2 bytes
# tic4x has 4 octet bytes
# tic54x doesn't support .p2align
# tilegx aligns code to minimum 8 bytes.
# tilepro aligns code to minimum 8 bytes.
# x86_64-pe aligns to 16 bytes
# z8k-coff aligns to 2 bytes

.*:     file format .*

Contents of section .text:
 [0-9a-f]+ cafebabe 01010101 02020202 12232323 .*
 [0-9a-f]+ 03030303 00345600 00004567 000089ab .*
 [0-9a-f]+ (deadbeef|efbeadde) 00004567 000089ab 0000cdef .*
 [0-9a-f]+ 00004567 000089ab 0000cdef 00000123 .*
