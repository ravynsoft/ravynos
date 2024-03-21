	.section .text
	.extern exdat1c
	.extern exdat2b
	.extern exdat1a
	.globl sda21_test

sda21_test:
	e_add16i  5, 4, exdat1c@sda21
	e_add16i  5, 4, exdat2b@sda21
	e_add16i  5, 4, exdat0b@sda21
