#as: -march=rv32ih
#source: h-ext-32.s
#objdump: -d

.*:[  	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+22000073[ 	]+hfence.vvma
[ 	]+[0-9a-f]+:[ 	]+22050073[ 	]+hfence.vvma[ 	]+a0
[ 	]+[0-9a-f]+:[ 	]+22b00073[ 	]+hfence.vvma[ 	]+zero,a1
[ 	]+[0-9a-f]+:[ 	]+22c58073[ 	]+hfence.vvma[ 	]+a1,a2
[ 	]+[0-9a-f]+:[ 	]+62000073[ 	]+hfence.gvma
[ 	]+[0-9a-f]+:[ 	]+62050073[ 	]+hfence.gvma[ 	]+a0
[ 	]+[0-9a-f]+:[ 	]+62b00073[ 	]+hfence.gvma[ 	]+zero,a1
[ 	]+[0-9a-f]+:[ 	]+62c58073[ 	]+hfence.gvma[ 	]+a1,a2
[     	]+[0-9a-f]+:[  	]+6005c573[    	]+hlv.b[      	]+a0,\(a1\)
[     	]+[0-9a-f]+:[  	]+6005c573[    	]+hlv.b[      	]+a0,\(a1\)
[     	]+[0-9a-f]+:[  	]+6015c573[    	]+hlv.bu[     	]+a0,\(a1\)
[     	]+[0-9a-f]+:[  	]+640645f3[    	]+hlv.h[      	]+a1,\(a2\)
[     	]+[0-9a-f]+:[  	]+6415c5f3[    	]+hlv.hu[     	]+a1,\(a1\)
[     	]+[0-9a-f]+:[  	]+643645f3[    	]+hlvx.hu[    	]+a1,\(a2\)
[     	]+[0-9a-f]+:[  	]+68064673[    	]+hlv.w[      	]+a2,\(a2\)
[     	]+[0-9a-f]+:[  	]+6836c673[    	]+hlvx.wu[    	]+a2,\(a3\)
[     	]+[0-9a-f]+:[  	]+62a5c073[    	]+hsv.b[      	]+a0,\(a1\)
[     	]+[0-9a-f]+:[  	]+62a5c073[    	]+hsv.b[      	]+a0,\(a1\)
[     	]+[0-9a-f]+:[  	]+66a5c073[    	]+hsv.h[      	]+a0,\(a1\)
[     	]+[0-9a-f]+:[  	]+6aa5c073[    	]+hsv.w[      	]+a0,\(a1\)
