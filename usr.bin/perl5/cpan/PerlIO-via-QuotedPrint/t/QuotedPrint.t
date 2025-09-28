use 5.008001;

use strict;
use warnings;

BEGIN {                         # Magic Perl CORE pragma
    unless (find PerlIO::Layer 'perlio') {
        print "1..0 # Skip: PerlIO not used\n";
        exit 0;
    }
    if (ord("A") == 193) {
        print "1..0 # Skip: EBCDIC\n";
        exit 0;
    }
}

use Test::More tests => 11;

BEGIN { use_ok('PerlIO::via::QuotedPrint') }

my $file = 'test.qp';

my $decoded = <<EOD;
This is a tést for quoted-printable text that has hàrdly any speçial characters
in it.
EOD

my $encoded = <<EOD;
This is a t=E9st for quoted-printable text that has h=E0rdly any spe=E7ial =
characters
in it.
EOD

# Create the encoded test-file

ok(
 open( my $out,'>:via(PerlIO::via::QuotedPrint)', $file ),
 "opening '$file' for writing"
);

ok( (print $out $decoded),              'print to file' );
ok( close( $out ),                      'closing encoding handle' );

# Check encoding without layers

{
local $/ = undef;
ok( open( my $test, '<', $file ),       'opening without layer' );
is( $encoded,readline( $test ),         'check encoded content' );
ok( close( $test ),                     'close test handle' );
}

# Check decoding _with_ layers

ok(
 open( my $in,'<:via(QuotedPrint)', $file ),
 "opening '$file' for reading"
);
is( $decoded,join( '',<$in> ),          'check decoding' );
ok( close( $in ),                       'close decoding handle' );

# Remove whatever we created now

ok( unlink( $file ),                    "remove test file '$file'" );
1 while unlink $file; # multiversioned filesystems
