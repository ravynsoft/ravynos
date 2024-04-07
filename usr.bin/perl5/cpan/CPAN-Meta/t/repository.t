use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

# 1.4 repository upgrade
{
  my $label = "(version 1.4) old repository winds up in 'url'";
  my $meta = CPAN::Meta->new(
    {
      name     => 'Module-Billed',
      abstract => 'inscrutable',
      version  => '1',
      author   => 'Joe',
      release_status => 'stable',
      license  => 'perl_5',
      dynamic_config => 1,
      generated_by   => 'hand',
      'meta-spec' => {
        version => '1.4',
        url     => 'http://module-build.sourceforge.net/META-spec-v1.4.html',
      },
      resources => {
        repository => 'http://example.com/',
      },
    },
    { lazy_validation => 1 },
  );

  is_deeply(
    $meta->resources,
    {
      repository => {
        url => 'http://example.com/',
      },
    },
    $label,
  );
}

{
  my $label = "(version 2  ) http in web passed through unchanged";
  my $meta = CPAN::Meta->new(
    {
      name     => 'Module-Billed',
      abstract => 'inscrutable',
      version  => '1',
      author   => 'Joe',
      release_status => 'stable',
      license  => 'perl_5',
      dynamic_config => 1,
      generated_by   => 'hand',
      'meta-spec' => {
        version => '2',
      },
      resources => {
        repository => {
          web => 'http://example.com/',
        },
      },
    },
    { lazy_validation => 1 },
  );


  is_deeply(
    $meta->{resources},
    {
      repository => {
        web => 'http://example.com/',
      },
    },
    $label
  );
}

{
  my $label = "(version 2  ) http in url passed through unchanged";
  my $meta = CPAN::Meta->new(
    {
      name     => 'Module-Billed',
      abstract => 'inscrutable',
      version  => '1',
      author   => 'Joe',
      release_status => 'stable',
      license  => 'perl_5',
      dynamic_config => 1,
      generated_by   => 'hand',
      'meta-spec' => {
        version => '2',
      },
      resources => {
        repository => {
          url => 'http://example.com/',
        },
      },
    },
    { lazy_validation => 1 },
  );


  is_deeply(
    $meta->{resources},
    {
      repository => {
        url => 'http://example.com/',
      },
    },
    $label
  );
}

{
  my $label = "(version 2  ) svn in url adds svn type";
  my $meta = CPAN::Meta->new(
    {
      name     => 'Module-Billed',
      abstract => 'inscrutable',
      version  => '1',
      author   => 'Joe',
      release_status => 'stable',
      license  => 'perl_5',
      dynamic_config => 1,
      generated_by   => 'hand',
      'meta-spec' => {
        version => '2',
      },
      resources => {
        repository => {
          url => 'svn://example.com/',
        },
      },
    },
    { lazy_validation => 1 },
  );


  is_deeply(
    $meta->{resources},
    {
      repository => {
        url => 'svn://example.com/',
        type => 'svn',
      },
    },
    $label
  );
}

{
  my $label = "(version 2  ) git in url adds svn type";
  my $meta = CPAN::Meta->new(
    {
      name     => 'Module-Billed',
      abstract => 'inscrutable',
      version  => '1',
      author   => 'Joe',
      release_status => 'stable',
      license  => 'perl_5',
      dynamic_config => 1,
      generated_by   => 'hand',
      'meta-spec' => {
        version => '2',
      },
      resources => {
        repository => {
          url => 'git://example.com/',
        },
      },
    },
    { lazy_validation => 1 },
  );


  is_deeply(
    $meta->{resources},
    {
      repository => {
        url => 'git://example.com/',
        type => 'git',
      },
    },
    $label
  );
}

{
  my $label = "(version 2  ) pre-existing type preserved";
  my $meta = CPAN::Meta->new(
    {
      name     => 'Module-Billed',
      abstract => 'inscrutable',
      version  => '1',
      author   => 'Joe',
      release_status => 'stable',
      license  => 'perl_5',
      dynamic_config => 1,
      generated_by   => 'hand',
      'meta-spec' => {
        version => '2',
      },
      resources => {
        repository => {
          url => 'git://example.com/',
          type => 'msysgit',
        },
      },
    },
    { lazy_validation => 1 },
  );


  is_deeply(
    $meta->{resources},
    {
      repository => {
        url => 'git://example.com/',
        type => 'msysgit',
      },
    },
    $label
  );
}
done_testing;
# vim: ts=2 sts=2 sw=2 et :
