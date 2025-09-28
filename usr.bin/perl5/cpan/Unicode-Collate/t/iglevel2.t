
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..42\n"; }
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

my $Collator = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  ignore_level2 => 1,
  entry => << 'ENTRIES',
1B00  ; [.0000.00FF.0002.1B00] # BALINESE SIGN ULU RICEM
1B01  ; [.0000.0100.0002.1B01] # BALINESE SIGN ULU CANDRA
1B02  ; [.0000.0101.0002.1B02] # BALINESE SIGN CECEK
03C6  ; [.1900.0020.0002.03C6] # GREEK SMALL LETTER PHI
03D5  ; [.1900.0020.0004.03D5] # GREEK PHI SYMBOL; QQK
03A6  ; [.1900.0020.0008.03A6] # GREEK CAPITAL LETTER PHI
ENTRIES
);

ok($Collator->eq("camel", "came\x{300}l"));
ok($Collator->eq("camel", "ca\x{300}me\x{301}l"));
ok($Collator->lt("camel", "Camel"));

# 4

$Collator->change(ignore_level2 => 0);

ok($Collator->lt("camel", "came\x{300}l"));
ok($Collator->lt("camel", "ca\x{300}me\x{301}l"));
ok($Collator->lt("camel", "Camel"));

$Collator->change(level => 1);

ok($Collator->eq("camel", "came\x{300}l"));
ok($Collator->eq("camel", "ca\x{300}me\x{301}l"));
ok($Collator->eq("camel", "Camel"));

$Collator->change(level => 2);

ok($Collator->lt("camel", "came\x{300}l"));
ok($Collator->lt("camel", "ca\x{300}me\x{301}l"));
ok($Collator->eq("camel", "Camel"));

# 13

$Collator->change(ignore_level2 => 1);

ok($Collator->eq("camel", "came\x{300}l"));
ok($Collator->eq("camel", "ca\x{300}me\x{301}l"));
ok($Collator->eq("camel", "Camel"));

$Collator->change(level => 3);

ok($Collator->eq("camel", "came\x{300}l"));
ok($Collator->eq("camel", "ca\x{300}me\x{301}l"));
ok($Collator->lt("camel", "Camel"));

#  secondary: neither 00FF nor 0100 is zero
ok($Collator->eq("camel", "came\x{1B00}l"));
ok($Collator->eq("camel", "came\x{1B01}l"));
ok($Collator->eq("camel", "came\x{1B02}l"));

#  primary: 1900 isn't zero
ok($Collator->lt("\x{3C6}", "\x{3D5}"));
ok($Collator->lt("\x{3D5}", "\x{3A6}"));

# 24

{
    my $s;
    my $txt = "Camel donkey zebra came\x{301}l CAMEL horse cAm\0E\0L.";

    $Collator->change(ignore_level2 => 0, level => 1);

    $s = $txt;
    $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
    ok($s, "=Camel= donkey zebra =came\x{301}l= =CAMEL= horse =cAm\0E\0L=.");

    $Collator->change(level => 2);

    $s = $txt;
    $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
    ok($s, "=Camel= donkey zebra came\x{301}l =CAMEL= horse =cAm\0E\0L=.");

    $Collator->change(level => 3);

    $s = $txt;
    $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
    ok($s, "Camel donkey zebra came\x{301}l CAMEL horse cAm\0E\0L.");

    $Collator->change(ignore_level2 => 1);

    $s = $txt;
    $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
    ok($s, "Camel donkey zebra =came\x{301}l= CAMEL horse cAm\0E\0L.");

    $Collator->change(level => 2);
    $s = $txt;
    $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
    ok($s, "=Camel= donkey zebra =came\x{301}l= =CAMEL= horse =cAm\0E\0L=.");

    $Collator->change(level => 1);
    $s = $txt;
    $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
    ok($s, "=Camel= donkey zebra =came\x{301}l= =CAMEL= horse =cAm\0E\0L=.");

}

# 30

{
    my $c = Unicode::Collate->new(
        table => 'keys.txt', normalization => undef, level => 1,
    );
    my $str = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l...";
    $c->gsubst($str, "camel", sub { "<b>$_[0]</b>" });
    ok($str, "<b>Camel</b> donkey zebra <b>came\x{301}l</b> <b>CAMEL</b> horse <b>cam\0e\0l</b>...");
}

{
    my $c = Unicode::Collate->new(
        table => 'keys.txt', normalization => undef, level => 2,
    );
    my $str = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l...";
    $c->gsubst($str, "camel", sub { "<b>$_[0]</b>" });
    ok($str, "<b>Camel</b> donkey zebra came\x{301}l <b>CAMEL</b> horse <b>cam\0e\0l</b>...");
}

{
    my $c = Unicode::Collate->new(
        table => 'keys.txt', normalization => undef, ignore_level2 => 1,
    );
    my $str = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l...";
    $c->gsubst($str, "camel", sub { "<b>$_[0]</b>" });
    ok($str, "Camel donkey zebra <b>came\x{301}l</b> CAMEL horse <b>cam\0e\0l</b>...");
}

{
    my $c = Unicode::Collate->new(
        table => 'keys.txt', normalization => undef, level => 3,
    );
    my $str = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l...";
    $c->gsubst($str, "camel", sub { "<b>$_[0]</b>" });
    ok($str, "Camel donkey zebra came\x{301}l CAMEL horse <b>cam\0e\0l</b>...");
}

# 34

{
    my $str;
    my $camel = "camel Camel came\x{301}l c-a-m-e-l cam\0e\0l";

    $Collator->change(ignore_level2 => 0);

    $Collator->change(level => 1);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= =Camel= =came\x{301}l= =c-a-m-e-l= =cam\0e\0l=");

    $Collator->change(level => 2);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= =Camel= came\x{301}l =c-a-m-e-l= =cam\0e\0l=");

    $Collator->change(level => 3);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= Camel came\x{301}l =c-a-m-e-l= =cam\0e\0l=");

    $Collator->change(level => 4);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= Camel came\x{301}l c-a-m-e-l =cam\0e\0l=");

    $Collator->change(ignore_level2 => 1);

    $Collator->change(level => 1);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= =Camel= =came\x{301}l= =c-a-m-e-l= =cam\0e\0l=");

    $Collator->change(level => 2);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= =Camel= =came\x{301}l= =c-a-m-e-l= =cam\0e\0l=");

    $Collator->change(level => 3);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= Camel =came\x{301}l= =c-a-m-e-l= =cam\0e\0l=");

    $Collator->change(level => 4);
    $str = $camel;
    $Collator->gsubst($str, "camel", sub { "=$_[0]=" });
    ok($str, "=camel= Camel =came\x{301}l= c-a-m-e-l =cam\0e\0l=");
}

# 42

