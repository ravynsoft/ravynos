#source: tls32.s
#as: -a32
#ld: tmpdir/libtlslib32.so
#objdump: -sj.tdata
#target: powerpc*-*-*

.*

Contents of section \.tdata:
.* (12345678|78563412) (23456789|89674523) (3456789a|9a785634) (456789ab|ab896745)  .*
.* (56789abc|bc9a7856) (6789abcd|cdab8967) (789abcde|debc9a78)           .*
