#source: ../../../elf/ifunc-1.s
#readelf: -s
#name: .set with IFUNC

#...
[ 	]+[0-9]+:[ 	]+[0-9a-f]+[ 	]+[0-9]+[ 	]+IFUNC[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+[1-9] __GI_foo
[ 	]+[0-9]+:[ 	]+[0-9a-f]+[ 	]+[0-9]+[ 	]+IFUNC[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+[1-9] foo
#pass
