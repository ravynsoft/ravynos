
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..40\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate::Locale;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

my $objNso = Unicode::Collate::Locale->
    new(locale => 'NSO', normalization => undef);

ok($objNso->getlocale, 'nso');

$objNso->change(level => 1);

ok($objNso->lt("e", "e\x{302}"));
ok($objNso->gt("f", "e\x{302}"));
ok($objNso->lt("o", "o\x{302}"));
ok($objNso->gt("p", "o\x{302}"));
ok($objNso->lt("s", "s\x{30C}"));
ok($objNso->gt("t", "s\x{30C}"));

# 8

$objNso->change(level => 2);

ok($objNso->eq("e\x{302}", "E\x{302}"));
ok($objNso->eq("o\x{302}", "O\x{302}"));
ok($objNso->eq("s\x{30C}", "S\x{30C}"));

$objNso->change(level => 3);

ok($objNso->lt("e\x{302}", "E\x{302}"));
ok($objNso->lt("o\x{302}", "O\x{302}"));
ok($objNso->lt("s\x{30C}", "S\x{30C}"));

# 14

ok($objNso->eq("e\x{302}", _pack_U(0xEA)));
ok($objNso->eq("E\x{302}", _pack_U(0xCA)));
ok($objNso->eq("o\x{302}", _pack_U(0xF4)));
ok($objNso->eq("O\x{302}", _pack_U(0xD4)));
ok($objNso->eq("s\x{30C}", "\x{161}"));
ok($objNso->eq("S\x{30C}", "\x{160}"));

# 20

ok($objNso->eq("e\x{302}\x{300}", "\x{1EC1}"));
ok($objNso->eq("E\x{302}\x{300}", "\x{1EC0}"));
ok($objNso->eq("e\x{302}\x{301}", "\x{1EBF}"));
ok($objNso->eq("E\x{302}\x{301}", "\x{1EBE}"));
ok($objNso->eq("e\x{302}\x{303}", "\x{1EC5}"));
ok($objNso->eq("E\x{302}\x{303}", "\x{1EC4}"));
ok($objNso->eq("e\x{302}\x{309}", "\x{1EC3}"));
ok($objNso->eq("E\x{302}\x{309}", "\x{1EC2}"));
ok($objNso->eq("e\x{302}\x{323}", "\x{1EC7}"));
ok($objNso->eq("E\x{302}\x{323}", "\x{1EC6}"));

ok($objNso->eq("o\x{302}\x{300}", "\x{1ED3}"));
ok($objNso->eq("O\x{302}\x{300}", "\x{1ED2}"));
ok($objNso->eq("o\x{302}\x{301}", "\x{1ED1}"));
ok($objNso->eq("O\x{302}\x{301}", "\x{1ED0}"));
ok($objNso->eq("o\x{302}\x{303}", "\x{1ED7}"));
ok($objNso->eq("O\x{302}\x{303}", "\x{1ED6}"));
ok($objNso->eq("o\x{302}\x{309}", "\x{1ED5}"));
ok($objNso->eq("O\x{302}\x{309}", "\x{1ED4}"));
ok($objNso->eq("o\x{302}\x{323}", "\x{1ED9}"));
ok($objNso->eq("O\x{302}\x{323}", "\x{1ED8}"));

# 40
