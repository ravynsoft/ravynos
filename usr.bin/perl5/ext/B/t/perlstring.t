#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

$|  = 1;
use warnings;
use strict;
BEGIN  {
    eval { require threads; threads->import; }
}
use Test::More;

BEGIN { use_ok( 'B' ); }

for my $do_utf8 (""," utf8") {
    my $max = $do_utf8 ? 1024  : 255;
    my @bad;
    for my $cp ( 0 .. $max ) {
        my $char= chr($cp);
        utf8::upgrade($char);
        my $escaped= B::perlstring($char);
        my $evalled= eval $escaped;
        push @bad, [ $cp, $evalled, $char, $escaped ] if $evalled ne $char;
    }
    is(0+@bad, 0, "Check if any$do_utf8 codepoints fail to round trip through B::perlstring()");
    if (@bad) {
        foreach my $tuple (@bad) {
            my ( $cp, $evalled, $char, $escaped ) = @$tuple;
            is($evalled, $char, "check if B::perlstring of$do_utf8 codepoint $cp round trips ($escaped)");
        }
    }
}

done_testing();
