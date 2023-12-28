#name: PR ld/14215
#as: --32
#ld: -melf_i386 -shared -z relro
#readelf: -l --wide

#failif
#...
   03     .dynamic .got .data 
#...
