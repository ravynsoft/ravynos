
BEGIN {
    unless (5.008 <= $]) {
	print "1..0 # skipped: Perl 5.8.0 or later needed for this test\n";
	print $@;
	exit;
    }
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..90\n"; }
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

no warnings 'utf8';

# Unicode 6.0 Sorting
#
# Special Database Values. The data files for CLDR provide
# special weights for two noncharacters:
#
# 1. A special noncharacter <HIGH> (U+FFFF) for specification of a range
#    in a database, allowing "Sch" <= X <= "Sch<HIGH>" to pick all strings
#    starting with "sch" plus those that sort equivalently.
# 2. A special noncharacter <LOW> (U+FFFE) for merged database fields,
#    allowing "Disi\x{301}lva<LOW>John" to sort next to "Disilva<LOW>John".

my $entry = <<'ENTRIES';
FFFE  ; [.0001.0020.0005.FFFE] # <noncharacter-FFFE>
FFFF  ; [.FFFE.0020.0005.FFFF] # <noncharacter-FFFF>
ENTRIES

my @disilva = ("di Silva", "diSilva", "di Si\x{301}lva", "diSi\x{301}lva");
my @dsf = map "$_\x{FFFE}Fred", @disilva;
my @dsj = map "$_\x{FFFE}John", @disilva;
my @dsJ = map        "$_ John", @disilva;

for my $norm (undef, 'NFD') {
    if (defined $norm) {
	eval { require Unicode::Normalize };
	if ($@) {
	    ok(1) for 1..34; # silent skip
	    next;
	}
    }

    my $coll = Unicode::Collate->new(
	table => 'keys.txt',
	level => 1,
	normalization => $norm,
	UCA_Version => 22,
	entry => $entry,
    );

    # 1..4
    ok($coll->lt("\x{FFFD}",   "\x{FFFF}"));
    ok($coll->lt("\x{1FFFD}",  "\x{1FFFF}"));
    ok($coll->lt("\x{2FFFD}",  "\x{2FFFF}"));
    ok($coll->lt("\x{10FFFD}", "\x{10FFFF}"));

    # 5..14
    ok($coll->lt("perl\x{FFFD}",   "perl\x{FFFF}"));
    ok($coll->lt("perl\x{1FFFD}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{1FFFE}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{1FFFF}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{2FFFD}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{2FFFE}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{2FFFF}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{10FFFD}", "perl\x{FFFF}"));
    ok($coll->lt("perl\x{10FFFE}", "perl\x{FFFF}"));
    ok($coll->lt("perl\x{10FFFF}", "perl\x{FFFF}"));

    # 15..16
    ok($coll->gt("perl\x{FFFF}AB", "perl\x{FFFF}"));
    ok($coll->lt("perl\x{FFFF}\x{10FFFF}", "perl\x{FFFF}\x{FFFF}"));

    $coll->change(level => 4);

    # 17..25
    for my $i (0 .. $#disilva - 1) {
	ok($coll->lt($dsf[$i], $dsf[$i+1]));
	ok($coll->lt($dsj[$i], $dsj[$i+1]));
	ok($coll->lt($dsJ[$i], $dsJ[$i+1]));
    }

    # 26
    ok($coll->lt($dsf[-1], $dsj[0]));

    $coll->change(level => 1);

    # 27..34
    for my $i (0 .. $#disilva) {
	ok($coll->lt($dsf[$i], $dsJ[$i]));
	ok($coll->lt($dsj[$i], $dsJ[$i]));
    }
}

# 69

{
    my $coll = Unicode::Collate->new(
	table => 'keys.txt',
	normalization => undef,
	highestFFFF => 1,
	minimalFFFE => 1,
    );

    $coll->change(level => 1);
    ok($coll->lt("perl\x{FFFD}",   "perl\x{FFFF}"));
    ok($coll->lt("perl\x{1FFFD}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{1FFFE}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{1FFFF}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{2FFFD}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{2FFFE}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{2FFFF}",  "perl\x{FFFF}"));
    ok($coll->lt("perl\x{10FFFD}", "perl\x{FFFF}"));
    ok($coll->lt("perl\x{10FFFE}", "perl\x{FFFF}"));
    ok($coll->lt("perl\x{10FFFF}", "perl\x{FFFF}"));

# 79

    $coll->change(level => 3);
    my @list = (
	"ab\x{FFFE}a",
	"Ab\x{FFFE}a",
	"ab\x{FFFE}c",
	"Ab\x{FFFE}c",
	"ab\x{FFFE}xyz",
	"abc\x{FFFE}def",
	"abc\x{FFFE}xYz",
	"aBc\x{FFFE}xyz",
	"abcX\x{FFFE}def",
	"abcx\x{FFFE}xyz",
	"b\x{FFFE}aaa",
	"bbb\x{FFFE}a",
    );
    my $p = shift @list;
    for my $c (@list) {
	ok($coll->lt($p, $c));
	$p = $c;
    }
}

# 90
