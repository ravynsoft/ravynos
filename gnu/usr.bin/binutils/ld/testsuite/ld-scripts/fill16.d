#source: fill16_0.s
#source: fill16_1.s
#source: fill16_2.s
#ld: -T fill.t
#objdump: -s -j .text
#notarget: [is_aout_format]
#skip: [is_xcoff_format]
#xfail: alpha*-*-*ecoff
#xfail: tic30-*-coff tic4x-*-* tic54x-*-* z8k-*-*
#xfail: z80-*-coff
#
# See also fill.d.  We use `skip' for configurations unsupported
# here that are covered there, and `xfail' for configurations that work
# with neither place.  See below for details as to why individual
# configurations are listed above.
#
# alpha-linuxecoff pads out code to 16 bytes.
# arm-coff always aligns code to 4 bytes.
# i386-coff always aligns code to 4 bytes.
# tic30-coff aligns to 2 bytes
# tic4x has 4 octet bytes
# tic54x doesn't support .p2align
# z8k-coff aligns to 2 bytes
# z80-coff aligns to 2 bytes

.*:     file format .*

Contents of section .text:
 [0-9a-f]+ cafebabe cafebabe cafebabe cafebabe .*
 [0-9a-f]+ 01010101 01010101 01010101 01010101 .*
 [0-9a-f]+ 02020202 02020202 02020202 02020202 .*
 [0-9a-f]+ 12232323 23232323 23232323 23232323 .*
 [0-9a-f]+ 03030303 03030303 03030303 03030303 .*
 [0-9a-f]+ 00345600 00004567 000089ab (deadbeef|efbeadde) .*
 [0-9a-f]+ 00004567 000089ab 0000cdef 00004567 .*
 [0-9a-f]+ 000089ab 0000cdef 00000123          .*
