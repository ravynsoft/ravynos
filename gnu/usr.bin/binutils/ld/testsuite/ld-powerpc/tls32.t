#source: tls32.s
#source: tlslib32.s
#as: -a32
#ld: 
#objdump: -sj.tdata
#target: powerpc*-*-*

.*

Contents of section \.tdata:
 1810124 (12345678|78563412) (23456789|89674523) (3456789a|9a785634) (456789ab|ab896745)  .*
 1810134 (56789abc|bc9a7856) (6789abcd|cdab8967) (789abcde|debc9a78) (00c0ffee|eeffc000)  .*
