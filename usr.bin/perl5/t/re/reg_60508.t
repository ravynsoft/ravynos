#!./perl

# This is a test for [perl #60508] which I can't figure out where else
# to put it or what the underlying problem is, but it has to go somewhere.
# --Schwern

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use utf8;
plan tests => 1;

{
    my $expect = <<"EXPECT";
k1 = ....
k2.1 = >\x{2022}
k2.2 = \x{2022}
EXPECT
    utf8::encode($expect);

    #local $TODO = "[perl #60508]";

    fresh_perl_is(<<'CODE', $expect, {});
binmode STDOUT, ":utf8";
sub f { $_[0] =~ s/([>X])//g; }

$k1 = "." x 4 . ">>";
f($k1);
print "k1 = $k1\n";

$k2 = "\x{f1}\x{2022}";
$k2 =~ s/([\360-\362])/>/g;
print "k2.1 = $k2\n";
f($k2);
print "k2.2 = $k2\n";
CODE
}
