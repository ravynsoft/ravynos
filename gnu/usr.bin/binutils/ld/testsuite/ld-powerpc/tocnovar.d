#source: tocnovar.s
#as: -a64
#ld: -melf64ppc -z relro
#readelf: -l --wide
#target: powerpc64*-*-*

#...
 +LOAD .*
 +LOAD .*
 +GNU_RELRO .*
#...
 +00 +\.text 
 +01 +\.opd \.got 
 +02 +\.opd \.got 
