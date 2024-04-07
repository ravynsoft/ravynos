
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..17\n"; }
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

my $code = sub {
    my $line = shift;
    $line =~ s/\[\.0000\..{4}\..{4}([.\]])/[.0000.0000.0000$1/g;
    return $line;
  };

#####

my $Collator = Unicode::Collate->new(
  table => 'keys.txt', normalization => undef, rewrite => $code,
);

ok($Collator->eq("camel", "came\x{300}l"));
ok($Collator->eq("camel", "ca\x{300}me\x{301}l"));
ok($Collator->lt("camel", "Camel"));
{
  my $s = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l.";
  $Collator->gsubst($s, "camel", sub { "=$_[0]=" });
  ok($s, "Camel donkey zebra =came\x{301}l= CAMEL horse =cam\0e\0l=.");
}

# 5

my $rewriteDUCET = Unicode::Collate->new(
  normalization => undef, rewrite => $code,
);

ok($rewriteDUCET->eq("camel", "came\x{300}l"));
ok($rewriteDUCET->eq("camel", "ca\x{300}me\x{301}l"));
ok($rewriteDUCET->lt("camel", "Camel"));
{
  my $s = "Camel donkey zebra came\x{301}l CAMEL horse cam\0e\0l.";
  $rewriteDUCET->gsubst($s, "camel", sub { "=$_[0]=" });
  ok($s, "Camel donkey zebra =came\x{301}l= CAMEL horse =cam\0e\0l=.");
}

# 9

my $undef_hira = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  level => 1,
  rewrite => sub {
    my $line = shift;
    return '' if $line =~ /HIRAGANA/;
    return $line;
  },
);

my $hiragana = "\x{3042}\x{3044}";
my $katakana = "\x{30A2}\x{30A4}";
my $cjkkanji = "\x{4E00}";

# HIRAGANA are undefined via rewrite
# So they are after CJK Unified Ideographs.

ok($undef_hira->lt("abc", "perl"));
ok($undef_hira->lt("", "ABC"));
ok($undef_hira->lt($katakana, $hiragana));
ok($undef_hira->lt($katakana, $cjkkanji));
ok($undef_hira->lt($cjkkanji, $hiragana));

$Collator->change(level => 1);
ok($Collator->eq($katakana, $hiragana));
ok($Collator->lt($katakana, $cjkkanji));
ok($Collator->gt($cjkkanji, $hiragana));

# 17
