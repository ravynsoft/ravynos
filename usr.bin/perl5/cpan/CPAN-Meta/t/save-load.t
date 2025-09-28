use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;
use File::Temp 0.20 ();
use Parse::CPAN::Meta;

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
};

my $meta = CPAN::Meta->new( $distmeta );

my $tmpdir = File::Temp->newdir();
my $metafile = File::Spec->catfile( $tmpdir, 'META.json' );

ok( $meta->save($metafile), "save returns true" );
ok( -f $metafile, "save meta to file" );

ok( my $loaded = Parse::CPAN::Meta->load_file($metafile), 'load saved file' );
is($loaded->{name},     'Module-Build', 'name correct');

like(
  $loaded->{x_serialization_backend},
  qr/\AJSON::PP version [0-9]/,
  "x_serialization_backend",
);

ok(
  ! exists $meta->{x_serialization_backend},
  "we didn't leak x_serialization_backend up into the saved struct",
);

ok( $loaded = Parse::CPAN::Meta->load_file('t/data-test/META-1_4.yml'), 'load META-1.4' );
is($loaded->{name},     'Module-Build', 'name correct');

# Test saving with conversion

my $metayml = File::Spec->catfile( $tmpdir, 'META.yml' );

$meta->save($metayml, {version => "1.4"});
ok( -f $metayml, "save meta to META.yml with conversion" );

ok( $loaded = Parse::CPAN::Meta->load_file($metayml), 'load saved file' );
is( $loaded->{name},     'Module-Build', 'name correct');
is( $loaded->{requires}{perl}, "5.006", 'prereq correct' );

like(
  $loaded->{x_serialization_backend},
  qr/\ACPAN::Meta::YAML version [0-9]/,
  "x_serialization_backend",
);

ok(
  ! exists $meta->{x_serialization_backend},
  "we didn't leak x_serialization_backend up into the saved struct",
);

# file without suffix

ok( $loaded = CPAN::Meta->load_file('t/data-test/META-2.meta'), 'load_file META-2.meta' );

my $string = do { open my $fh, '<', 't/data-test/META-2.meta'; local $/; <$fh> };
ok( $loaded = CPAN::Meta->load_string($string), 'load META-2.meta from string' );

done_testing;
# vim: ts=2 sts=2 sw=2 et :
