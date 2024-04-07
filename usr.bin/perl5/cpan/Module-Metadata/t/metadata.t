# -*- mode: cperl; tab-width: 8; indent-tabs-mode: nil; basic-offset: 2 -*-
# vim:ts=8:sw=2:et:sta:sts=2

use strict;
use warnings;
use Encode 'decode';
use Test::More 0.82;
use IO::File;
use File::Spec;
use File::Temp;
use File::Basename;
use Cwd ();
use File::Path;

use lib 't/lib';
use GeneratePackage;

my $tmpdir = GeneratePackage::tmpdir();

plan tests => 72;

require_ok('Module::Metadata');

{
    # class method C<find_module_by_name>
    my $module = Module::Metadata->find_module_by_name(
                   'Module::Metadata' );
    ok( -e $module, 'find_module_by_name() succeeds' );
}

#########################

# generates a new distribution:
# files => { relative filename => $content ... }
# returns the name of the distribution (not including version),
# and the absolute path name to the dist.
{
  my $test_num = 0;
  sub new_dist {
    my %opts = @_;

    my $distname = 'Simple' . $test_num++;
    my $distdir = File::Spec->catdir($tmpdir, $distname);
    note "using dist $distname in $distdir";

    File::Path::mkpath($distdir) or die "failed to create '$distdir'";

    foreach my $rel_filename (keys %{$opts{files}})
    {
      my $abs_filename = File::Spec->catfile($distdir, $rel_filename);
      my $dirname = File::Basename::dirname($abs_filename);
      unless (-d $dirname) {
        File::Path::mkpath($dirname) or die "Can't create '$dirname'";
      }

      note "creating $abs_filename";
      my $fh = IO::File->new(">$abs_filename") or die "Can't write '$abs_filename'\n";
      print $fh $opts{files}{$rel_filename};
      close $fh;
    }

    chdir $distdir;
    return ($distname, $distdir);
  }
}

{
  # fail on invalid module name
  my $pm_info = Module::Metadata->new_from_module(
                  'Foo::Bar', inc => [] );
  ok( !defined( $pm_info ), 'fail if can\'t find module by module name' );
}

{
  # fail on invalid filename
  my $file = File::Spec->catfile( 'Foo', 'Bar.pm' );
  my $pm_info = Module::Metadata->new_from_file( $file, inc => [] );
  ok( !defined( $pm_info ), 'fail if can\'t find module by file name' );
}

{
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => "package Simple;\n" });

  # construct from module filename
  my $pm_info = Module::Metadata->new_from_file( $file );
  ok( defined( $pm_info ), 'new_from_file() succeeds' );

  # construct from filehandle
  my $handle = IO::File->new($file);
  $pm_info = Module::Metadata->new_from_handle( $handle, $file );
  ok( defined( $pm_info ), 'new_from_handle() succeeds' );
  $pm_info = Module::Metadata->new_from_handle( $handle );
  is( $pm_info, undef, "new_from_handle() without filename returns undef" );
  close($handle);
}

{
  # construct from module name, using custom include path
  my $pm_info = Module::Metadata->new_from_module(
               'Simple', inc => [ 'lib', @INC ] );
  ok( defined( $pm_info ), 'new_from_module() succeeds' );
}


{
  # Find each package only once
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple;
$VERSION = '1.23';
package Error::Simple;
$VERSION = '2.34';
package Simple;
---

  my $pm_info = Module::Metadata->new_from_file( $file );

  my @packages = $pm_info->packages_inside;
  is( @packages, 2, 'record only one occurence of each package' );
}

{
  # Module 'Simple.pm' does not contain package 'Simple';
  # constructor should not complain, no default module name or version
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple::Not;
$VERSION = '1.23';
---

  my $pm_info = Module::Metadata->new_from_file( $file );

  is( $pm_info->name, undef, 'no default package' );
  is( $pm_info->version, undef, 'no version w/o default package' );
}

# parse $VERSION lines scripts for package main
my @scripts = (
  <<'---', # package main declared
#!perl -w
package main;
$VERSION = '0.01';
---
  <<'---', # on first non-comment line, non declared package main
#!perl -w
$VERSION = '0.01';
---
  <<'---', # after non-comment line
#!perl -w
use strict;
$VERSION = '0.01';
---
  <<'---', # 1st declared package
#!perl -w
package main;
$VERSION = '0.01';
package _private;
$VERSION = '999';
---
  <<'---', # 2nd declared package
#!perl -w
package _private;
$VERSION = '999';
package main;
$VERSION = '0.01';
---
  <<'---', # split package
#!perl -w
package main;
package _private;
$VERSION = '999';
package main;
$VERSION = '0.01';
---
  <<'---', # define 'main' version from other package
package _private;
$::VERSION = 0.01;
$VERSION = '999';
---
  <<'---', # define 'main' version from other package
package _private;
$VERSION = '999';
$::VERSION = 0.01;
---
);

my ( $i, $n ) = ( 1, scalar( @scripts ) );
foreach my $script ( @scripts ) {
  note '-------';
  my $errs;
  my $file = File::Spec->catfile('bin', 'simple.plx');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => $script } );
  my $pm_info = Module::Metadata->new_from_file( $file );

  is( $pm_info->name, 'main', 'name for script is always main');
  is( $pm_info->version, '0.01', "correct script version ($i of $n)" ) or $errs++;
  $i++;

  diag 'parsed module: ', explain($pm_info) if $errs and not $ENV{PERL_CORE}
    and ($ENV{AUTHOR_TESTING} or $ENV{AUTOMATED_TESTING});
}

{
  # examine properties of a module: name, pod, etc
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple;
$VERSION = '0.01';
package Simple::Ex;
$VERSION = '0.02';

=encoding UTF-8

=head1 NAME

Simple - It's easy.

=head1 AUTHOR

Símple Simon

You can find me on the IRC channel
#simon on irc.perl.org.

=cut
---

  my $pm_info = Module::Metadata->new_from_module(
             'Simple', inc => [ 'lib', @INC ] );

  is( $pm_info->name, 'Simple', 'found default package' );
  is( $pm_info->version, '0.01', 'version for default package' );

  # got correct version for secondary package
  is( $pm_info->version( 'Simple::Ex' ), '0.02',
      'version for secondary package' );

  my $filename = $pm_info->filename;
  ok( defined( $filename ) && -e $filename,
      'filename() returns valid path to module file' );

  my @packages = $pm_info->packages_inside;
  is( @packages, 2, 'found correct number of packages' );
  is( $packages[0], 'Simple', 'packages stored in order found' );

  # we can detect presence of pod regardless of whether we are collecting it
  ok( $pm_info->contains_pod, 'contains_pod() succeeds' );

  my @pod = $pm_info->pod_inside;
  is_deeply( \@pod, [qw(NAME AUTHOR)], 'found all pod sections' );

  is( $pm_info->pod('NONE') , undef,
      'return undef() if pod section not present' );

  is( $pm_info->pod('NAME'), undef,
      'return undef() if pod section not collected' );


  # collect_pod
  $pm_info = Module::Metadata->new_from_module(
               'Simple', inc => [ 'lib', @INC ], collect_pod => 1 );

  my %pod;
  for my $section (qw(NAME AUTHOR)) {
    my $content = $pm_info->pod( $section );
    if ( $content ) {
      $content =~ s/^\s+//;
      $content =~ s/\s+$//;
    }
    $pod{$section} = $content;
  }
  my %expected = (
    NAME   => q|Simple - It's easy.|,
    AUTHOR => <<'EXPECTED'
Símple Simon

You can find me on the IRC channel
#simon on irc.perl.org.
EXPECTED
  );
  for my $text (values %expected) {
    $text =~ s/^\s+//;
    $text =~ s/\s+$//;
  }
  is( $pod{NAME},   $expected{NAME},   'collected NAME pod section' );
  is( $pod{AUTHOR}, $expected{AUTHOR}, 'collected AUTHOR pod section' );

  my $pm_info2 = Module::Metadata->new_from_module(
               'Simple', inc => [ 'lib', @INC ], collect_pod => 1, decode_pod => 1 );
  my $author = $pm_info2->pod( 'AUTHOR' );
  $author =~ s/^\s+//;
  $author =~ s/\s+$//;
  is( $author, decode('UTF-8', $expected{AUTHOR} ), 'collected AUTHOR pod section in UTF-8' );
}

{
  # test things that look like POD, but aren't
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple;

=YES THIS STARTS POD

our $VERSION = '999';

=cute

our $VERSION = '666';

=cut

*foo
=*no_this_does_not_start_pod;

our $VERSION = '1.23';

---
  my $pm_info = Module::Metadata->new_from_file('lib/Simple.pm');
  is( $pm_info->name, 'Simple', 'found default package' );
  is( $pm_info->version, '1.23', 'version for default package' );
}

my $undef;
my $test_num = 0;

{
  # and now a real pod file
  # (this test case is ready to be rolled into a corpus loop, later)
  my $test_case = {
    name => 'file only contains pod',
    filename => 'Simple/Documentation.pod',
    code => <<'---',
# PODNAME: Simple::Documentation
# ABSTRACT: My documentation

=pod

Hello, this is pod.

=cut
---
    module => '', # TODO: should probably be $undef actually
    all_versions => { },
  };

  note $test_case->{name};
  my $code = $test_case->{code};
  my $expected_name = $test_case->{module};
  local $TODO = $test_case->{TODO};

  my $errs;

  my ($vol, $dir, $basename) = File::Spec->splitpath(File::Spec->catfile($tmpdir, "Simple${test_num}", ($test_case->{filename} || 'Simple.pm')));
  my $pm_info = Module::Metadata->new_from_file(generate_file($dir, $basename, $code));

  my $got_name = $pm_info->name;
  is(
    $got_name,
    $expected_name,
    "case '$test_case->{name}': module name matches",
  )
  or $errs++;

  diag 'parsed module: ', explain($pm_info) if $errs and not $ENV{PERL_CORE}
    and ($ENV{AUTHOR_TESTING} or $ENV{AUTOMATED_TESTING});
}

{
  # Make sure processing stops after __DATA__
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple;
$VERSION = '0.01';
__DATA__
*UNIVERSAL::VERSION = sub {
  foo();
};
---

  my $pm_info = Module::Metadata->new_from_file('lib/Simple.pm');
  is( $pm_info->name, 'Simple', 'found default package' );
  is( $pm_info->version, '0.01', 'version for default package' );
  my @packages = $pm_info->packages_inside;
  is_deeply(\@packages, ['Simple'], 'packages inside');
}

{
  # Make sure we handle version.pm $VERSIONs well
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple;
$VERSION = version->new('0.60.' . (qw$Revision: 128 $)[1]);
package Simple::Simon;
$VERSION = version->new('0.61.' . (qw$Revision: 129 $)[1]);
---

  my $pm_info = Module::Metadata->new_from_file('lib/Simple.pm');
  is( $pm_info->name, 'Simple', 'found default package' );
  is( $pm_info->version, '0.60.128', 'version for default package' );
  my @packages = $pm_info->packages_inside;
  is_deeply([sort @packages], ['Simple', 'Simple::Simon'], 'packages inside');
  is( $pm_info->version('Simple::Simon'), '0.61.129', 'version for embedded package' );
}

# check that package_versions_from_directory works

{
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package Simple;
$VERSION = '0.01';
package Simple::Ex;
$VERSION = '0.02';
{
  package main; # should ignore this
}
{
  package DB; # should ignore this
}
{
  package Simple::_private; # should ignore this
}

=head1 NAME

Simple - It's easy.

=head1 AUTHOR

Simple Simon

=cut
---

  my $exp_pvfd = {
    'Simple' => {
      'file' => 'Simple.pm',
      'version' => '0.01'
    },
    'Simple::Ex' => {
      'file' => 'Simple.pm',
      'version' => '0.02'
    }
  };

  my $dir = "lib";
  my $got_pvfd = Module::Metadata->package_versions_from_directory($dir);

  is_deeply( $got_pvfd, $exp_pvfd, "package_version_from_directory()" )
    or diag explain $got_pvfd;

  my $absolute_file = File::Spec->rel2abs($exp_pvfd->{Simple}{file}, $dir);
  my $got_pvfd2 = Module::Metadata->package_versions_from_directory($dir, [$absolute_file]);

  is_deeply( $got_pvfd2, $exp_pvfd, "package_version_from_directory() with provided absolute file path" )
    or diag explain $got_pvfd;

{
  my $got_provides = Module::Metadata->provides(dir => 'lib', version => 2);
  my $exp_provides = {
    'Simple' => {
      'file' => 'lib/Simple.pm',
      'version' => '0.01'
    },
    'Simple::Ex' => {
      'file' => 'lib/Simple.pm',
      'version' => '0.02'
    }
  };

  is_deeply( $got_provides, $exp_provides, "provides()" )
    or diag explain $got_provides;
}

{
  my $got_provides = Module::Metadata->provides(dir => 'lib', prefix => 'other', version => 1.4);
  my $exp_provides = {
    'Simple' => {
      'file' => 'other/Simple.pm',
      'version' => '0.01'
    },
    'Simple::Ex' => {
      'file' => 'other/Simple.pm',
      'version' => '0.02'
    }
  };

  is_deeply( $got_provides, $exp_provides, "provides()" )
    or diag explain $got_provides;
}
}

# Check package_versions_from_directory with regard to case-sensitivity
{
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package simple;
$VERSION = '0.01';
---

  my $pm_info = Module::Metadata->new_from_file('lib/Simple.pm');
  is( $pm_info->name, undef, 'no default package' );
  is( $pm_info->version, undef, 'version for default package' );
  is( $pm_info->version('simple'), '0.01', 'version for lower-case package' );
  is( $pm_info->version('Simple'), undef, 'version for capitalized package' );
  ok( $pm_info->is_indexable(), 'an indexable package is found' );
  ok( $pm_info->is_indexable('simple'), 'the simple package is indexable' );
  ok( !$pm_info->is_indexable('Simple'), 'the Simple package would not be indexed' );
}

{
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package simple;
$VERSION = '0.01';
package Simple;
$VERSION = '0.02';
package SiMpLe;
$VERSION = '0.03';
---

  my $pm_info = Module::Metadata->new_from_file('lib/Simple.pm');
  is( $pm_info->name, 'Simple', 'found default package' );
  is( $pm_info->version, '0.02', 'version for default package' );
  is( $pm_info->version('simple'), '0.01', 'version for lower-case package' );
  is( $pm_info->version('Simple'), '0.02', 'version for capitalized package' );
  is( $pm_info->version('SiMpLe'), '0.03', 'version for mixed-case package' );
  ok( $pm_info->is_indexable('simple'), 'the simple package is indexable' );
  ok( $pm_info->is_indexable('Simple'), 'the Simple package is indexable' );
}

{
  my $file = File::Spec->catfile('lib', 'Simple.pm');
  my ($dist_name, $dist_dir) = new_dist(files => { $file => <<'---' } );
package ## hide from PAUSE
   simple;
$VERSION = '0.01';
---

  my $pm_info = Module::Metadata->new_from_file('lib/Simple.pm');
  is( $pm_info->name, undef, 'no package names found' );
  ok( !$pm_info->is_indexable('simple'), 'the simple package would not be indexed' );
  ok( !$pm_info->is_indexable('Simple'), 'the Simple package would not be indexed' );
  ok( !$pm_info->is_indexable(), 'no indexable package is found' );
}
