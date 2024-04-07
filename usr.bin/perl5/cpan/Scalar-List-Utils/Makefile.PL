# -*- perl -*-
BEGIN { require 5.006; }
use strict;
use warnings;
use Config;
use File::Spec;
use ExtUtils::MakeMaker;
my $PERL_CORE = grep { $_ eq 'PERL_CORE=1' } @ARGV;
my $defines = $ENV{PERL_CORE} ? q[-DPERL_EXT] : q[-DPERL_EXT -DUSE_PPPORT_H];

my %params = (
  NAME         => q[List::Util],
  ABSTRACT     => q[Common Scalar and List utility subroutines],
  AUTHOR       => q[Graham Barr <gbarr@cpan.org>],
  DEFINE       => $defines,
  DISTNAME     => q[Scalar-List-Utils],
  VERSION_FROM => 'lib/List/Util.pm',

  # We go through the ListUtil.xs trickery to foil platforms
  # that have the feature combination of
  # (1) static builds
  # (2) allowing only one object by the same name in the static library
  # (3) the object name matching being case-blind
  # This means that we can't have the top-level util.o
  # and the extension-level Util.o in the same build.
  # One such platform is the POSIX-BC BS2000 EBCDIC mainframe platform.
  XS     => {'ListUtil.xs' => 'ListUtil.c'},
  OBJECT => 'ListUtil$(OBJ_EXT)',
  ( $PERL_CORE
    ? ()
    : (
      INSTALLDIRS      => ($] < 5.011 ? q[perl] : q[site]),
      TEST_REQUIRES => {
        'Test::More' => 0,
      },
      (eval { ExtUtils::MakeMaker->VERSION(6.31) } ? (LICENSE => 'perl') : ()),
      (eval { ExtUtils::MakeMaker->VERSION(6.48) } ? (MIN_PERL_VERSION => '5.006') : ()),
      ( eval { ExtUtils::MakeMaker->VERSION(6.46) } ? (
          META_MERGE => {
            'meta-spec' => { version => 2 },
            dynamic_config => 0,
            resources => {    ##
              repository => {
                url => 'https://github.com/Scalar-List-Utils/Scalar-List-Utils.git',
                web => 'https://github.com/Scalar-List-Utils/Scalar-List-Utils',
                type => 'git',
              },
              bugtracker => {
                mailto => 'bug-Scalar-List-Utils@rt.cpan.org',
                web => 'https://rt.cpan.org/Public/Dist/Display.html?Name=Scalar-List-Utils',
              },
            },
          }
          )
        : ()
      ),
    )
  ),
);

if ($params{TEST_REQUIRES} and !eval { ExtUtils::MakeMaker->VERSION(6.64) }) {
    $params{BUILD_REQUIRES} = {
        %{$params{BUILD_REQUIRES} || {}},
        %{delete $params{TEST_REQUIRES}},
    };
}
if ($params{BUILD_REQUIRES} and !eval { ExtUtils::MakeMaker->VERSION(6.5503) }) {
    $params{PREREQ_PM} = {
        %{$params{PREREQ_PM} || {}},
        %{delete $params{BUILD_REQUIRES}},
    };
}

WriteMakefile(%params);
