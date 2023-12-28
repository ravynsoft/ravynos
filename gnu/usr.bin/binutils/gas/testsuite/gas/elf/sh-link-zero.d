#readelf: --wide --sections
#name: Setting the sh_link field to 0
#source: sh-link-zero.s

#...
.*\.meta1.*WAL[ 	]+0.*
#...
.*\.meta2.*WAL[ 	]+[1-9].*
#pass
