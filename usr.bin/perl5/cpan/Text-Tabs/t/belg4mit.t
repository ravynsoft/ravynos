use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Wrap;

print "1..2\n";

my $w; $SIG{'__WARN__'} = sub { $w = join '', @_ };

$Text::Wrap::columns = 1;
my $ok = eval { wrap('', '', <<''); 1 };
H4sICNoBwDoAA3NpZwA9jbsNwDAIRHumuC4NklvXTOD0KSJEnwU8fHz4Q8M9i3sGzkS7BBrm
OkCTwsycb4S3DloZuMIYeXpLFqw5LaMhXC2ymhreVXNWMw9YGuAYdfmAbwomoPSyFJuFn2x8
Opr8bBBidccAAAA

ok( $ok, 'no exception thrown' ) or diag( $@ );
ok( defined $w && $w =~ /^Increasing \$Text::Wrap::columns from 1 to 2 at ${\quotemeta __FILE__} line [0-9]+/,
	'warning about increase of $columns' );
