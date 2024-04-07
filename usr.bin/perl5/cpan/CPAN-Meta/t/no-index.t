use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my %distmeta = (
  name     => 'Module-Billed',
  abstract => 'inscrutable',
  version  => '1',
  author   => [ 'Joe' ],
  release_status => 'stable',
  license  => [ 'perl_5' ],
  'meta-spec' => {
    version => '2',
    url     => 'http://search.cpan.org/perldoc?CPAN::Meta::Spec',
  },
  dynamic_config => 1,
  generated_by   => 'Module::Build version 0.36',
);

{
  my $meta = CPAN::Meta->new({ %distmeta });

  ok(
    $meta->should_index_package('Foo::Bar::Baz'),
    'we index any old package, without a no_index rule'
  );

  ok(
    $meta->should_index_file('lib/Foo/Bar/Baz.pm'),
    'we index any old file, without a no_index rule'
  );
}

{
  my $meta = CPAN::Meta->new({
    %distmeta,
    no_index => {
      package   => [ 'Foo::Bar' ],
      namespace => [ 'Foo::Bar::Baz' ],
    }
  });

  ok(
    ! $meta->should_index_package('Foo::Bar'),
    'exclude a specific package'
  );

  ok(
    $meta->should_index_package('Foo::Bar::Baz'),
    'namespace X does not exclude package X'
  );

  ok(
    ! $meta->should_index_package('Foo::Bar::Baz::Quux'),
    'exclude something under a namespace'
  );
}

{
  my $meta = CPAN::Meta->new({
    %distmeta,
    no_index => {
      file      => [ 'lib/Foo/Bar.pm'  ],
      directory => [ 'lib/Foo/Bar/Baz' ],
    }
  });

  ok(
    ! $meta->should_index_file('lib/Foo/Bar.pm'),
    'exclude a specific file'
  );

  ok(
    $meta->should_index_file('lib/Foo/Bar/Baz.pm'),
    'do not exclude a file with a name like an excluded dir',
  );

  ok(
    ! $meta->should_index_file('lib/Foo/Bar/Baz/Quux.pm'),
    'exclude something under a directory'
  );
}

done_testing;
# vim: ts=2 sts=2 sw=2 et :
