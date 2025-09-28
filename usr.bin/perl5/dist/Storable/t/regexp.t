#!perl -w
use strict;
use Storable "dclone";
use Test::More;

my $version = int(($]-5)*1000);

$version >= 8
  or plan skip_all => "regexps not supported before 5.8";

my @tests;
while (<DATA>) {
    chomp;
    next if /^\s*#/ || !/\S/;
    my ($range, $code, $match, $name) = split /\s*;\s*/;
    defined $name or die "Bad test line";
    my $ascii_only = $range =~ s/A//;
    next if $ascii_only and ord("A") != 65;
    if ($range =~ /^(\d+)-$/) {
        next if $version < $1
    }
    elsif ($range =~ /^-(\d+)$/) {
        next if $version > $1
    }
    elsif ($range =~ /^(\d+)-(\d+)$/) {
        next if $version < $1 || $version > $2;
    }
    elsif ($range ne "-") {
        die "Invalid version range $range for $name";
    }
    my @match = split /\s*,\s*/, $match;
    for my $m (@match) {
	my $not = $m =~ s/^!//;
	my $cmatch = eval $m;
	die if $@;
        push @tests, [ $code, $not, $cmatch, $m, $name ];
    }
}

plan tests => 10 + 3*scalar(@tests);

SKIP:
{
    $version >= 14 && $version < 20
      or skip "p introduced in 5.14, pointless from 5.20", 4;
    my $q1 = eval "qr/b/p";
    my $q2 = eval "qr/b/";
    my $c1 = dclone($q1);
    my $c2 = dclone($q2);
    ok("abc" =~ $c1, "abc matches $c1");
    is(${^PREMATCH}, "a", "check p worked");
    ok("cba" =~ $c2, "cba matches $c2");
    isnt(${^PREMATCH}, "c", "check no p worked");
}

SKIP:
{
    $version >= 24
      or skip "n introduced in 5.22", 4;
    my $c1 = dclone(eval "qr/(\\w)/");
    my $c2 = dclone(eval "qr/(\\w)/n");
    ok("a" =~ $c1, "a matches $c1");
    is($1, "a", "check capturing preserved");
    ok("b" =~ $c2, "b matches $c2");
    isnt($1, "b", "check non-capturing preserved");
}

SKIP:
{
    $version >= 8
      or skip "Cannot retrieve before 5.8", 1;
    my $x;
    my $re = qr/a(?{ $x = 1 })/;
    use re 'eval';
    ok(!eval { dclone($re) }, "should fail to clone, even with use re 'eval'");
}

is(ref(dclone(bless qr//, "Foo")), "Foo", "check reblessed regexps");

for my $test (@tests) {
    my ($code, $not, $match, $matchc, $name) = @$test;
    my $qr = eval $code;
    die "Could not compile $code: $@" if $@;
    if ($not) {
	unlike($match, $qr, "$name: pre(not) match $matchc");
    }
    else {
	like($match, $qr, "$name: prematch $matchc");
    }
    my $qr2 = dclone($qr);
    if ($not) {
	unlike($match, $qr2, "$name: (not) match $matchc");
    }
    else {
	like($match, $qr2, "$name: match $matchc");
    }

    # this is unlikely to be a problem, but make sure regexps are frozen sanely
    # as part of a data structure
    my $a2 = dclone([ $qr ]);
    if ($not) {
	unlike($match, $a2->[0], "$name: (not) match $matchc (array)");
    }
    else {
	like($match, $a2->[0], "$name: match $matchc (array)");
    }
}

__DATA__
# semi-colon separated:
# perl version range; regexp qr; match string; name
# - version range is PERL_VERSION, ie 22 for 5.22 as from-to with both from
#   and to optional (so "-" is all versions.
# - match string is , separated match strings
# - if a match string starts with ! it mustn't match, otherwise it must
#   spaces around the commas ignored.
#   The initial "!" is stripped and the remainder treated as perl code to define
#   the string to (not) be matched
-; qr/foo/ ; "foo",!"fob" ; simple
-; qr/foo/i ; "foo","FOO",!"fob" ; simple case insensitive
-; qr/f o o/x ; "foo", !"f o o" ; /x
-; qr(a/b) ; "a/b" ; alt quotes
A-; qr(\x2E) ; ".", !"a" ; \x2E - hex meta
-; qr/\./ ; "." , !"a" ; \. - backslash meta
8- ; qr/\x{100}/ ; "\x{100}" ; simple unicode
A12- ; qr/fss/i ; "f\xDF\x{101}" ; case insensive unicode promoted
A22-; qr/fss/ui ; "f\xDF" ; case insensitive unicode SS /iu
A22-; qr/fss/aai ; !"f\xDF" ; case insensitive unicode SS /iaa
A22-; qr/f\w/a ; "fo", !"f\xff" ; simple /a flag
