
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..91\n"; }
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

our $IsEBCDIC = ord("A") != 0x41;

my $Collator = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
);

##### 1

my %old_level = $Collator->change(level => 2);

my $str;

my $orig = "This is a Perl book.";
my $sub = "PERL";
my $rep = "camel";
my $ret = "This is a camel book.";

$str = $orig;
if (my($pos,$len) = $Collator->index($str, $sub)) {
  substr($str, $pos, $len, $rep);
}

ok($str, $ret);

$Collator->change(%old_level);

$str = $orig;
if (my($pos,$len) = $Collator->index($str, $sub)) {
  substr($str, $pos, $len, $rep);
}

ok($str, $orig);

##### 3

my $match;

$Collator->change(level => 1);

$str = "Pe\x{300}rl";
$sub = "pe";
$ret = "Pe\x{300}";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$str = "P\x{300}e\x{300}\x{301}\x{303}rl";
$sub = "pE";
$ret = "P\x{300}e\x{300}\x{301}\x{303}";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$Collator->change(level => 2);

$str = "Pe\x{300}rl";
$sub = "pe";
$ret = undef;
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$str = "P\x{300}e\x{300}\x{301}\x{303}rl";
$sub = "pE";
$ret = undef;
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$str = "Pe\x{300}rl";
$sub = "pe\x{300}";
$ret = "Pe\x{300}";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$str = "P\x{300}e\x{300}\x{301}\x{303}rl";
$sub = "p\x{300}E\x{300}\x{301}\x{303}";
$ret = "P\x{300}e\x{300}\x{301}\x{303}";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

##### 9

$Collator->change(level => 1);

$str = $IsEBCDIC
    ? "Ich mu\x{0059} studieren Perl."
    : "Ich mu\x{00DF} studieren Perl.";
$sub = $IsEBCDIC
    ? "m\x{00DC}ss"
    : "m\x{00FC}ss";
$ret = $IsEBCDIC
    ? "mu\x{0059}"
    : "mu\x{00DF}";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$Collator->change(%old_level);

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, undef);

$match = undef;
if (my($pos,$len) = $Collator->index("", "")) {
    $match = substr("", $pos, $len);
}
ok($match, "");

$match = undef;
if (my($pos,$len) = $Collator->index("", "abc")) {
    $match = substr("", $pos, $len);
}
ok($match, undef);

##### 13

$Collator->change(level => 1);

$str = "\0\cA\0\cAe\0\x{300}\cA\x{301}\cB\x{302}\0 \0\cA";
$sub = "e";
$ret = "e\0\x{300}\cA\x{301}\cB\x{302}\0";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

$Collator->change(level => 1);

$str = "\0\cA\0\cAe\0\cA\x{300}\0\cAe";
$sub = "e";
$ret = "e\0\cA\x{300}\0\cA";
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);


$Collator->change(%old_level);

$str = "e\x{300}";
$sub = "e";
$ret = undef;
$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub)) {
    $match = substr($str, $pos, $len);
}
ok($match, $ret);

##### 16

$Collator->change(level => 1);

$str = "The Perl is a language, and the perl is an interpreter.";
$sub = "PERL";

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub, -40)) {
    $match = substr($str, $pos, $len);
}
ok($match, "Perl");

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub, 4)) {
    $match = substr($str, $pos, $len);
}
ok($match, "Perl");

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub, 5)) {
    $match = substr($str, $pos, $len);
}
ok($match, "perl");

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub, 32)) {
    $match = substr($str, $pos, $len);
}
ok($match, "perl");

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub, 33)) {
    $match = substr($str, $pos, $len);
}
ok($match, undef);

$match = undef;
if (my($pos, $len) = $Collator->index($str, $sub, 100)) {
    $match = substr($str, $pos, $len);
}
ok($match, undef);

$Collator->change(%old_level);

##### 22

my @ret;

$Collator->change(level => 1);

$ret = $Collator->match("P\cBe\x{300}\cBrl and PERL", "pe");
ok($ret);
ok($$ret eq "P\cBe\x{300}\cB");

@ret = $Collator->match("P\cBe\x{300}\cBrl and PERL", "pe");
ok($ret[0], "P\cBe\x{300}\cB");

$str = $IsEBCDIC ? "mu\x{0059}" : "mu\x{00DF}";
$sub = $IsEBCDIC ? "m\x{00DC}ss" : "m\x{00FC}ss";

($ret) = $Collator->match($str, $sub);
ok($ret, $str);

$str = $IsEBCDIC ? "mu\x{0059}" : "mu\x{00DF}";
$sub = $IsEBCDIC ? "m\x{00DC}s" : "m\x{00FC}s";

($ret) = $Collator->match($str, $sub);
ok($ret, undef);

$ret = join ':', $Collator->gmatch("P\cBe\x{300}\cBrl, perl, and PERL", "pe");
ok($ret eq "P\cBe\x{300}\cB:pe:PE");

$ret = $Collator->gmatch("P\cBe\x{300}\cBrl, perl, and PERL", "pe");
ok($ret == 3);

$str = "ABCDEF";
$sub = "cde";
$ret = $Collator->match($str, $sub);
$str = "01234567";
ok($ret && $$ret, "CDE");

$str = "ABCDEF";
$sub = "cde";
($ret) = $Collator->match($str, $sub);
$str = "01234567";
ok($ret, "CDE");


$Collator->change(level => 3);

$ret = $Collator->match("P\cBe\x{300}\cBrl and PERL", "pe");
ok($ret, undef);

@ret = $Collator->match("P\cBe\x{300}\cBrl and PERL", "pe");
ok(@ret == 0);

$ret = join ':', $Collator->gmatch("P\cBe\x{300}\cBrl and PERL", "pe");
ok($ret eq "");

$ret = $Collator->gmatch("P\cBe\x{300}\cBrl and PERL", "pe");
ok($ret == 0);

$ret = join ':', $Collator->gmatch("P\cBe\x{300}\cBrl, perl, and PERL", "pe");
ok($ret eq "pe");

$ret = $Collator->gmatch("P\cBe\x{300}\cBrl, perl, and PERL", "pe");
ok($ret == 1);

$str = $IsEBCDIC ? "mu\x{0059}" : "mu\x{00DF}";
$sub = $IsEBCDIC ? "m\x{00DC}ss" : "m\x{00FC}ss";

($ret) = $Collator->match($str, $sub);
ok($ret, undef);

$Collator->change(%old_level);

##### 38

$Collator->change(level => 1);

sub strreverse { scalar reverse shift }

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->subst($str, "perl", 'Camel');
ok($ret, 1);
ok($str, "Camel and PERL.");

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->subst($str, "perl", \&strreverse);
ok($ret, 1);
ok($str, "lr\cB\x{300}e\cBP and PERL.");

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->gsubst($str, "perl", 'Camel');
ok($ret, 2);
ok($str, "Camel and Camel.");

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->gsubst($str, "perl", \&strreverse);
ok($ret, 2);
ok($str, "lr\cB\x{300}e\cBP and LREP.");

$str = "Camel donkey zebra came\x{301}l CAMEL horse cAm\0E\0L...";
$Collator->gsubst($str, "camel", sub { "<b>$_[0]</b>" });
ok($str, "<b>Camel</b> donkey zebra <b>came\x{301}l</b> "
	. "<b>CAMEL</b> horse <b>cAm\0E\0L</b>...");

##### 47

# http://www.xray.mpe.mpg.de/mailing-lists/perl-unicode/2010-09/msg00014.html
# when the substring includes an ignorable element like a space...

$str = "Camel donkey zebra came\x{301}l CAMEL horse cAm\0E\0L...";
$Collator->gsubst($str, "camel horse", sub { "<b>$_[0]</b>" });
ok($str, "Camel donkey zebra came\x{301}l <b>CAMEL horse</b> cAm\0E\0L...");

$str = "Camel donkey zebra camex{301}l CAMEL horse cAmEL-horse...";
$Collator->gsubst($str, "camel horse", sub { "=$_[0]=" });
ok($str, "Camel donkey zebra camex{301}l =CAMEL horse= =cAmEL-horse=...");

$str = "Camel donkey zebra camex{301}l CAMEL horse cAmEL-horse...";
$Collator->gsubst($str, "camel-horse", sub { "=$_[0]=" });
ok($str, "Camel donkey zebra camex{301}l =CAMEL horse= =cAmEL-horse=...");

$str = "Camel donkey zebra camex{301}l CAMEL horse cAmEL-horse...";
$Collator->gsubst($str, "camelhorse", sub { "=$_[0]=" });
ok($str, "Camel donkey zebra camex{301}l =CAMEL horse= =cAmEL-horse=...");

$str = "Camel donkey zebra camex{301}l CAMEL horse cAmEL-horse...";
$Collator->gsubst($str, "  ca  mel  hor  se  ", sub { "=$_[0]=" });
ok($str, "Camel donkey zebra camex{301}l =CAMEL horse= =cAmEL-horse=...");

$str = "Camel donkey zebra camex{301}l CAMEL horse cAmEL-horse...";
$Collator->gsubst($str, "ca\x{300}melho\x{302}rse", sub { "=$_[0]=" });
ok($str, "Camel donkey zebra camex{301}l =CAMEL horse= =cAmEL-horse=...");

##### 53

$Collator->change(level => 3);

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->subst($str, "perl", "Camel");
ok(! $ret);
ok($str, "P\cBe\x{300}\cBrl and PERL.");

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->subst($str, "perl", \&strreverse);
ok(! $ret);
ok($str, "P\cBe\x{300}\cBrl and PERL.");

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->gsubst($str, "perl", "Camel");
ok($ret, 0);
ok($str, "P\cBe\x{300}\cBrl and PERL.");

$str = "P\cBe\x{300}\cBrl and PERL.";
$ret = $Collator->gsubst($str, "perl", \&strreverse);
ok($ret, 0);
ok($str, "P\cBe\x{300}\cBrl and PERL.");

$Collator->change(%old_level);

##### 61

$str = "Perl and Camel";
$ret = $Collator->gsubst($str, "\cA\cA\0", "AB");
ok($ret, 15);
ok($str, "ABPABeABrABlAB ABaABnABdAB ABCABaABmABeABlAB");

$str = '';
$ret = $Collator->subst($str, "", "ABC");
ok($ret, 1);
ok($str, "ABC");

$str = '';
$ret = $Collator->gsubst($str, "", "ABC");
ok($ret, 1);
ok($str, "ABC");

$str = 'PPPPP';
$ret = $Collator->gsubst($str, 'PP', "ABC");
ok($ret, 2);
ok($str, "ABCABCP");

##### 69

# Shifted; ignorable after variable

($ret) = $Collator->match("A?\x{300}!\x{301}\x{344}B\x{315}", "?!");
ok($ret, "?\x{300}!\x{301}\x{344}");

$Collator->change(alternate => 'Non-ignorable');

($ret) = $Collator->match("A?\x{300}!\x{301}B\x{315}", "?!");
ok($ret, undef);

##### 71

# Now preprocess is defined.

$Collator->change(preprocess => sub {''});

eval { $Collator->index("", "") };
ok($@ && $@ =~ /Don't use Preprocess with index\(\)/);

eval { $Collator->index("a", "a") };
ok($@ && $@ =~ /Don't use Preprocess with index\(\)/);

eval { $Collator->match("", "") };
ok($@ && $@ =~ /Don't use Preprocess with.*match\(\)/);

eval { $Collator->match("a", "a") };
ok($@ && $@ =~ /Don't use Preprocess with.*match\(\)/);

$Collator->change(preprocess => sub { uc shift });

eval { $Collator->index("", "") };
ok($@ && $@ =~ /Don't use Preprocess with index\(\)/);

eval { $Collator->index("a", "a") };
ok($@ && $@ =~ /Don't use Preprocess with index\(\)/);

eval { $Collator->match("", "") };
ok($@ && $@ =~ /Don't use Preprocess with.*match\(\)/);

eval { $Collator->match("a", "a") };
ok($@ && $@ =~ /Don't use Preprocess with.*match\(\)/);

##### 79

eval { require Unicode::Normalize };
my $has_norm = !$@;

if ($has_norm) {
    # Now preprocess and normalization are defined.

    $Collator->change(normalization => 'NFD');

    eval { $Collator->index("", "") };
    ok($@ && $@ =~ /Don't use Preprocess with index\(\)/);

    eval { $Collator->index("a", "a") };
    ok($@ && $@ =~ /Don't use Preprocess with index\(\)/);

    eval { $Collator->match("", "") };
    ok($@ && $@ =~ /Don't use Preprocess with.*match\(\)/);

    eval { $Collator->match("a", "a") };
    ok($@ && $@ =~ /Don't use Preprocess with.*match\(\)/);
} else {
    ok(1) for 1..4;
}

$Collator->change(preprocess => undef);

if ($has_norm) {
    # Now only normalization is defined.

    eval { $Collator->index("", "") };
    ok($@ && $@ =~ /Don't use Normalization with index\(\)/);

    eval { $Collator->index("a", "a") };
    ok($@ && $@ =~ /Don't use Normalization with index\(\)/);

    eval { $Collator->match("", "") };
    ok($@ && $@ =~ /Don't use Normalization with.*match\(\)/);

    eval { $Collator->match("a", "a") };
    ok($@ && $@ =~ /Don't use Normalization with.*match\(\)/);

    $Collator->change(normalization => undef);
} else {
    ok(1) for 1..4;
}

##### 87

# Now preprocess and normalization are undef.

eval { $Collator->index("", "") };
ok(!$@);

eval { $Collator->index("a", "a") };
ok(!$@);

eval { $Collator->match("", "") };
ok(!$@);

eval { $Collator->match("a", "a") };
ok(!$@);

##### 91
