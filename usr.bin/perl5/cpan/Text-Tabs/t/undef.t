use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Tabs;
use Text::Wrap;

print "1..4\n";

my $w; $SIG{'__WARN__'} = sub { $w = join '', @_ };

sub cleanup { diag $w; undef $w }

expand( undef );
ok( !defined $w, 'expand accepts undef silently' ) or cleanup;

unexpand( undef, undef, undef );
ok( !defined $w, 'unexpand accepts undef silently' ) or cleanup;

wrap( undef, undef, undef, ( ( "abc " x 20 ) . "abc\n" ) x 5, undef );
ok( !defined $w, 'wrap accepts undef silently' ) or cleanup;

fill( undef, undef, undef, ( ( "abc " x 20 ) . "abc\n" ) x 5, undef );
ok( !defined $w, 'fill accepts undef silently' ) or cleanup;
