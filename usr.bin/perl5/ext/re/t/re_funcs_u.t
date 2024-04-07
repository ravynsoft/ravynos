#!./perl

BEGIN {
    require Config;
    if (($Config::Config{'extensions'} !~ /\bre\b/) ){
	print "1..0 # Skip -- Perl configured without re module\n";
	    exit 0;
    }
    require 'test.pl';      # For watchdog
    require 'loc_tools.pl'; # To see if platform has locales
}

use strict;
use warnings;

use re qw(is_regexp regexp_pattern
          regname regnames regnames_count);
{
    use feature 'unicode_strings';  # Force 'u' pat mod
    my $qr=qr/foo/pi;
    no feature 'unicode_strings';
    my $rx = $$qr;

    ok(is_regexp($qr),'is_regexp(REGEXP ref)');
    ok(is_regexp($rx),'is_regexp(REGEXP)');
    ok(!is_regexp(''),'is_regexp("")');

    is((regexp_pattern($qr))[0],'foo','regexp_pattern[0] (ref)');
    is((regexp_pattern($qr))[1],'uip','regexp_pattern[1] (ref)');
    is(regexp_pattern($qr),'(?^upi:foo)','scalar regexp_pattern (ref)');

    is((regexp_pattern($rx))[0],'foo','regexp_pattern[0] (bare REGEXP)');
    is((regexp_pattern($rx))[1],'uip','regexp_pattern[1] (bare REGEXP)');
    is(regexp_pattern($rx),'(?^upi:foo)', 'scalar regexp_pattern (bare REGEXP)');

    ok(!regexp_pattern(''),'!regexp_pattern("")');
    is +()=regexp_pattern(''), 0, 'regexp_pattern("") in list cx';
}

if ('1234'=~/(?:(?<A>\d)|(?<C>!))(?<B>\d)(?<A>\d)(?<B>\d)/){
    my @names = sort +regnames();
    is("@names","A B","regnames");
    @names = sort +regnames(0);
    is("@names","A B","regnames");
    my $names = regnames();
    ok(($names eq  "B" || $names eq "A"), "regnames in scalar context");
    @names = sort +regnames(1);
    is("@names","A B C","regnames");
    is(join("", @{regname("A",1)}),"13");
    is(join("", @{regname("B",1)}),"24");
    {
        if ('foobar'=~/(?<foo>foo)(?<bar>bar)/) {
            is(regnames_count(),2);
        } else {
            ok(0); ok(0);
        }
    }
    is(regnames_count(),3);
}

{
    my ($pat, $mods);
    $|=1;

    my $re = qr/a/d;
    ($pat, $mods) = regexp_pattern($re);
    is($mods, "", "Verify /d results in default mod");
    $re = qr/a/u;
    ($pat, $mods) = regexp_pattern($re);
    is($mods, "u", "Verify /u is understood");
    $re = qr/a/l;
    ($pat, $mods) = regexp_pattern($re);
    is($mods, "l", "Verify /l is understood");
    $re = qr/a/a;
    ($pat, $mods) = regexp_pattern($re);
    is($mods, "a", "Verify /a is understood");
    $re = qr/a/aa;
    ($pat, $mods) = regexp_pattern($re);
    is($mods, "aa", "Verify /aa is understood");
}

{
    # tests for new regexp flags
    my $text = chr utf8::unicode_to_native(0xE4);
    my $check;

    {
        # check u/d-flag without setting a locale
        $check = $text =~ /(?u)\w/;
        ok( $check );
        $check = $text =~ /(?d)\w/;
        ok( !$check );
    }

    SKIP: {
        skip 'No locales available', 3 unless locales_enabled('LC_CTYPE');
        require POSIX;
        my $current_locale = POSIX::setlocale( &POSIX::LC_CTYPE, 'de_DE.ISO-8859-1' );
        if ( !$current_locale || $current_locale ne 'de_DE.ISO-8859-1' ) {
            skip( 'cannot use locale de_DE.ISO-8859-1', 3 );
        }

        $check = $text =~ /(?u)\w/;
        ok( $check );
        $check = $text =~ /(?d)\w/;
        ok( !$check );
        $check = $text =~ /(?l)\w/;
        ok( $check );
    }

    SKIP: {
        skip 'No locales available', 3 unless locales_enabled('LC_CTYPE');
        require POSIX;
        my $current_locale = POSIX::setlocale( &POSIX::LC_CTYPE, 'C' );
        if ( !$current_locale || $current_locale ne 'C' ) {
            skip( 'cannot set locale C', 3 );
        }

        $check = $text =~ /(?u)\w/;
        ok( $check );
        $check = $text =~ /(?d)\w/;
        ok( !$check );
        $check = $text =~ /(?l)\w/;
        ok( !$check );
    }
}

# New tests go here ^^^

    { # Keep these tests last, as whole script will be interrupted if times out
        # Bug #72998; this can loop 
        watchdog(10);
        eval '"\x{100}\x{FB00}" =~ /\x{100}\N{U+66}+/i';
        pass("Didn't loop");

        # Bug #78058; this can loop
        no warnings;    # Because the 8 may be warned on
        eval 'qr/\18/';
        pass(q"qr/\18/ didn't loop");
    }

done_testing();

__END__
# New tests go up there^^^
