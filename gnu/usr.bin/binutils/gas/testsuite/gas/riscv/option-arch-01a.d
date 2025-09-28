#as: -misa-spec=2.2
#source: option-arch-01.s
#objdump: -d

.*:[   ]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[  	]+952e[    	]+add[        	]+a0,a0,a1
[ 	]+[0-9a-f]+:[  	]+00b50533[    	]+add[        	]+a0,a0,a1
[ 	]+[0-9a-f]+:[  	]+00302573[    	]+frcsr[        	]+a0
#...
