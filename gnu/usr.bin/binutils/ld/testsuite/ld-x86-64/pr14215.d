#name: PR ld/14215
#as: --64
#ld: -melf_x86_64 -shared -z relro
#readelf: -l --wide

#failif
#...
   03     .dynamic .got .data 
#...
