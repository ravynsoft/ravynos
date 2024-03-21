#source: start.s
#source: endsym.s
#ld: --sort-common
#nm: -n
#xfail: m68hc1*-* xgate-* cr16-*-* crx-*-* dlx-*-* nds32*-*-* visium-*-* s12z-*-*
#xfail: pru-*-*

#...
.* end
#...
.* end2
#...
.* _?_end
#pass
