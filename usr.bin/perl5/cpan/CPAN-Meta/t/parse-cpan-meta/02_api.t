#!/usr/bin/perl

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

# Testing of a known-bad file from an editor

use strict;
BEGIN {
	$|  = 1;
	$^W = 1;
}

use lib 't/lib';
use File::Spec::Functions ':ALL';
use Parse::CPAN::Meta;
use Parse::CPAN::Meta::Test;
# use Test::More skip_all => 'Temporarily ignoring failing test';
use Test::More 'no_plan';
use utf8;

#####################################################################
# Testing that Perl::Smith config files work

my $want = {
  "abstract" => "a set of version requirements for a CPAN dist",
  "author"   => [ 'Ricardo Signes <rjbs@cpan.org>' ],
  "build_requires" => {
     "Test::More" => "0.88"
  },
  "configure_requires" => {
     "ExtUtils::MakeMaker" => "6.31"
  },
  "generated_by" => "Dist::Zilla version 2.100991",
  "license" => "perl",
  "meta-spec" => {
     "url" => "http://module-build.sourceforge.net/META-spec-v1.4.html",
     "version" => 1.4
  },
  "name" => "Version-Requirements",
  "recommends" => {},
  "requires" => {
     "Carp" => "0",
     "Scalar::Util" => "0",
     "version" => "0.77"
  },
  "resources" => {
     "repository" => "git://git.codesimply.com/Version-Requirements.git"
  },
  "version" => "0.101010",
  "x_contributors" => [
    "Dagfinn Ilmari Mannsåker <ilmari\@ilmari.org>",
  ],
};

my $meta_json = catfile( test_data_directory(), 'META-VR.json' );
my $meta_yaml = catfile( test_data_directory(), 'META-VR.yml' );
my $bare_yaml_meta = catfile( test_data_directory(), 'bareyaml.meta' );
my $bad_yaml_meta = catfile( test_data_directory(), 'BadMETA.yml' );
my $CL018_yaml_meta = catfile( test_data_directory(), 'CL018_yaml.meta' );

# These test YAML/JSON detection without the typical file name suffix
my $yaml_meta = catfile( test_data_directory(), 'yaml.meta' );
my $json_meta = catfile( test_data_directory(), 'json.meta' );

### YAML tests
{
  local $ENV{PERL_YAML_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get CPAN::META::YAML

  is(Parse::CPAN::Meta->yaml_backend(), 'CPAN::Meta::YAML', 'yaml_backend(): CPAN::Meta::YAML');
  my $from_yaml = Parse::CPAN::Meta->load_file( $meta_yaml );
  is_deeply($from_yaml, $want, "load from YAML file results in expected data");
}

{
  local $ENV{PERL_YAML_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get CPAN::META::YAML

  note '';
  is(Parse::CPAN::Meta->yaml_backend(), 'CPAN::Meta::YAML', 'yaml_backend(): CPAN::Meta::YAML');
  my $from_yaml = Parse::CPAN::Meta->load_file( $yaml_meta );
  is_deeply($from_yaml, $want, "load from YAML .meta file results in expected data");
}

{
  local $ENV{PERL_YAML_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get CPAN::META::YAML

  note '';
  is(Parse::CPAN::Meta->yaml_backend(), 'CPAN::Meta::YAML', 'yaml_backend(): CPAN::Meta::YAML');
  my $from_yaml = Parse::CPAN::Meta->load_file( $bare_yaml_meta );
  is_deeply($from_yaml, $want, "load from bare YAML .meta file results in expected data");
}

{
  local $ENV{PERL_YAML_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get CPAN::META::YAML

  note '';
  is(Parse::CPAN::Meta->yaml_backend(), 'CPAN::Meta::YAML', 'yaml_backend(): CPAN::Meta::YAML');
  my $from_yaml = Parse::CPAN::Meta->load_file( $CL018_yaml_meta );
  like($from_yaml->{x_contributors}[5], qr/Olivier Mengu/, "Open question: what to expect from double encoded UTF-8");
}

{
  local $ENV{PERL_YAML_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get CPAN::META::YAML

  note '';
  is(Parse::CPAN::Meta->yaml_backend(), 'CPAN::Meta::YAML', 'yaml_backend(): CPAN::Meta::YAML');
  my $yaml   = load_ok( $meta_yaml, $meta_yaml, 100, ":encoding(UTF-8)");
  my $from_yaml = Parse::CPAN::Meta->load_yaml_string( $yaml );
  is_deeply($from_yaml, $want, "load from YAML str results in expected data");
}

{
  local $ENV{PERL_YAML_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get CPAN::META::YAML

  note '';
  is(Parse::CPAN::Meta->yaml_backend(), 'CPAN::Meta::YAML', 'yaml_backend(): CPAN::Meta::YAML');
  my @yaml   = Parse::CPAN::Meta::LoadFile( $bad_yaml_meta );
  is($yaml[0]{author}[0], 'Olivier Mengu\xE9', "Bad UTF-8 is replaced");
}


SKIP: {
  note '';
  skip 'these tests are for cpan builds only', 2 if $ENV{PERL_CORE};
  skip "YAML module not installed", 2
    unless eval "require YAML; 1";
  local $ENV{PERL_YAML_BACKEND} = 'YAML';

  is(Parse::CPAN::Meta->yaml_backend(), 'YAML', 'yaml_backend(): YAML');
  my $yaml   = load_ok( $meta_yaml, $meta_yaml, 100, ":encoding(UTF-8)");
  my $from_yaml = Parse::CPAN::Meta->load_yaml_string( $yaml );
  is_deeply($from_yaml, $want, "load_yaml_string using PERL_YAML_BACKEND");
}

### JSON tests
{
  # JSON tests with JSON::PP
  local $ENV{PERL_JSON_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get JSON::PP

  note '';
  is(Parse::CPAN::Meta->json_backend(), 'JSON::PP', 'json_backend(): JSON::PP');
  my $from_json = Parse::CPAN::Meta->load_file( $meta_json );
  is_deeply($from_json, $want, "load from JSON file results in expected data");
}

{
  # JSON tests with JSON::PP
  local $ENV{PERL_JSON_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get JSON::PP

  note '';
  is(Parse::CPAN::Meta->json_backend(), 'JSON::PP', 'json_backend(): JSON::PP');
  my $from_json = Parse::CPAN::Meta->load_file( $json_meta );
  is_deeply($from_json, $want, "load from JSON .meta file results in expected data");
}

{
  # JSON tests with JSON::PP
  local $ENV{PERL_JSON_BACKEND} if not $ENV{PERL_CORE}; # ensure we always get JSON::PP

  note '';
  is(Parse::CPAN::Meta->json_backend(), 'JSON::PP', 'json_backend(): JSON::PP');
  my $json   = load_ok( $meta_json, $meta_json, 100, "encoding(UTF-8)");
  my $from_json = Parse::CPAN::Meta->load_json_string( $json );
  is_deeply($from_json, $want, "load from JSON str results in expected data");
}

{
  # JSON tests with JSON::PP, take 2
  local $ENV{PERL_JSON_BACKEND} = 0 if not $ENV{PERL_CORE}; # request JSON::PP

  note '';
  is(Parse::CPAN::Meta->json_backend(), 'JSON::PP', 'json_backend(): JSON::PP');
  my $json   = load_ok( $meta_json, $meta_json, 100, ":encoding(UTF-8)");
  my $from_json = Parse::CPAN::Meta->load_json_string( $json );
  is_deeply($from_json, $want, "load_json_string with PERL_JSON_BACKEND = 0");
}

{
  # JSON tests with JSON::PP, take 3
  local $ENV{PERL_JSON_BACKEND} = 'JSON::PP' if not $ENV{PERL_CORE}; # request JSON::PP

  note '';
  is(Parse::CPAN::Meta->json_backend(), 'JSON::PP', 'json_backend(): JSON::PP');
  my $json   = load_ok( $meta_json, $meta_json, 100, ":encoding(UTF-8)");
  my $from_json = Parse::CPAN::Meta->load_json_string( $json );
  is_deeply($from_json, $want, "load_json_string with PERL_JSON_BACKEND = 'JSON::PP'");
}

SKIP: {
  # JSON tests with fake backend

  note '';
  skip 'these tests are for cpan builds only', 2 if $ENV{PERL_CORE};

  { package MyJSONThingy; $INC{'MyJSONThingy.pm'} = __FILE__; require JSON::PP;
    sub decode_json { JSON::PP::decode_json(@_) } }

  local $ENV{CPAN_META_JSON_DECODER} = 'MyJSONThingy'; # request fake backend

  is(Parse::CPAN::Meta->json_decoder(), 'MyJSONThingy', 'json_decoder(): MyJSONThingy');
  my $json   = load_ok( $meta_json, $meta_json, 100, ":encoding(UTF-8)");
  my $from_json = Parse::CPAN::Meta->load_json_string( $json );
  is_deeply($from_json, $want, "load_json_string with PERL_JSON_DECODER = 'MyJSONThingy'");
}

SKIP: {
  note '';
  skip 'these tests are for cpan builds only', 2 if $ENV{PERL_CORE};
  skip "JSON module version 2.5 not installed", 2
    unless eval "require JSON; JSON->VERSION(2.5); 1";
  local $ENV{PERL_JSON_BACKEND} = 1;

  is(Parse::CPAN::Meta->json_backend(), 'JSON', 'json_backend(): JSON');
  my $json   = load_ok( $meta_json, $meta_json, 100, ":encoding(UTF-8)");
  my $from_json = Parse::CPAN::Meta->load_json_string( $json );
  is_deeply($from_json, $want, "load_json_string with PERL_JSON_BACKEND = 1");
}

