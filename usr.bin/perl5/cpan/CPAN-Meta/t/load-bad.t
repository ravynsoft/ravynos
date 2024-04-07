use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;
use File::Spec;
use IO::Dir;

sub _slurp { do { local(@ARGV,$/)=shift(@_); <> } }

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my $data_dir = IO::Dir->new( 't/data-fixable' );
my @files = sort grep { /^\w/ } $data_dir->read;

for my $f ( sort @files ) {
  my $path = File::Spec->catfile('t','data-fixable',$f);
  ok( eval { CPAN::Meta->load_file( $path ) }, "load_file('$f')" ) or diag $@;
  my $string = _slurp($path);
  my $method =  $path =~ /\.json/ ? "load_json_string" : "load_yaml_string";
  ok( eval { CPAN::Meta->$method( $string, { fix_errors => 1 } ) }, "$method(slurp('$f'))" ) or diag $@;
}

done_testing;
# vim: ts=2 sts=2 sw=2 et:
