
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..104\n"; }
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

my $objTa = Unicode::Collate::Locale->
    new(locale => 'TA', normalization => undef);

ok($objTa->getlocale, 'ta');

$objTa->change(level => 1);

my $Kssa = "\x{B95}\x{BCD}\x{BB7}";
my $v    = "\x{BCD}";

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : "";
    $objTa->change(highestFFFF => 1) if $h;

    ok($objTa->lt("\x{B94}$t",   "\x{B82}"));
    ok($objTa->lt("\x{B82}$t",   "\x{B83}"));
    ok($objTa->lt("\x{B83}$t",   "\x{B95}$v"));
    ok($objTa->lt("\x{B95}$v$t", "\x{B95}"));
    ok($objTa->lt("\x{B95}$t",   "\x{B99}$v"));
    ok($objTa->lt("\x{B99}$v$t", "\x{B99}"));
    ok($objTa->lt("\x{B99}$t",   "\x{B9A}$v"));
    ok($objTa->lt("\x{B9A}$v$t", "\x{B9A}"));
    ok($objTa->lt("\x{B9A}$t",   "\x{B9E}$v"));
    ok($objTa->lt("\x{B9E}$v$t", "\x{B9E}"));
    ok($objTa->lt("\x{B9E}$t",   "\x{B9F}$v"));
    ok($objTa->lt("\x{B9F}$v$t", "\x{B9F}"));
    ok($objTa->lt("\x{B9F}$t",   "\x{BA3}$v"));
    ok($objTa->lt("\x{BA3}$v$t", "\x{BA3}"));
    ok($objTa->lt("\x{BA3}$t",   "\x{BA4}$v"));
    ok($objTa->lt("\x{BA4}$v$t", "\x{BA4}"));
    ok($objTa->lt("\x{BA4}$t",   "\x{BA8}$v"));
    ok($objTa->lt("\x{BA8}$v$t", "\x{BA8}"));
    ok($objTa->lt("\x{BA8}$t",   "\x{BAA}$v"));
    ok($objTa->lt("\x{BAA}$v$t", "\x{BAA}"));
    ok($objTa->lt("\x{BAA}$t",   "\x{BAE}$v"));
    ok($objTa->lt("\x{BAE}$v$t", "\x{BAE}"));
    ok($objTa->lt("\x{BAE}$t",   "\x{BAF}$v"));
    ok($objTa->lt("\x{BAF}$v$t", "\x{BAF}"));
    ok($objTa->lt("\x{BAF}$t",   "\x{BB0}$v"));
    ok($objTa->lt("\x{BB0}$v$t", "\x{BB0}"));
    ok($objTa->lt("\x{BB0}$t",   "\x{BB2}$v"));
    ok($objTa->lt("\x{BB2}$v$t", "\x{BB2}"));
    ok($objTa->lt("\x{BB2}$t",   "\x{BB5}$v"));
    ok($objTa->lt("\x{BB5}$v$t", "\x{BB5}"));
    ok($objTa->lt("\x{BB5}$t",   "\x{BB4}$v"));
    ok($objTa->lt("\x{BB4}$v$t", "\x{BB4}"));
    ok($objTa->lt("\x{BB4}$t",   "\x{BB3}$v"));
    ok($objTa->lt("\x{BB3}$v$t", "\x{BB3}"));
    ok($objTa->lt("\x{BB3}$t",   "\x{BB1}$v"));
    ok($objTa->lt("\x{BB1}$v$t", "\x{BB1}"));
    ok($objTa->lt("\x{BB1}$t",   "\x{BA9}$v"));
    ok($objTa->lt("\x{BA9}$v$t", "\x{BA9}"));
    ok($objTa->lt("\x{BA9}$t",   "\x{B9C}$v"));
    ok($objTa->lt("\x{B9C}$v$t", "\x{B9C}"));
    ok($objTa->lt("\x{B9C}$t",   "\x{BB6}$v"));
    ok($objTa->lt("\x{BB6}$v$t", "\x{BB6}"));
    ok($objTa->lt("\x{BB6}$t",   "\x{BB7}$v"));
    ok($objTa->lt("\x{BB7}$v$t", "\x{BB7}"));
    ok($objTa->lt("\x{BB7}$t",   "\x{BB8}$v"));
    ok($objTa->lt("\x{BB8}$v$t", "\x{BB8}"));
    ok($objTa->lt("\x{BB8}$t",   "\x{BB9}$v"));
    ok($objTa->lt("\x{BB9}$v$t", "\x{BB9}"));
    ok($objTa->lt("\x{BB9}$t",   "${Kssa}$v"));
    ok($objTa->lt("${Kssa}$v$t", "${Kssa}"));
    ok($objTa->lt("${Kssa}$t",   "\x{BBE}"));
}

# 104
