use strict;
use warnings;
use File::Spec;
use Test::More;

use Module::Metadata;

if ("$]" < 5.008_003) {
  plan skip_all => 'Encoding test needs at least perl 5.8.3';
}

my %versions = (
 UTF8    => 3,
 UTF16BE => 4,
 UTF16LE => 5,
);

plan tests => 4 * scalar(keys %versions);

for my $enc (sort keys %versions) {
  my $pkg  = "BOMTest::$enc";
  my $vers = $versions{$enc};
  my $pm   = File::Spec->catfile(qw<corpus BOMTest> => "$enc.pm");
  my $info = Module::Metadata->new_from_file($pm);
  is( $info->name, $pkg, "$enc: default package was found" );
  is( $info->version, $vers, "$enc: version for default package" );
  is( $info->version('Heart'), '1', 'version for ASCII package' );
  is( $info->version("C\x{153}ur"), '2', 'version for Unicode package' );
}
