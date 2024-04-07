#!./perl

BEGIN {
    require Config;
    if (($Config::Config{'extensions'} !~ /\bre\b/) ){
        print "1..0 # Skip -- Perl configured without re module\n";
        exit 0;
    }
}

use strict;
use warnings;

use Test::More; # test count at bottom of file
{
    use re qw{regmust};
    my $qr=qr/here .* there/x;
    my ($anchored,$floating)=regmust($qr);
    is($anchored,'here',"Regmust anchored - qr//");
    is($floating,'there',"Regmust floating - qr//");
    my $foo='blah';
    ($anchored,$floating)=regmust($foo);
    is($anchored,undef,"Regmust anchored - non ref");
    is($floating,undef,"Regmust anchored - non ref");
    my $bar=['blah'];
    ($anchored,$floating)=regmust($foo);
    is($anchored,undef,"Regmust anchored - ref");
    is($floating,undef,"Regmust anchored - ref");
}

{
    use re qw{optimization};
    # try to show each element is populated, without working the regexp
    # engine any harder than necessary - the real work will be testing
    # that optimization happens correctly using this under t/re.

    is(optimization(undef), undef, "non-qr returns undef");
    is(optimization("foo"), undef, "non-qr returns undef");
    is(optimization(bless {}, "Regexp"), undef, "non-qr returns undef");

    my $o = optimization(qr{foo});
    is(ref($o), 'HASH', "qr returns a hashref");
    is($o->{minlen}, 3, "/foo/ has minlen");

    $o = optimization(qr{foo(?=bar)});
    is($o->{minlenret}, 3, "/foo(?=bar)/ has minlenret");

    $o = optimization(qr{.\G.});
    ok($o->{'anchor GPOS'}, "/.\\G./ has anchor GPOS");
    is($o->{gofs}, 1, "/.\\G./ has gofs");

    $o = optimization(qr{a|bc});
    is($o->{anchored}, undef, "/a|bc/ has no anchored substring");
    is($o->{floating}, undef, "/a|bc/ has no floating substring");
    is($o->{checking}, "none", "/a|bc/ is checking no substring");

    $o = optimization(qr{foo});
    ok($o->{isall}, "/foo/ has isall");
    is($o->{anchored}, "foo", "/foo/ has anchored substring");
    is($o->{'anchored utf8'}, undef, "/foo/ has no anchored utf8");
    is($o->{floating}, undef, "/foo/ has no floating substring");
    is($o->{checking}, "anchored", "/foo/ is checking anchored");

    $o = optimization(qr{.foo});
    is($o->{'anchored min offset'}, 1, "/.foo/ has anchored min offset");
    like($o->{'anchored max offset'}, qr{^[01]\z},
            "/.foo/ has valid anchored max offset");

    $o = optimization(qr{.foo\x{100}});
    is($o->{anchored}, undef, "/.foo\\x{100}/ has no anchored");
    is($o->{'anchored utf8'}, "foo\x{100}", "/.foo\\x{100}/ has anchored utf8");
    is($o->{'anchored min offset'}, 1, "/.foo\\x{100}/ has anchored min");
    like($o->{'anchored max offset'}, qr{^[01]\z},
            "/.foo\\x{100}/ has valid anchored max offset");

    $o = optimization(qr{.x?foo});
    is($o->{anchored}, undef, "/.x?foo/ has no anchored substring");
    is($o->{floating}, "foo", "/.x?foo/ has floating substring");
    is($o->{'floating utf8'}, undef, "/.x?foo/ has no floating utf8");
    is($o->{'floating min offset'}, 1, "/.x?foo/ has floating min offset");
    is($o->{'floating max offset'}, 2, "/.x?foo/ has floating max offset");
    is($o->{checking}, "floating", "/foo/ is checking floating");

    $o = optimization(qr{[ab]+});
    ok($o->{skip}, "/[ab]+/ has skip");
    like($o->{stclass}, qr{^ANYOF}, "/[ab]+/ has stclass");

    ok(optimization(qr{^foo})->{'anchor SBOL'}, "/^foo/ has anchor SBOL");
    ok(optimization(qr{^foo}m)->{'anchor MBOL'}, "/^foo/m has anchor MBOL");
    ok(optimization(qr{.*})->{implicit}, "/.*/ has implicit anchor");
    ok(optimization(qr{^.foo})->{noscan}, "/^.foo/ has noscan");

    # TODO: test anchored/floating end shift
}
# New tests above this line, don't forget to update the test count below!
use Test::More tests => 40;
# No tests here!

#
# ex: set ts=8 sts=4 sw=4 et:
#
