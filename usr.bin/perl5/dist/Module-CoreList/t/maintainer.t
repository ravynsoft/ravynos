use strict;
use warnings;
use Test::More;

plan skip_all => 'These tests only run in core'
  unless $ENV{PERL_CORE};

my @mods = qw[
Module::CoreList
Module::CoreList::Utils
];

plan tests => 3 + scalar @mods;

my %vers;

foreach my $mod ( @mods ) {
  use_ok($mod);
  $vers{ $mod->VERSION }++;
}

is( scalar keys %vers, 1, 'All Module-CoreList modules should have the same $VERSION' );

# Check that there is a release entry for the current perl version
my $released = $Module::CoreList::released{ $] };
# duplicate fetch to avoid 'used only once: possible typo' warning
$released = $Module::CoreList::released{ $] };

ok( defined $released, "There is a released entry for $]" );
like( $released, qr!^\d{4}\-\d{2}\-\d{2}$!, 'It should be a date in YYYY-MM-DD format' );
