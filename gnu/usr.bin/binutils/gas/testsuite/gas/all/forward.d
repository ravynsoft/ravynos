#objdump: -s -j .data
#name: forward references
# tic30 and tic4x have 4 octets per byte, tic54x has 2 octets per byte
#notarget: *c30-*-* *c4x-*-* *c54x-*-*
# hppa uses non-standard .equ syntax
#notarget: hppa*-*-*
# linkrelax targets should really resolve the relocs in this test but some
# choose to emit them even though the relocs are in .data, leading to
# "redefined symbol cannot be used on reloc".
#xfail: am33*-*-* crx*-*-* h8300*-*-* mn10200*-*-* mn10300*-*-*
# mep and microblaze use complex relocs and don't resolve the relocs.
# one of the relocs references minus_one, which is a bug, but no one
# seems bothered enough to fix this.
#xfail: mep-*-* microblaze-*-*

.*: .*

Contents of section .data:
 0000 01020304 ff0203fc 01020304 ff0203fc  ................
#pass
