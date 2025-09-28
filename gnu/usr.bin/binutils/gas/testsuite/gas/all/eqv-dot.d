#objdump: -s -j .data
#name: eqv involving dot
# bfin doesn't support 'symbol = expression'
# tic30 and tic4x have 4 octets per byte, tic54x has 2 octets per byte
#notarget: bfin-*-* *c30-*-* *c4x-*-* *c54x-*-* *bpf-*-*
# linkrelax targets don't handle equivalence expressions well (nor any
# other forward expression).  mep uses complex relocs
#xfail: am33_2.0-*-* crx-*-* h8300-*-* mn10200-*-* mn10300-*-* mep-*-*

.*: .*

Contents of section \.data:
 0000 (0+00 0+01 0+02 0+0c|000+ 010+ 020+ 0c0+) .*
 0010 (0+10 0+14 0+10 0+1c|100+ 140+ 100+ 1c0+) .*
#pass
