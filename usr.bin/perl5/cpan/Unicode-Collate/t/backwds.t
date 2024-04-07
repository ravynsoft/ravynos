
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..36\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

# 2..12
{
    my $backLevel1 = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	backwards => [ 1 ],
    );

    ok($backLevel1->gt("a\x{300}a", "aa\x{300}"));
    ok($backLevel1->lt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($backLevel1->lt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));

    # all strings are reversed at level 1.
    ok($backLevel1->gt("AB", "BA"));
    ok($backLevel1->gt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));

    $backLevel1->change(backwards => []);
    ok($backLevel1->lt("AB", "BA"));
    ok($backLevel1->lt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));

    $backLevel1->change(backwards => 1);
    ok($backLevel1->gt("AB", "BA"));
    ok($backLevel1->gt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));

    $backLevel1->change(backwards => undef);
    ok($backLevel1->lt("AB", "BA"));
    ok($backLevel1->lt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));
}

# 13..26
{
    my $backLevel2 = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	backwards => 2,
    );

    ok($backLevel2->lt("AB", "BA"));
    ok($backLevel2->lt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));

    # all strings are reversed at level 2.
    ok($backLevel2->lt("a\x{300}a", "aa\x{300}"));
    ok($backLevel2->gt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($backLevel2->gt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));

    $backLevel2->change(backwards => undef);
    ok($backLevel2->gt("a\x{300}a", "aa\x{300}"));
    ok($backLevel2->lt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($backLevel2->lt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));

    $backLevel2->change(backwards => [2]);
    ok($backLevel2->lt("a\x{300}a", "aa\x{300}"));
    ok($backLevel2->gt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($backLevel2->gt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));

    $backLevel2->change(backwards => []);
    ok($backLevel2->gt("a\x{300}a", "aa\x{300}"));
    ok($backLevel2->lt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($backLevel2->lt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));
}

# 27..31
{
    my $undef = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	backwards => undef,
    );

    ok($undef->lt("AB", "BA"));
    ok($undef->lt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));

    ok($undef->gt("a\x{300}a", "aa\x{300}"));
    ok($undef->lt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($undef->lt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));
}

# 32..36
{
    my $empty = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	backwards => [ ],
    );

    ok($empty->lt("AB", "BA"));
    ok($empty->lt("\x{3042}\x{3044}", "\x{3044}\x{3042}"));

    ok($empty->gt("a\x{300}a", "aa\x{300}"));
    ok($empty->lt("Ca\x{300}ca\x{302}", "ca\x{302}ca\x{300}"));
    ok($empty->lt("ca\x{300}ca\x{302}", "Ca\x{302}ca\x{300}"));
}

