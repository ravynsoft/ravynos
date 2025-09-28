#!/usr/bin/perl -w
################################################################################
#
#  buildperl.pl -- build various versions of perl automatically
#
################################################################################
#
#  Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.
#  Version 2.x, Copyright (C) 2001, Paul Marquess.
#  Version 1.x, Copyright (C) 1999, Kenneth Albanowski.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

use strict;
use Getopt::Long;
use Pod::Usage;
use File::Find;
use File::Path;
use Data::Dumper;
use IO::File;
use Cwd;

# TODO: - extra arguments to Configure

#
#  --test-archives=1      check if archives can be read
#  --test-archives=2      like 1, but also extract archives
#  --test-archives=3      like 2, but also apply patches
#

my %opt = (
  prefix    => '/tmp/perl/install/<config>/<perl>',
  build     => '/tmp/perl/build/<config>',
  source    => '/tmp/perl/source',
  force     => 0,
  test      => 0,
  install   => 1,
  oneshot   => 0,
  configure => 0,
  jobs => 1,
  'test-archives' => 0,
);

my $Configure_extra = '-Dman1dir="none" -Dman3dir="none"';


my %config = (
  default     => {
                   config_args => "-des $Configure_extra" ,
                 },
  thread      => {
                   config_args     => "-des -Dusethreads $Configure_extra",
                   masked_versions => [ qr/^5\.00[01234]/ ],
                 },
  thread5005  => {
                   config_args     => "-des -Duse5005threads $Configure_extra",
                   masked_versions => [ qr/^5\.00[012345]|^5\.(9|\d\d)|^5\.8\.9/ ],
                 },
  debug       => {
                   config_args => "-des -Doptimize=-g $Configure_extra",
                 },
);

my @patch = (
  {
    perl => [
              qr/^5\.00[01234]/,
              qw/
                5.005
                5.005_01
                5.005_02
                5.005_03
              /,
            ],
    subs => [
              [ \&patch_db, 1 ],
            ],
  },
  {
    perl => [
              qw/
                5.6.0
                5.6.1
                5.7.0
                5.7.1
                5.7.2
                5.7.3
                5.8.0
              /,
            ],
    subs => [
              [ \&patch_db, 3 ],
            ],
  },
  {
    perl => [
              qr/^5\.004_0[1234]$/,
            ],
    subs => [
              [ \&patch_doio ],
            ],
  },
  {
    perl => [
              qw/
                5.005
                5.005_01
                5.005_02
              /,
            ],
    subs => [
              [ \&patch_sysv, old_format => 1 ],
            ],
  },
  {
    perl => [
              qw/
                5.005_03
                5.005_04
              /,
              qr/^5\.6\.[0-2]$/,
              qr/^5\.7\.[0-3]$/,
              qr/^5\.8\.[0-8]$/,
              qr/^5\.9\.[0-5]$/
            ],
    subs => [
              [ \&patch_sysv ],
            ],
  },
  {
    perl => [
              qr/^5\.004_05$/,
              qr/^5\.005(?:_0[1-4])?$/,
              qr/^5\.6\.[01]$/,
            ],
    subs => [
              [ \&patch_configure ],
              [ \&patch_makedepend_lc ],
            ],
  },
  {
    perl => [
              '5.8.0',
            ],
    subs => [
              [ \&patch_makedepend_lc ],
            ],
  },
);

my(%perl, @perls);

GetOptions(\%opt, qw(
  config=s@
  prefix=s
  build=s
  source=s
  perl=s@
  force
  test
  install!
  test-archives=i
  patch!
  oneshot
  jobs=i
)) or pod2usage(2);

my %current;

my $job_string = "";
$job_string = "-j$opt{jobs}" if $opt{jobs} != 1;

if ($opt{patch} || $opt{oneshot}) {
  @{$opt{perl}} == 1 or die "Exactly one --perl must be given with --patch or --oneshot\n";
  my $perl = $opt{perl}[0];
  patch_source($perl) if !exists $opt{patch} || $opt{patch};
  if (exists $opt{oneshot}) {
    eval { require String::ShellQuote };
    die "--oneshot requires String::ShellQuote to be installed\n" if $@;
    %current = (config => 'oneshot', version => $perl);
    $config{oneshot} = { config_args => String::ShellQuote::shell_quote(@ARGV) };
    build_and_install($perl{$perl});
  }
  exit 0;
}

if (exists $opt{config}) {
  for my $cfg (@{$opt{config}}) {
    exists $config{$cfg} or die "Unknown configuration: $cfg\n";
  }
}
else {
  $opt{config} = [sort keys %config];
}

find(sub {
  /^(perl-?(5\..*))\.tar\.(gz|bz2|lzma)$/ or return;
  $perl{$1} = { version => $2, source => $File::Find::name, compress => $3 };
}, $opt{source});

if (exists $opt{perl}) {
  for my $perl (@{$opt{perl}}) {
    my $p = $perl;
    exists $perl{$p} or $p = "perl$perl";
    exists $perl{$p} or $p = "perl-$perl";
    exists $perl{$p} or die "Cannot find perl: $perl\n";
    push @perls, $p;
  }
}
else {
  @perls = sort keys %perl;
}

if ($opt{'test-archives'}) {
  my $test = 'test';
  my $cwd = cwd;
  -d $test or mkpath($test);
  chdir $test or die "chdir $test: $!\n";
  for my $perl (@perls) {
    eval {
      my $d = extract_source($perl{$perl});
      if ($opt{'test-archives'} > 2) {
        my $cwd2 = cwd;
        chdir $d or die "chdir $d: $!\n";
        patch_source($perl{$perl}{version});
        chdir $cwd2 or die "chdir $cwd2:$!\n"
      }
      rmtree($d) if -e $d;
    };
    warn $@ if $@;
  }
  chdir $cwd or die "chdir $cwd: $!\n";
  print STDERR "cleaning up\n";
  rmtree($test);
  exit 0;
}

for my $cfg (@{$opt{config}}) {
  for my $perl (@perls) {
    my $config = $config{$cfg};
    %current = (config => $cfg, perl => $perl, version => $perl{$perl}{version});

    if (is($config->{masked_versions}, $current{version})) {
      print STDERR "skipping $perl for configuration $cfg (masked)\n";
      next;
    }

    if (-d expand($opt{prefix}) and !$opt{force}) {
      print STDERR "skipping $perl for configuration $cfg (already installed)\n";
      next;
    }

    my $cwd = cwd;

    my $build = expand($opt{build});
    -d $build or mkpath($build);
    chdir $build or die "chdir $build: $!\n";

    print STDERR "building $perl with configuration $cfg\n";
    buildperl($perl, $config);

    chdir $cwd or die "chdir $cwd: $!\n";
  }
}

sub expand
{
  my $in = shift;
  $in =~ s/(<(\w+)>)/exists $current{$2} ? $current{$2} : $1/eg;
  return $in;
}

sub is
{
  my($s1, $s2) = @_;

  defined $s1 != defined $s2 and return 0;

  ref $s2 and ($s1, $s2) = ($s2, $s1);

  if (ref $s1) {
    if (ref $s1 eq 'ARRAY') {
      is($_, $s2) and return 1 for @$s1;
      return 0;
    }
    return $s2 =~ $s1;
  }

  return $s1 eq $s2;
}

sub buildperl
{
  my($perl, $cfg) = @_;

  my $d = extract_source($perl{$perl});
  chdir $d or die "chdir $d: $!\n";

  patch_source($perl{$perl}{version});

  build_and_install($perl{$perl});
}

sub extract_source
{
  eval { require Archive::Tar };
  die "Archive processing requires Archive::Tar to be installed\n" if $@;

  my $perl = shift;

  my $what = $opt{'test-archives'} ? 'test' : 'read';
  print "${what}ing $perl->{source}\n";

  my $target;

  for my $f (Archive::Tar->list_archive($perl->{source})) {
    my($t) = $f =~ /^([^\\\/]+)/ or die "ooops, should always match...\n";
    die "refusing to extract $perl->{source}, as it would not extract to a single directory\n"
        if defined $target and $target ne $t;
    $target = $t;
  }

  if ($opt{'test-archives'} == 0 || $opt{'test-archives'} > 1) {
    if (-d $target) {
      print "removing old build directory $target\n";
      rmtree($target);
    }

    print "extracting $perl->{source}\n";

    Archive::Tar->extract_archive($perl->{source})
        or die "extract failed: " . Archive::Tar->error() . "\n";

    -d $target or die "oooops, $target not found\n";
  }

  return $target;
}

sub patch_source
{
  my $version = shift;

  for my $p (@patch) {
    if (is($p->{perl}, $version)) {
      for my $s (@{$p->{subs}}) {
        my($sub, @args) = @$s;
        $sub->(@args);
      }
    }
  }
}

sub build_and_install
{
  my $perl = shift;
  my $prefix = expand($opt{prefix});

  run_or_die(q{sed -i -e "s:\\*/\\*) finc=\\"-I\\`echo \\$file | sed 's#/\\[^/\\]\\*\\$##\\`\\" ;;:*/*) finc=\\"-I\\`echo \\$file | sed 's#/[^/]\\*\\$##'\\`\\" ;;:" makedepend.SH});

  print "building perl $perl->{version} ($current{config})\n";

  run_or_die("./Configure $config{$current{config}}{config_args} -Dusedevel -Uinstallusrbinperl -Dprefix=$prefix");
  if (-f "x2p/makefile") {
    run_or_die("sed -i -e '/^.*<builtin>/d' -e '/^.*<built-in>/d' -e '/^.*<command line>/d' -e '/^.*<command-line>/d' makefile x2p/makefile");
  }
  run_or_die("make $job_string all");
  run("TEST_JOBS=$opt{jobs} make $job_string test") if $opt{test};
  if ($opt{install}) {
    run("make $job_string install");
  }
  else {
    print "\n*** NOT INSTALLING PERL ***\n\n";
  }
}

sub patch_db
{
  my $ver = shift;
  print "patching ext/DB_File/DB_File.xs\n";
  run_or_die("sed -i -e 's/<db.h>/<db$ver\\/db.h>/' ext/DB_File/DB_File.xs");
}

sub patch_doio
{
  patch(<<'END');
--- doio.c.org	2004-06-07 23:14:45.000000000 +0200
+++ doio.c	2003-11-04 08:03:03.000000000 +0100
@@ -75,6 +75,16 @@
 #  endif
 #endif

+#if _SEM_SEMUN_UNDEFINED
+union semun
+{
+  int val;
+  struct semid_ds *buf;
+  unsigned short int *array;
+  struct seminfo *__buf;
+};
+#endif
+
 bool
 do_open(gv,name,len,as_raw,rawmode,rawperm,supplied_fp)
 GV *gv;
END
}

sub patch_sysv
{
  my %opt = @_;

  # check if patching is required
  return if $^O ne 'linux' or -f '/usr/include/asm/page.h';

  if ($opt{old_format}) {
    patch(<<'END');
--- ext/IPC/SysV/SysV.xs.org	1998-07-20 10:20:07.000000000 +0200
+++ ext/IPC/SysV/SysV.xs	2007-08-12 10:51:06.000000000 +0200
@@ -3,9 +3,6 @@
 #include "XSUB.h"
 
 #include <sys/types.h>
-#ifdef __linux__
-#include <asm/page.h>
-#endif
 #if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
 #include <sys/ipc.h>
 #ifdef HAS_MSG
END
  }
  else {
    patch(<<'END');
--- ext/IPC/SysV/SysV.xs.org	2007-08-11 00:12:46.000000000 +0200
+++ ext/IPC/SysV/SysV.xs	2007-08-11 00:10:51.000000000 +0200
@@ -3,9 +3,6 @@
 #include "XSUB.h"
 
 #include <sys/types.h>
-#ifdef __linux__
-#   include <asm/page.h>
-#endif
 #if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
 #ifndef HAS_SEM
 #   include <sys/ipc.h>
END
  }
}

sub patch_configure
{
  patch(<<'END');
--- Configure
+++ Configure
@@ -3380,6 +3380,18 @@
 test "X$gfpthkeep" != Xy && gfpth=""
 EOSC
 
+# gcc 3.1 complains about adding -Idirectories that it already knows about,
+# so we will take those off from locincpth.
+case "$gccversion" in
+3*)
+    echo "main(){}">try.c
+    for incdir in `$cc -v -c try.c 2>&1 | \
+       sed '1,/^#include <\.\.\.>/d;/^End of search list/,$d;s/^ //'` ; do
+       locincpth=`echo $locincpth | sed s!$incdir!!`
+    done
+    $rm -f try try.*
+esac
+
 : What should the include directory be ?
 echo " "
 $echo $n "Hmm...  $c"
END
}

sub patch_makedepend_lc
{
  patch(<<'END');
--- makedepend.SH
+++ makedepend.SH
@@ -58,6 +58,10 @@ case $PERL_CONFIG_SH in
       ;;
 esac
 
+# Avoid localized gcc/cc messages
+LC_ALL=C
+export LC_ALL
+
 # We need .. when we are in the x2p directory if we are using the
 # cppstdin wrapper script.
 # Put .. and . first so that we pick up the present cppstdin, not
END
}

sub patch
{
  my($patch) = @_;
  print "patching $_\n" for $patch =~ /^\+{3}\s+(\S+)/gm;
  my $diff = 'tmp.diff';
  write_or_die($diff, $patch);
  run_or_die("patch -s -p0 <$diff");
  unlink $diff or die "unlink $diff: $!\n";
}

sub write_or_die
{
  my($file, $data) = @_;
  my $fh = new IO::File ">$file" or die "$file: $!\n";
  $fh->print($data);
}

sub run_or_die
{
  # print "[running @_]\n";
  system "@_" and die "@_: $?\n";
}

sub run
{
  # print "[running @_]\n";
  system "@_" and warn "@_: $?\n";
}

__END__

=head1 NAME

buildperl.pl - build/install perl distributions

=head1 SYNOPSIS

  perl buildperl.pl [options]

  --help                      show this help

  --source=directory          directory containing source tarballs
                              [default: /tmp/perl/source]

  --build=directory           directory used for building perls [EXPAND]
                              [default: /tmp/perl/build/<config>]

  --prefix=directory          use this installation prefix [EXPAND]
                              [default:
                              /tmp/perl/install/<config>/<perl>]

  --config=configuration      build this configuration [MULTI]
                              The possibilities for this parameter are:
                                'thread', 'thread5005', 'debug';
                                 and 'default',
                                 which means none of the others.
                              [default: all possible configurations]

  --perl=version              build this version of perl [MULTI]
                              [default: all possible versions]

  --force                     rebuild and install already installed
                              versions

  --test                      run test suite after building

  --noinstall                 don't install after building

  --patch                     only patch the perl source in the current
                              directory

  --oneshot                   build from the perl source in the current
                              directory (extra arguments are passed to
                              Configure)

  -j N                        Build and test with N parallel jobs
                              [default: 1]

  options tagged with [MULTI] can be given multiple times

  options tagged with [EXPAND] expand the following items

    <perl>      versioned perl directory  (e.g. 'perl-5.6.1')
    <version>   perl version              (e.g. '5.6.1')
    <config>    name of the configuration (e.g. 'default')

=head1 EXAMPLES

The following examples assume that your Perl source tarballs are
in F</tmp/perl/source>. If they are somewhere else, use the C<--source>
option to specify a different source directory.

To build a default configuration of perl5.004_05 and install it
to F</opt/perl5.004_05>, you would say:

  buildperl.pl --prefix='/opt/<perl>' --perl=5.004_05 --config=default

To build debugging configurations of all perls in the source directory
and install them to F</opt>, use:

  buildperl.pl --prefix='/opt/<perl>' --config=debug

To build all configurations for perl-5.8.5 and perl-5.8.6, test them
and don't install them, run:

  buildperl.pl --perl=5.8.5 --perl=5.8.6 --test --noinstall

To build and install a single version of perl with special configuration
options, use:

  buildperl.pl --perl=5.6.0 --prefix=/opt/p560ld --oneshot -- -des \
                                                   -Duselongdouble

=head1 COPYRIGHT

Copyright (c) 2004-2013, Marcus Holland-Moritz.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=head1 SEE ALSO

See L<Devel::PPPort> and L<HACKERS>.
