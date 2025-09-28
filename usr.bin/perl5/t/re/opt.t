#!./perl
#
# ex: set ts=8 sts=4 sw=4 et:
#
# Here we test for optimizations in the regexp engine.
# We try to distinguish between "nice to have" optimizations and those
# we consider essential: failure of the latter should be considered bugs,
# while failure of the former should at worst be TODO.
#
# Format of data lines is tab-separated: pattern, minlen, anchored, floating,
# other-options, comment.
# - pattern will be subject to string eval as "qr{$pattern}".
# - minlen is a non-negative integer.
# - anchored/floating are of the form "u23:45+string". If initial "u" is
#   present we expect a utf8 substring, else a byte substring; subsequent
#   digits are the min offset; optional /:\d+/ is the max offset (not
#   supported for anchored; assumed undef if not present for floating);
#   subsequent '-' or '+' indicates if this is the substring being checked;
#   "string" is the substring to expect. Use "-" for the whole entry to
#   indicate no substring of this type.
# - other-options is a comma-separated list of bare flags or option=value
#   strings. Those with an initial "T" mark the corresponding test TODO.
#   Booleans (noscan, isall, skip, implicit, anchor SBOL, anchor MBOL,
#   anchor GPOS) are expected false if not mentioned, expected true if
#   supplied as bare flags. stclass may be supplied as a pattern match
#   as eg "stclass=~^ANYOF".
# - as a special-case, minlenret is expected to be the same as minlen
#   unless specified in other-options.
#

use strict;
use warnings;
use 5.010;

$| = 1;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("no dynamic loading on miniperl, no re::optimization");
}

no warnings qw{ experimental };
use feature qw{ refaliasing declared_refs };
our \$TODO = \$::TODO;

use re ();

while (<DATA>) {
    chomp;
    if (m{^\s*(?:#|\z)}) {
        # skip blank/comment lines
        next;
    }
    my($pat, $minlen, $anchored, $floating, $other, $comment) = split /\t/;
    my %todo;
    my %opt = map {
        my($k, $v) = split /=/, $_, 2;
        ($k =~ s/^T//) ? do { $todo{$k} = $v; () } : ($k => $v);
    } split /,/, $other // '';
    $comment = (defined $comment && length $comment)
        ? "$pat ($comment):"
        : "$pat:";

    my $o = re::optimization(eval "qr{$pat}");
    ok($o, "$comment compiled ok");

    my $skip = $o ? undef : "could not get info for qr{$pat}";
    my $test = 0;

    my($got, $expect) = ($o->{minlen}, $minlen);
    if (exists $todo{minlen}) {
        ++$test;
        $skip || ok($got >= $expect, "$comment minlen $got >= $expect");
        my $todo = $todo{minlen};
        local $TODO = 1;
        $skip || is($got, $todo, "$comment minlen $got = $todo");
    } else {
        ++$test;
        $skip || is($got, $expect, "$comment minlen $got = $expect");
    }

    ($got, $expect) = ($o->{minlenret}, $opt{minlenret} // $minlen);
    if (exists $todo{minlenret}) {
        ++$test;
        $skip || ok($got >= $expect, "$comment minlenret $got >= $expect");
        my $todo = $todo{minlenret};
        local $TODO = 1;
        $skip || is($got, $todo, "$comment minlenret $got = $todo");
    } else {
        ++$test;
        $skip || is($got, $expect, "$comment minlenret $got = $expect");
    }

    my($autf, $aoff, $acheck, $astr) = ($anchored =~ m{
        ^ (u?) (\d*) ([-+]) (.*) \z
    }sx) or die "Can't parse anchored test '$anchored'";
    if ($autf eq 'u') {
        ++$test;
        $skip || is($o->{anchored}, undef, "$comment no anchored");
        ++$test;
        local $TODO = 1 if exists $todo{'anchored utf8'};
        $skip || is($o->{'anchored utf8'}, $astr, "$comment got anchored utf8");
    } elsif (length $astr) {
        ++$test;
        $skip || is($o->{anchored_utf8}, undef, "$comment no anchored utf8");
        ++$test;
        local $TODO = 1 if exists $todo{anchored};
        $skip || is($o->{anchored}, $astr, "$comment got anchored");
    } else {
        ++$test;
        $skip || is($o->{anchored}, undef, "$comment no anchored");
        ++$test;
        $skip || is($o->{anchored_utf8}, undef, "$comment no anchored utf8");
    }
    # skip offset checks if we failed to find a string
    my $local_skip = (
        !$skip && !defined($o->{anchored} // $o->{anchored_utf8})
    ) ? 'no anchored string' : undef;
    if (length $aoff) {
        ++$test;
        SKIP: {
            skip($local_skip) if $local_skip;
            local $TODO = 1 if exists $todo{'anchored min offset'};
            $skip || is($o->{'anchored min offset'}, $aoff,
                    "$comment anchored min offset");
        }
        # we don't care about anchored max: it may be set same as min or 0
    }

    my($futf, $fmin, $fmax, $fcheck, $fstr) = ($floating =~ m{
        ^ (u?) (\d*) (?: : (\d*) )? ([-+]) (.*) \z
    }sx) or die "Can't parse floating test '$floating'";
    if ($futf eq 'u') {
        ++$test;
        $skip || is($o->{floating}, undef, "$comment no floating");
        ++$test;
        local $TODO = 1 if exists $todo{'floating utf8'};
        $skip || is($o->{'floating utf8'}, $fstr, "$comment got floating utf8");
    } elsif (length $fstr) {
        ++$test;
        $skip || is($o->{floating_utf8}, undef, "$comment no floating utf8");
        ++$test;
        local $TODO = 1 if exists $todo{floating};
        $skip || is($o->{floating}, $fstr, "$comment got floating");
    } else {
        ++$test;
        $skip || is($o->{floating}, undef, "$comment no floating");
        ++$test;
        $skip || is($o->{floating_utf8}, undef, "$comment no floating utf8");
    }
    # skip offset checks if we failed to find a string
    $local_skip = (
        !$skip && !defined($o->{floating} // $o->{floating_utf8})
    ) ? 'no floating string' : undef;
    if (length $fmin) {
        ++$test;
        SKIP: {
            skip($local_skip) if $local_skip;
            local $TODO = 1 if exists $todo{'floating min offset'};
            $skip || is($o->{'floating min offset'}, $fmin,
                    "$comment floating min offset");
        }
    }
    if (defined $fmax) {
        ++$test;
        SKIP: {
            skip($local_skip) if $local_skip;
            local $TODO = 1 if exists $todo{'floating max offset'};
            $skip || is($o->{'floating max offset'}, $fmax,
                    "$comment floating max offset");
        }
    }

    my $check = ($acheck eq '+') ? 'anchored'
            : ($fcheck eq '+') ? 'floating'
            : ($acheck eq '-') ? undef
            : 'none';
    $local_skip = (
        !$skip && $check && (
            ($check eq 'anchored'
                    && !defined($o->{anchored} // $o->{anchored_utf8}))
            || ($check eq 'floating'
                    && !defined($o->{floating} // $o->{floating_utf8}))
        )
    ) ? "$check not found" : undef;
    if (defined $check) {
        ++$test;
        SKIP: {
            skip($local_skip) if $local_skip;
            local $TODO = 1 if exists $todo{checking};
            $skip || is($o->{checking}, $check, "$comment checking $check");
        }
    }

    # booleans
    for (qw{ noscan isall skip implicit },
        'anchor SBOL', 'anchor MBOL', 'anchor GPOS'
    ) {
        my $got = $o->{$_};
        my $expect = exists($opt{$_}) ? ($opt{$_} // 1) : 0;
        ++$test;
        local $TODO = 1 if exists $todo{"T$_"};
        $skip || is($got, $expect ? 1 : 0, "$comment $_");
    }

    # integer
    for (qw{ gofs }) {
        my $got = $o->{$_};
        my $expect = $opt{$_} // 0;
        ++$test;
        local $TODO = 1 if exists $todo{"T$_"};
        $skip || is($got, $expect || 0, "$comment $_");
    }

    # string
    for (qw{ stclass }) {
        my $got = $o->{$_};
        my $expect = $opt{$_};
        my $qr = (defined($expect) && ($expect =~ s{^~}{})) ? 1 : 0;
        ++$test;
        local $TODO = 1 if exists $todo{"T$_"};
        $skip || ($qr
            ? like($got, qr{$expect}, "$comment $_")
            : is($got, $expect, "$comment $_")
        );
    }

    skip($skip, $test) if $skip;
}
done_testing();
__END__
(?:)	0	-	-	Tisall

# various forms of anchored substring
abc	3	0+abc	-	isall
.{10}abc	13	10+abc	-	-
(?i:)abc	3	0+abc	-	isall
a(?:)bc	3	0+abc	-	isall
a()bc	3	0+abc	-	-
a(?i:)bc	3	0+abc	-	isall
a(b)c	3	0+abc	-	-
a((?i:b))c	3	0+abc	-	Tanchored
a[bB]c	3	0+abc	-	Tanchored
(?=abc)	0	0+abc	-	Tanchored,Tminlen=3,minlenret=0
abc|abc	3	0+abc	-	isall
abcd|abce	4	0+abc	-	-
acde|bcde	4	1+cde	-	Tanchored,stclass=~[ab]
acdef|bcdeg	5	1+cde	-	Tanchored,stclass=~[ab]

# same as above, floating
.?abc	3	-	0:1+abc	-
.?.{10}abc	13	-	10:11+abc	-
.?(?i:)abc	3	-	0:1+abc	-
.?a(?:)bc	3	-	0:1+abc	-
.?a()bc	3	-	0:1+abc	-
.?a(?i:)bc	3	-	0:1+abc	-
.?a(b)c	3	-	0+abc	-
.?a((?i:b))c	3	-	0+abc	Tfloating
.?a[bB]c	3	-	0:1+abc	Tfloating
.?(?=abc)	0	-	0:1+abc	Tfloating,Tminlen=3,minlenret=0
.?(?:abc|abc)	3	-	0:1+abc	-
.?(?:abcd|abce)	4	-	0:1+abc	-
.?(?:acde|bcde)	4	-	1:2+cde	Tfloating
.?(?:acdef|bcdeg)	5	-	1:2+cde	Tfloating

a(b){2,3}c	4	-abb	1+bbc
a(b|bb)c	3	-ab	1-bc	Tfloating,Tfloating min offset
a(b|bb){2}c	4	-abb	1-bbc	Tanchored,Tfloating,Tfloating min offset

abc(*COMMIT)xyz	6	0+abc	-	-
abc(*ACCEPT)xyz	3	0+abc	-	-
# Must not have stclass=[x]
(*ACCEPT)xyz	0	-	-	-
(a(*ACCEPT)){2}	1	0+a	-	-
