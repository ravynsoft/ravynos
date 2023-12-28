#source: tls.s
#as: -a64
#ld: tmpdir/libtlslib.so
#objdump: -sj.tdata
#target: powerpc64*-*-*

.*

Contents of section \.tdata:
 .* (12345678|f0debc9a) (9abcdef0|78563412) (23456789|01efcdab) (abcdef01|89674523)  .*
 .* (3456789a|12f0debc) (bcdef012|9a785634) (456789ab|2301efcd) (cdef0123|ab896745)  .*
 .* (56789abc|3412f0de) (def01234|bc9a7856) (6789abcd|452301ef) (ef012345|cdab8967)  .*
 .* (789abcde|563412f0) (f0123456|debc9a78)                    .*
