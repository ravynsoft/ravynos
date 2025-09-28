use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;
use CPAN::Meta::Validator;
use CPAN::Meta::Converter;
use File::Spec;
use IO::Dir;
use Parse::CPAN::Meta;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my @data_dirs = qw( t/data-valid t/data-fixable );
my @files = sort map {
  my $d = $_;
  map { "$d/$_" } grep { substr($_,0,1) ne '.' } IO::Dir->new($d)->read
} @data_dirs;

*_spec_version = \&CPAN::Meta::Converter::_extract_spec_version;

#use Data::Dumper;

for my $f ( reverse sort @files ) {
  my $path = File::Spec->catfile($f);
  my $original = Parse::CPAN::Meta->load_file( $path  );
  ok( $original, "loaded $f" );
  my $original_v = _spec_version($original);
  # UPCONVERSION
  if ( $original_v lt '2' ) {
    my $cmc = CPAN::Meta::Converter->new( $original );
    my $converted = $cmc->convert( version => 2 );
    is ( _spec_version($converted), 2, "up converted spec version $original_v to spec version 2");
    my $cmv = CPAN::Meta::Validator->new( $converted );
    ok ( $cmv->is_valid, "up converted META is valid" )
      or diag( "ERRORS:\n" . join( "\n", $cmv->errors )
#      . "\nMETA:\n" . Dumper($converted)
    );
  }
  # UPCONVERSION - partial
  if ( $original_v lt '1.4' ) {
    my $cmc = CPAN::Meta::Converter->new( $original );
    my $converted = $cmc->convert( version => '1.4' );
    is ( _spec_version($converted), 1.4, "up converted spec version $original_v to spec version 1.4");
    my $cmv = CPAN::Meta::Validator->new( $converted );
    ok ( $cmv->is_valid, "up converted META is valid" )
      or diag( "ERRORS:\n" . join( "\n", $cmv->errors )
#      . "\nMETA:\n" . Dumper($converted)
    );
  }
  # DOWNCONVERSION - partial
  if ( $original_v gt '1.2' ) {
    my $cmc = CPAN::Meta::Converter->new( $original );
    my $converted = $cmc->convert( version => '1.2' );
    is ( _spec_version($converted), '1.2', "down converted spec version $original_v to spec version 1.2");
    my $cmv = CPAN::Meta::Validator->new( $converted );
    ok ( $cmv->is_valid, "down converted META is valid" )
      or diag( "ERRORS:\n" . join( "\n", $cmv->errors )
#      . "\nMETA:\n" . Dumper($converted)
    );
  }
  # DOWNCONVERSION
  if ( $original_v gt '1.0' ) {
    my $cmc = CPAN::Meta::Converter->new( $original );
    my $converted = $cmc->convert( version => '1.0' );
    is ( _spec_version($converted), '1.0', "down converted spec version $original_v to spec version 1.0");
    my $cmv = CPAN::Meta::Validator->new( $converted );
    ok ( $cmv->is_valid, "down converted META is valid" )
      or diag( "ERRORS:\n" . join( "\n", $cmv->errors )
#      . "\nMETA:\n" . Dumper($converted)
    );
  }
}

done_testing;
# vim: ts=2 sts=2 sw=2 et :
