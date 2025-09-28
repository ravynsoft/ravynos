#source: init-fini-arrays.s
#ld:
#nm: -n
# Most targets with their own scripts don't support init/fini_array and
# thus don't define __init/fini_array_start.
#xfail: avr-*-* cr16-*-* crx-*-* d10v-*-* d30v-*-* dlx-*-* ft32-*-* iq2000-*-*
#xfail: m68hc1*-*-* mep-*-* microblaze*-*-elf* s12z-*-* v850-*-* visium-*-*
#xfail: xgate-*-* xstormy*-*-*
# Some targets with their own scripts haven't kept up with elf.sc and
# PROVIDE __init_array_start rather than using PROVIDE_HIDDEN.  These
# result in D symbols.  rx-elf makes .init/fini_array SHF_EXECINSTR so
# gets t symbols.

#...
[0-9a-f]+ [dDt] _?__init_array_start
#...
[0-9a-f]+ [dDt] _?__fini_array_start
#pass
