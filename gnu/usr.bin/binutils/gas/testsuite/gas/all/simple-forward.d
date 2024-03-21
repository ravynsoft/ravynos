#objdump: -s -j .data
#name: simple forward references
# tic30 and tic4x have 4 octets per byte, tic54x has 2 octets per byte
#notarget: *c30-*-* *c4x-*-* *c54x-*-*
# am33, crx and mn10300 all emit relocs unnecessarily for this test,
# but the code they generate is correct.  Others emit incorrect relocs
# which lead to incorrect results after linking.
#xfail: am33*-*-* crx-*-* mn10300-*-*

.*: .*

Contents of section \.data:
 0000 0c000000 (0c000000 0c000000|000c0000 0000000c) .*
#pass
