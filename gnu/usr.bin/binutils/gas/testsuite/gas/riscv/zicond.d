#as: -march=rv64i_zicond
#source: zicond.s
#objdump: -d

.*:[	 ]+file format .*


Disassembly of section .text:

0+000 <target>:
[	 ]+0:[	 ]+0ec5d533[	 ]+czero.eqz[	 ]+a0,a1,a2
[	 ]+4:[	 ]+0ee6f533[	 ]+czero.nez[	 ]+a0,a3,a4
