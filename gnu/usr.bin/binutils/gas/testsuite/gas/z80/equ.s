 .data
_start:
lab0:	.equ .-_start
 .long lab3
lab1:	equ -(_start - .)
 .long lab2
lab2 	.equ (.-_start)
 .long lab1
lab3 	equ ~~(.-_start)
 .long lab0
