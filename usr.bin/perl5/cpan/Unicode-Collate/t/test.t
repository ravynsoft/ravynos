
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..96\n"; }
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

my $A_acute = _pack_U(0xC1);
my $a_acute = _pack_U(0xE1);
my $acute   = _pack_U(0x0301);

my $hiragana = "\x{3042}\x{3044}";
my $katakana = "\x{30A2}\x{30A4}";

# 1

my $Collator = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
);

ok(ref $Collator, "Unicode::Collate");

ok($Collator->cmp("", ""), 0);
ok($Collator->eq("", ""));
ok($Collator->cmp("", "perl"), -1);

ok(
  join(':', $Collator->sort( qw/ acha aca ada acia acka / ) ),
  join(':',                  qw/ aca acha acia acka ada / ),
);

ok(
  join(':', $Collator->sort( qw/ ACHA ACA ADA ACIA ACKA / ) ),
  join(':',                  qw/ ACA ACHA ACIA ACKA ADA / ),
);

# 7

ok($Collator->cmp("A$acute", $A_acute), 0); # @version 3.1.1 (prev: -1)
ok($Collator->cmp($a_acute, $A_acute), -1);
ok($Collator->eq("A\cA$acute", $A_acute)); # UCA v9. \cA is invariant.

my %old_level = $Collator->change(level => 1);
ok($Collator->eq("A$acute", $A_acute));
ok($Collator->eq("A", $A_acute));

ok($Collator->change(level => 2)->eq($a_acute, $A_acute));
ok($Collator->lt("A", $A_acute));

ok($Collator->change(%old_level)->lt("A", $A_acute));
ok($Collator->lt("A", $A_acute));
ok($Collator->lt("A", $a_acute));
ok($Collator->lt($a_acute, $A_acute));

# 18

$Collator->change(level => 2);

ok($Collator->{level}, 2);

ok( $Collator->cmp("ABC","abc"), 0);
ok( $Collator->eq("ABC","abc") );
ok( $Collator->le("ABC","abc") );
ok( $Collator->cmp($hiragana, $katakana), 0);
ok( $Collator->eq($hiragana, $katakana) );
ok( $Collator->ge($hiragana, $katakana) );

# 25

# hangul
ok( $Collator->eq("a\x{AC00}b", "a\x{1100}\x{1161}b") );
ok( $Collator->eq("a\x{AE00}b", "a\x{1100}\x{1173}\x{11AF}b") );
ok( $Collator->gt("a\x{AE00}b", "a\x{1100}\x{1173}b\x{11AF}") );
ok( $Collator->lt("a\x{AC00}b", "a\x{AE00}b") );
ok( $Collator->gt("a\x{D7A3}b", "a\x{C544}b") );
ok( $Collator->lt("a\x{C544}b", "a\x{30A2}b") ); # hangul < hiragana

# 31

$Collator->change(%old_level, katakana_before_hiragana => 1);

ok($Collator->{level}, 4);

ok( $Collator->cmp("abc", "ABC"), -1);
ok( $Collator->ne("abc", "ABC") );
ok( $Collator->lt("abc", "ABC") );
ok( $Collator->le("abc", "ABC") );
ok( $Collator->cmp($hiragana, $katakana), 1);
ok( $Collator->ne($hiragana, $katakana) );
ok( $Collator->gt($hiragana, $katakana) );
ok( $Collator->ge($hiragana, $katakana) );

# 40

$Collator->change(upper_before_lower => 1);

ok( $Collator->cmp("abc", "ABC"), 1);
ok( $Collator->ge("abc", "ABC"), 1);
ok( $Collator->gt("abc", "ABC"), 1);
ok( $Collator->cmp($hiragana, $katakana), 1);
ok( $Collator->ge($hiragana, $katakana), 1);
ok( $Collator->gt($hiragana, $katakana), 1);

# 46

$Collator->change(katakana_before_hiragana => 0);

ok( $Collator->cmp("abc", "ABC"), 1);
ok( $Collator->cmp($hiragana, $katakana), -1);

# 48

$Collator->change(upper_before_lower => 0);

ok( $Collator->cmp("abc", "ABC"), -1);
ok( $Collator->le("abc", "ABC") );
ok( $Collator->cmp($hiragana, $katakana), -1);
ok( $Collator->lt($hiragana, $katakana) );

# 52

{
    my $ignoreAE = Unicode::Collate->new(
	table => 'keys.txt',
	normalization => undef,
	ignoreChar => qr/^[aAeE]$/,
    );
    ok($ignoreAE->eq("element","lament"));
    ok($ignoreAE->eq("Perl","ePrl"));
}

# 54

{
    my $undefAE = Unicode::Collate->new(
	table => 'keys.txt',
	normalization => undef,
	undefChar => qr/^[aAeE]$/,
    );
    ok($undefAE ->gt("edge","fog"));
    ok($Collator->lt("edge","fog"));
    ok($undefAE ->gt("lake","like"));
    ok($Collator->lt("lake","like"));
}

# 58

{
    my $dropArticles = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	preprocess => sub {
	    my $string = shift;
	    $string =~ s/\b(?:an?|the)\s+//ig;
	    $string;
	},
    );
    ok($dropArticles->eq("camel", "a    camel"));
    ok($dropArticles->eq("Perl", "The Perl"));
    ok($dropArticles->lt("the pen", "a pencil"));
    ok($Collator->lt("Perl", "The Perl"));
    ok($Collator->gt("the pen", "a pencil"));
}

# 63

{
    my $undefName = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	undefName => qr/HANGUL|HIRAGANA|KATAKANA|BOPOMOFO/,
    );
    # HIRAGANA and KATAKANA are made undefined via undefName.
    # So they are after CJK Unified Ideographs.

    ok($undefName->lt("\x{4E00}", $hiragana));
    ok($undefName->lt("\x{4E03}", $katakana));
    ok($Collator ->gt("\x{4E00}", $hiragana));
    ok($Collator ->gt("\x{4E03}", $katakana));
}

# 67

{
    my $O_str = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	entry => <<'ENTRIES',
0008  ; [*0008.0000.0000.0000] # BACKSPACE (need to be non-ignorable)
004F 0337 ; [.0B53.0020.0008.004F] # capital O WITH SHORT SOLIDUS OVERLAY
006F 0008 002F ; [.0B53.0020.0002.006F] # LATIN SMALL LETTER O WITH STROKE
004F 0008 002F ; [.0B53.0020.0008.004F] # LATIN CAPITAL LETTER O WITH STROKE
006F 0337 ; [.0B53.0020.0002.004F] # small O WITH SHORT SOLIDUS OVERLAY
200B  ; [.2000.0000.0000.0000] # ZERO WIDTH SPACE (may be non-sense but ...)
#00F8 ; [.0B53.0020.0002.00F8] # LATIN SMALL LETTER O WITH STROKE
#00D8 ; [.0B53.0020.0008.00D8] # LATIN CAPITAL LETTER O WITH STROKE
ENTRIES
    );

    my $o_BS_slash = _pack_U(0x006F, 0x0008, 0x002F);
    my $O_BS_slash = _pack_U(0x004F, 0x0008, 0x002F);
    my $o_sol    = _pack_U(0x006F, 0x0337);
    my $O_sol    = _pack_U(0x004F, 0x0337);
    my $o_stroke = _pack_U(0x00F8);
    my $O_stroke = _pack_U(0x00D8);

    ok($O_str->eq($o_stroke, $o_BS_slash));
    ok($O_str->eq($O_stroke, $O_BS_slash));

    ok($O_str->eq($o_stroke, $o_sol));
    ok($O_str->eq($O_stroke, $O_sol));

    ok($Collator->eq("\x{200B}", "\0"));
    ok($O_str   ->gt("\x{200B}", "\0"));
    ok($O_str   ->gt("\x{200B}", "A"));
}

# 74

my %origVer = $Collator->change(UCA_Version => 8);

$Collator->change(level => 3);

ok($Collator->gt("!\x{300}", ""));
ok($Collator->gt("!\x{300}", "!"));
ok($Collator->eq("!\x{300}", "\x{300}"));

$Collator->change(level => 2);

ok($Collator->eq("!\x{300}", "\x{300}"));

$Collator->change(level => 4);

ok($Collator->gt("!\x{300}", "!"));
ok($Collator->lt("!\x{300}", "\x{300}"));

$Collator->change(%origVer, level => 3);

ok($Collator->eq("!\x{300}", ""));
ok($Collator->eq("!\x{300}", "!"));
ok($Collator->lt("!\x{300}", "\x{300}"));

$Collator->change(level => 4);

ok($Collator->gt("!\x{300}", ""));
ok($Collator->eq("!\x{300}", "!"));

# 85

$_ = 'Foo';

my $c = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  upper_before_lower => 1,
);

ok($_, 'Foo'); # fixed at v. 0.52; no longer clobber $_

my($temp, @temp); # Not the result but the side effect matters.

$_ = 'Foo';
$temp = $c->getSortKey("abc");
ok($_, 'Foo');

$_ = 'Foo';
$temp = $c->viewSortKey("abc");
ok($_, 'Foo');

$_ = 'Foo';
@temp = $c->sort("abc", "xyz", "def");
ok($_, 'Foo');

$_ = 'Foo';
@temp = $c->index("perl5", "RL");
ok($_, 'Foo');

$_ = 'Foo';
@temp = $c->index("perl5", "LR");
ok($_, 'Foo');

# 91

{
    my $caseless = Unicode::Collate->new(
	table => "keys.txt",
	normalization => undef,
	preprocess => sub { uc shift },
    );
    ok( $Collator->gt("ABC","abc") );
    ok( $caseless->eq("ABC","abc") );
}

# 93

{
    eval { require Unicode::Normalize; };
    if ($@) {
	eval { my $n1 = Unicode::Collate->new(table => "keys.txt"); };
        ok($@ =~ /Unicode::Normalize is required/);

	eval { my $n2 = Unicode::Collate->new
		(table => "keys.txt", normalization => undef); };
	ok(!$@);

	eval { my $n3 = Unicode::Collate->new
		(table => "keys.txt", normalization => 'prenormalized'); };
        ok($@ =~ /Unicode::Normalize is required/);
    } else {
	ok(1) for 1..3;
    }
}

# 96

