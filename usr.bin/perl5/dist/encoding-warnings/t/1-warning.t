#!/usr/bin/perl
# $File: /member/local/autrijus/encoding-warnings//t/1-warning.t $ $Author: autrijus $
# $Revision: #5 $ $Change: 6145 $ $DateTime: 2004-07-16T03:49:06.717424Z $

BEGIN {
    if (ord("A") != 65) {
        print "1..0 # Skip: Encode not working on EBCDIC\n";
        exit 0;
    }
    unless (eval { require Encode } ) {
        print "1..0 # Skip: no Encode\n";
        exit 0;
    }
}

use strict;
use warnings;
use Test::More;

BEGIN {
    if ("$]" >= 5.025) {
        # Test the new almost-noop behaviour in new perls.
        plan tests => 3;
        my $w;
        $SIG{__WARN__} = sub { $w .= shift };
        require encoding::warnings;
        is $w, undef, 'no warning from requiring encoding::warnings';
        ok(encoding::warnings->VERSION);
        encoding::warnings->import;
        like $w, qr/^encoding::warnings is not supported /, 'import warning';
        exit;
    }
    # else continue with your usual scheduled testing...
    plan tests => 2;
}

use encoding::warnings;
ok(encoding::warnings->VERSION);

if ($] < 5.008) {
    ok(1);
    exit;
}

my ($a, $b, $c, $ok);

$SIG{__WARN__} = sub {
    if ($_[0] =~ /upgraded/) { ok(1); exit }
};

utf8::encode($a = chr(20000));
$b = chr(20000);
$c = $a . $b;

ok($ok);

__END__
