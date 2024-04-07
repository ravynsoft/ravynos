use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;
use Storable qw(dclone);
use Scalar::Util qw(blessed);

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my $distmeta = {
  name     => 'Module-Build',
  abstract => 'Build and install Perl modules',
  description =>  "Module::Build is a system for building, testing, "
              .   "and installing Perl modules.  It is meant to be an "
              .   "alternative to ExtUtils::MakeMaker... blah blah blah",
  version  => '0.36',
  author   => [
    'Ken Williams <kwilliams@cpan.org>',
    'Module-Build List <module-build@perl.org>', # additional contact
  ],
  release_status => 'stable',
  license  => [ 'perl_5' ],
  prereqs => {
    runtime => {
      requires => {
        'perl'   => '5.006',
        'Config' => '0',
        'Cwd'    => '0',
        'Data::Dumper' => '0',
        'ExtUtils::Install' => '0',
        'File::Basename' => '0',
        'File::Compare'  => '0',
        'File::Copy' => '0',
        'File::Find' => '0',
        'File::Path' => '0',
        'File::Spec' => '0',
        'IO::File'   => '0',
      },
      recommends => {
        'Archive::Tar' => '1.00',
        'ExtUtils::Install' => '0.3',
        'ExtUtils::ParseXS' => '2.02',
        'Pod::Text' => '0',
        'YAML' => '0.35',
      },
    },
    build => {
      requires => {
        'Test::More' => '0',
      },
    }
  },
  resources => {
    license => ['http://dev.perl.org/licenses/'],
  },
  optional_features => {
    domination => {
      description => 'Take over the world',
      prereqs     => {
        develop => { requires => { 'Genius::Evil'     => '1.234' } },
        runtime => { requires => { 'Machine::Weather' => '2.0'   } },
      },
    },
  },
  dynamic_config => 1,
  keywords => [ qw/ toolchain cpan dual-life / ],
  'meta-spec' => {
    version => '2',
    url     => 'http://search.cpan.org/perldoc?CPAN::Meta::Spec',
  },
  generated_by => 'Module::Build version 0.36',
  x_authority => 'cpan:FLORA',
  X_deep => { deep => 'structure' },
};

my $meta = CPAN::Meta->new(dclone $distmeta);

is(
  blessed($meta->as_struct),
  undef,
  "the result of ->as_struct is unblessed",
);

is_deeply( $meta->as_struct, $distmeta, "->as_struct (deep comparison)" );
isnt( $meta->as_struct, $distmeta, "->as_struct (is a deep clone)" );

my $old_copy = $meta->as_struct( {version => "1.4"} );
is( $old_copy->{'meta-spec'}{version}, "1.4", "->as_struct (downconversion)" );

isnt( $meta->resources, $meta->{resources}, "->resource (map values are deep cloned)");

is($meta->name,     'Module-Build', '->name');
is($meta->abstract, 'Build and install Perl modules', '->abstract');

like($meta->description, qr/Module::Build.+blah blah blah/, '->description');

is($meta->version,   '0.36', '->version');

ok($meta->dynamic_config, "->dynamic_config");

is_deeply(
  [ $meta->author ],
  [
    'Ken Williams <kwilliams@cpan.org>',
    'Module-Build List <module-build@perl.org>',
  ],
  '->author',
);

is_deeply(
  [ $meta->authors ],
  [
    'Ken Williams <kwilliams@cpan.org>',
    'Module-Build List <module-build@perl.org>',
  ],
  '->authors',
);

is_deeply(
  [ $meta->license ],
  [ qw(perl_5) ],
  '->license',
);

is_deeply(
  [ $meta->licenses ],
  [ qw(perl_5) ],
  '->licenses',
);

is_deeply(
  [ $meta->keywords ],
  [ qw/ toolchain cpan dual-life / ],
  '->keywords',
);

is_deeply(
  $meta->resources,
  { license => [ 'http://dev.perl.org/licenses/' ] },
  '->resources',
);

is_deeply(
  $meta->meta_spec,
  {
    version => '2',
    url     => 'http://search.cpan.org/perldoc?CPAN::Meta::Spec',
  },
  '->meta_spec',
);

is($meta->meta_spec_version, '2', '->meta_spec_version');

like($meta->generated_by, qr/Module::Build version 0.36/, '->generated_by');

my $basic = $meta->effective_prereqs;

is_deeply(
  $basic->as_string_hash,
  $distmeta->{prereqs},
  "->effective_prereqs()"
);

is_deeply( [ sort $meta->custom_keys ] , [ 'X_deep', 'x_authority' ],
  "->custom_keys"
);

is( $meta->custom('x_authority'), 'cpan:FLORA', "->custom(X)" );

is_deeply( $meta->custom('X_deep'), $distmeta->{'X_deep'},
  "->custom(X) [is_deeply]"
);

isnt( $meta->custom('X_deep'), $distmeta->{'X_deep'},
  "->custom(x) [is a deep clone]"
);

my $with_features = $meta->effective_prereqs([ qw(domination) ]);

is_deeply(
  $with_features->as_string_hash,
  {
    develop => { requires => { 'Genius::Evil'     => '1.234' } },
    runtime => {
      requires => {
        'perl'   => '5.006',
        'Config' => '0',
        'Cwd'    => '0',
        'Data::Dumper' => '0',
        'ExtUtils::Install' => '0',
        'File::Basename' => '0',
        'File::Compare'  => '0',
        'File::Copy' => '0',
        'File::Find' => '0',
        'File::Path' => '0',
        'File::Spec' => '0',
        'IO::File'   => '0',
        'Machine::Weather' => '2.0',
      },
      recommends => {
        'Archive::Tar' => '1.00',
        'ExtUtils::Install' => '0.3',
        'ExtUtils::ParseXS' => '2.02',
        'Pod::Text' => '0',
        'YAML' => '0.35',
      },
    },
    build => {
      requires => {
        'Test::More' => '0',
      },
    }
  },
  "->effective_prereqs([ qw(domination) ])"
);

my $chk_feature = sub {
  my $feature = shift;

  isa_ok($feature, 'CPAN::Meta::Feature');

  is($feature->identifier,  'domination',          '$feature->identifier');
  is($feature->description, 'Take over the world', '$feature->description');

  is_deeply(
    $feature->prereqs->as_string_hash,
    {
      develop => { requires => { 'Genius::Evil'     => '1.234' } },
      runtime => { requires => { 'Machine::Weather' => '2.0'   } },
    },
    '$feature->prereqs',
  );
};

my @features = $meta->features;
is(@features, 1, "we got one feature");
$chk_feature->($features[0]);

$chk_feature->( $meta->feature('domination') );


sub read_file {
  my $filename = shift;
  open my $fh, '<', $filename;
  local $/;
  my $string = <$fh>;
  $string =~ s/\$VERSION/$CPAN::Meta::VERSION/g;
  $string;
}

sub clean_backends {
  my $string = shift;
  $string =~ s{"?generated_by.*}{};
  $string =~ s{"?x_serialization_backend.*}{};
  return $string;
}

is(
  clean_backends($meta->as_string()),
  clean_backends(read_file('t/data-valid/META-2.json')),
  'as_string with no arguments defaults to version 2 and JSON',
);

is(
  clean_backends($meta->as_string({ version => 2 })),
  clean_backends(read_file('t/data-valid/META-2.json')),
  'as_string using version 2 defaults to JSON',
);

is(
  clean_backends($meta->as_string({ version => 1.4 })),
  clean_backends(read_file('t/data-valid/META-1_4.yml')),
  'as_string using version 1.4 defaults to YAML',
);

done_testing;
# vim: ts=2 sts=2 sw=2 et :
