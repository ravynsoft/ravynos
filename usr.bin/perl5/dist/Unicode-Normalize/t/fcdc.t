
BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir('t') if -d 't';
        @INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

#########################

use strict;
use warnings;
BEGIN { $| = 1; print "1..70\n"; }
my $count = 0;
sub ok { Unicode::Normalize::ok(\$count, @_) }

use Unicode::Normalize qw(:all);

ok(1);

sub _pack_U { Unicode::Normalize::dot_t_pack_U(@_) }
sub hexU { _pack_U map hex, split ' ', shift }
sub answer { defined $_[0] ? $_[0] ? "YES" : "NO" : "MAYBE" }

#########################

ok(FCD(''), "");
ok(FCC(''), "");
ok(FCD('A'), "A");
ok(FCC('A'), "A");

ok(normalize('FCD', ""), "");
ok(normalize('FCC', ""), "");
ok(normalize('FCC', "A"), "A");
ok(normalize('FCD', "A"), "A");

# 9

# if checkFCD is YES, the return value from FCD should be same as the original
ok(FCD(hexU("00C5")),		hexU("00C5"));		# A with ring above
ok(FCD(hexU("0041 030A")),	hexU("0041 030A"));	# A+ring
ok(FCD(hexU("0041 0327 030A")), hexU("0041 0327 030A")); # A+cedilla+ring
ok(FCD(hexU("AC01 1100 1161")), hexU("AC01 1100 1161")); # hangul
ok(FCD(hexU("212B F900")),	hexU("212B F900"));	# compat

ok(normalize('FCD', hexU("00C5")),		hexU("00C5"));
ok(normalize('FCD', hexU("0041 030A")),		hexU("0041 030A"));
ok(normalize('FCD', hexU("0041 0327 030A")),	hexU("0041 0327 030A"));
ok(normalize('FCD', hexU("AC01 1100 1161")),	hexU("AC01 1100 1161"));
ok(normalize('FCD', hexU("212B F900")),		hexU("212B F900"));

# 19

# if checkFCD is MAYBE or NO, FCD returns NFD (this behavior isn't documented)
ok(FCD(hexU("00C5 0327")),	hexU("0041 0327 030A"));
ok(FCD(hexU("0041 030A 0327")),	hexU("0041 0327 030A"));
ok(FCD(hexU("00C5 0327")),	NFD(hexU("00C5 0327")));
ok(FCD(hexU("0041 030A 0327")),	NFD(hexU("0041 030A 0327")));

ok(normalize('FCD', hexU("00C5 0327")),		hexU("0041 0327 030A"));
ok(normalize('FCD', hexU("0041 030A 0327")),	hexU("0041 0327 030A"));
ok(normalize('FCD', hexU("00C5 0327")),		NFD(hexU("00C5 0327")));
ok(normalize('FCD', hexU("0041 030A 0327")),	NFD(hexU("0041 030A 0327")));

# 27

ok(answer(checkFCD('')), 'YES');
ok(answer(checkFCD('A')), 'YES');
ok(answer(checkFCD("\x{030A}")), 'YES');  # 030A;COMBINING RING ABOVE
ok(answer(checkFCD("\x{0327}")), 'YES');  # 0327;COMBINING CEDILLA
ok(answer(checkFCD(_pack_U(0x00C5))), 'YES'); # A with ring above
ok(answer(checkFCD(hexU("0041 030A"))), 'YES'); # A+ring
ok(answer(checkFCD(hexU("0041 0327 030A"))), 'YES'); # A+cedilla+ring
ok(answer(checkFCD(hexU("0041 030A 0327"))), 'NO');  # A+ring+cedilla
ok(answer(checkFCD(hexU("00C5 0327"))), 'NO');    # A-ring+cedilla
ok(answer(checkNFC(hexU("00C5 0327"))), 'MAYBE'); # NFC: A-ring+cedilla
ok(answer(check("FCD", hexU("00C5 0327"))), 'NO');
ok(answer(check("NFC", hexU("00C5 0327"))), 'MAYBE');
ok(answer(checkFCD("\x{AC01}\x{1100}\x{1161}")), 'YES'); # hangul
ok(answer(checkFCD("\x{212B}\x{F900}")), 'YES'); # compat

ok(answer(checkFCD(hexU("1EA7 05AE 0315 0062"))), "NO");
ok(answer(checkFCC(hexU("1EA7 05AE 0315 0062"))), "NO");
ok(answer(check('FCD', hexU("1EA7 05AE 0315 0062"))), "NO");
ok(answer(check('FCC', hexU("1EA7 05AE 0315 0062"))), "NO");

# 45

ok(FCC(hexU("00C5 0327")), hexU("0041 0327 030A"));
ok(FCC(hexU("0045 0304 0300")), "\x{1E14}");
ok(FCC("\x{1100}\x{1161}\x{1100}\x{1173}\x{11AF}"), "\x{AC00}\x{AE00}");
ok(normalize('FCC', hexU("00C5 0327")), hexU("0041 0327 030A"));
ok(normalize('FCC', hexU("0045 0304 0300")), "\x{1E14}");
ok(normalize('FCC', hexU("1100 1161 1100 1173 11AF")), "\x{AC00}\x{AE00}");

ok(FCC("\x{0B47}\x{0300}\x{0B3E}"), "\x{0B47}\x{0300}\x{0B3E}");
ok(FCC("\x{1100}\x{0300}\x{1161}"), "\x{1100}\x{0300}\x{1161}");
ok(FCC("\x{0B47}\x{0B3E}\x{0300}"), "\x{0B4B}\x{0300}");
ok(FCC("\x{1100}\x{1161}\x{0300}"), "\x{AC00}\x{0300}");
ok(FCC("\x{0B47}\x{300}\x{0B3E}\x{327}"), "\x{0B47}\x{300}\x{0B3E}\x{327}");
ok(FCC("\x{1100}\x{300}\x{1161}\x{327}"), "\x{1100}\x{300}\x{1161}\x{327}");

# 57

ok(answer(checkFCC('')), 'YES');
ok(answer(checkFCC('A')), 'YES');
ok(answer(checkFCC("\x{030A}")), 'MAYBE');  # 030A;COMBINING RING ABOVE
ok(answer(checkFCC("\x{0327}")), 'MAYBE'); # 0327;COMBINING CEDILLA
ok(answer(checkFCC(hexU("00C5"))), 'YES'); # A with ring above
ok(answer(checkFCC(hexU("0041 030A"))), 'MAYBE'); # A+ring
ok(answer(checkFCC(hexU("0041 0327 030A"))), 'MAYBE'); # A+cedilla+ring
ok(answer(checkFCC(hexU("0041 030A 0327"))), 'NO');    # A+ring+cedilla
ok(answer(checkFCC(hexU("00C5 0327"))), 'NO'); # A-ring+cedilla
ok(answer(checkFCC("\x{AC01}\x{1100}\x{1161}")), 'MAYBE'); # hangul
ok(answer(checkFCC("\x{212B}\x{F900}")), 'NO'); # compat
ok(answer(checkFCC("\x{212B}\x{0327}")), 'NO'); # compat
ok(answer(checkFCC("\x{0327}\x{212B}")), 'NO'); # compat

# 70

