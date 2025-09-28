#source: start1.s
#source: globsym1ref.s
#source: globsymw2.s
#target: cris-*-*elf* cris-*-*aout*
#as: --em=crisaout
#ld: -mcrisaout
#warning: warning: isatty is not implemented, will always fail$
#objdump: -p
.*:     file format a\.out-cris
#pass
