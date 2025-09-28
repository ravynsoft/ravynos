use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;
use CPAN::Meta::Validator;
use File::Spec;
use IO::Dir;
use Parse::CPAN::Meta;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

{
  my @data_dirs = qw( t/data-test t/data-valid );
  my @files = sort map {
        my $d = $_;
        map { "$d/$_" } grep { substr($_,0,1) ne '.' } IO::Dir->new($d)->read
  } @data_dirs;

  for my $f ( @files ) {
    my $meta = Parse::CPAN::Meta->load_file( File::Spec->catfile($f) );
    my $cmv = CPAN::Meta::Validator->new({%$meta});
    ok( $cmv->is_valid, "$f validates" )
      or diag( "ERRORS:\n" . join( "\n", $cmv->errors ) );
  }
}

{
  my @data_dirs = qw( t/data-fail t/data-fixable );
  my @files = sort map {
        my $d = $_;
        map { "$d/$_" } grep { substr($_,0,1) ne '.' } IO::Dir->new($d)->read
  } @data_dirs;

  for my $f ( @files ) {
    my $meta = Parse::CPAN::Meta->load_file( File::Spec->catfile($f) );
    my $cmv = CPAN::Meta::Validator->new({%$meta});
    ok( ! $cmv->is_valid, "$f shouldn't validate" );
    note 'validation error: ', $_ foreach $cmv->errors;
  }
}

done_testing;
# vim: ts=2 sts=2 sw=2 et :
