use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Wrap;

print "1..3\n";

my $w; $SIG{'__WARN__'} = sub { $w = join '', @_ };

$Text::Wrap::columns = 4;
my $x;
my $ok = eval { $x = wrap('', '123', 'some text'); 1 };
ok( $ok, 'no exception thrown' );
ok( defined $x && $x eq "some\n123t\n123e\n123x\n123t", 'expected wrapping returned' );
ok( defined $w && $w =~ /^Increasing \$Text::Wrap::columns from 4 to 8 to accommodate length of subsequent tab at ${\quotemeta __FILE__} line [0-9]+/,
	'warning about increase of $columns' );
