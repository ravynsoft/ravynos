#name: nto-stack-note-2
#source: start.s
#as:
#ld: -z stack-size=0x10000 --lazy-stack -z execstack
#readelf: -n

Displaying notes found in: .note
[ 	]+Owner[ 	]+Data size[ 	]+Description
  QNX                  0x0000000c	QNX stack
   Stack Size: 0x10000
   Stack allocated: 1000
   Executable: yes
