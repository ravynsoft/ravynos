BEGIN { chdir 't' if -d 't' }

use Test::More;
use strict;
use lib '../lib';

plan skip_all => "Files contain an alien character set" if ord "A" != 65;

use File::Spec ();
use File::Temp qw( tempfile );

use Archive::Tar;

BEGIN {
  eval { require IPC::Cmd; };
  unless ( $@ ) {
    *can_run = \&IPC::Cmd::can_run;
  }
  else {
    *can_run = sub {
        require ExtUtils::MakeMaker;
        my $cmd = shift;
        my $_cmd = $cmd;
        return $_cmd if (-x $_cmd or $_cmd = MM->maybe_command($_cmd));
        require Config;
        for my $dir ((split /$Config::Config{path_sep}/, $ENV{PATH}), '.') {
          next if $dir eq '';
          require File::Spec;
          my $abs = File::Spec->catfile($dir, $cmd, $Config::Config{exe_ext});
          return $abs if (-x $abs or $abs = MM->maybe_command($abs));
        }
        return;
    };
  }
}

# Identify tarballs available for testing
# Some contain only files
# Others contain both files and directories

my @file_only_archives = (
  [qw( src short bar.tar )],
);
push @file_only_archives, [qw( src short foo.tgz )]
  if Archive::Tar->has_zlib_support;
push @file_only_archives, [qw( src short foo.tbz )]
  if Archive::Tar->has_bzip2_support;
push @file_only_archives, [qw( src short foo.txz )]
  if Archive::Tar->has_xz_support;

@file_only_archives = map File::Spec->catfile(@$_), @file_only_archives;


my @file_and_directory_archives = (
    [qw( src long bar.tar )],
    [qw( src linktest linktest_with_dir.tar )],
);
push @file_and_directory_archives, [qw( src long foo.tgz )]
  if Archive::Tar->has_zlib_support;
push @file_and_directory_archives, [qw( src long foo.tbz )]
  if Archive::Tar->has_bzip2_support;

@file_and_directory_archives = map File::Spec->catfile(@$_), @file_and_directory_archives;

my @archives = (@file_only_archives, @file_and_directory_archives);
plan tests => scalar @archives;

# roundtrip test
for my $archive_name (@file_only_archives) {

      # create a new tarball with the same content as the old one
      my $old = Archive::Tar->new($archive_name);
      my $new = Archive::Tar->new();
      $new->add_files( $old->get_files );

      # save differently if compressed
      my $ext = ( split /\./, $archive_name )[-1];
      my @compress =
          $ext =~ /t?gz$/       ? (COMPRESS_GZIP)
        : $ext =~ /(tbz|bz2?)$/ ? (COMPRESS_BZIP)
        : $ext =~ /(t?xz)$/     ? (COMPRESS_XZ)
        : ();

      my ( $fh, $filename ) = tempfile( UNLINK => 1 );
      $new->write( $filename, @compress );

      # read the archive again from disk
      $new = Archive::Tar->new($filename);

      # compare list of files
      is_deeply(
          [ $new->list_files ],
          [ $old->list_files ],
          "$archive_name roundtrip on file names"
      );
}

# rt.cpan.org #115160
# t/09_roundtrip.t was added with all 7 then existent tests marked TODO even
# though 3 of them were passing.  So what was really TODO was to figure out
# why the other 4 were not passing.
#
# It turns out that the tests are expecting behavior which, though on the face
# of it plausible and desirable, is not Archive::Tar::write()'s current
# behavior.  write() -- which is used in the unit tests in this file -- relies
# on Archive::Tar::File::_prefix_and_file().  Since at least 2006 this helper
# method has had the effect of removing a trailing slash from archive entries
# which are in fact directories.  So we have to adjust our expectations for
# what we'll get when round-tripping on an archive which contains one or more
# entries for directories.

# Divine whether the external tar command can do gzip/bzip2
# from the output of 'tar --help'.
# GNU tar:
# ...
# -j, --bzip2                filter the archive through bzip2
# -z, --gzip, --gunzip, --ungzip   filter the archive through gzip
#
# BSD tar:
# ....
#   -z, -j, -J, --lzma  Compress archive with gzip/bzip2/xz/lzma
# ...
#
# BSD tar (older)
# tar: unknown option -- help
# usage: tar [-]{crtux}[-befhjklmopqvwzHOPSXZ014578] [archive] [blocksize]
# ...

sub can_tar_gzip {
  my ($tar_help) = @_;
  return 0 unless can_run('gzip');
  $tar_help =~ /-z, --gzip|-z,.+gzip/;
}

sub can_tar_bzip2 {
  my ($tar_help) = @_;
  return 0 unless can_run('bzip2');
  $tar_help =~ /-j, --bzip2|-j,+bzip2/;
}

# The name of the external tar executable.
my $TAR_EXE;

SKIP: {
  my $skip_count = scalar @file_and_directory_archives;

  # The preferred 'tar' command may not be called tar,:
  # especially on legacy unix systems.  Test first various
  # alternative names that are more likely to work for us.
  #
  my @TRY_TAR = qw[gtar gnutar bsdtar tar];
  my $can_tar_gzip;
  my $can_tar_bzip2;
  for my $tar_try (@TRY_TAR) {
    if (can_run($tar_try)) {
      print "# Found tar executable '$tar_try'\n";
      my $tar_help = qx{$tar_try --help 2>&1};
      $can_tar_gzip  = can_tar_gzip($tar_help);
      $can_tar_bzip2 = can_tar_bzip2($tar_help);
      printf "# can_tar_gzip  = %d\n", $can_tar_gzip;
      printf "# can_tar_bzip2 = %d\n", $can_tar_bzip2;
      # We could dance more intricately and handle the case
      # of only either of gzip and bzip2 being supported,
      # or neither, but let's keep this simple.
      if ($can_tar_gzip && $can_tar_bzip2) {
        $TAR_EXE = $tar_try;
        last;
      }
    }
  }
  unless (defined $TAR_EXE) {
    skip("No suitable tar command found (tried: @TRY_TAR)", $skip_count);
  }

  for my $archive_name (@file_and_directory_archives) {
    if ($^O eq 'VMS' && $TAR_EXE =~ m/gnutar$/i) {
      $archive_name = VMS::Filespec::unixify($archive_name);
    }
    my $command;
    if ($archive_name =~ m/\.tar$/) {
      $command = "$TAR_EXE tvf $archive_name";
    }
    elsif ($archive_name =~ m/\.tgz$/) {
      $command = "$TAR_EXE tzvf $archive_name";
    }
    elsif ($archive_name =~ m/\.tbz$/) {
      $command = "$TAR_EXE tjvf $archive_name";
    }
    print "# command = '$command'\n";
    my @contents = qx{$command};
    if ($?) {
      fail("Failed running '$command'");
    } else {
      chomp(@contents);
      my @directory_or_not;
      for my $entry (@contents) {
        my $perms = (split(/\s+/ => $entry))[0];
        my @chars = split('' => $perms);
            push @directory_or_not,
          ($chars[0] eq 'd' ? 1 : 0);
      }

      # create a new tarball with the same content as the old one
      my $old = Archive::Tar->new($archive_name);
      my $new = Archive::Tar->new();
      $new->add_files( $old->get_files );

      # save differently if compressed
      my $ext = ( split /\./, $archive_name )[-1];
      my @compress =
        $ext =~ /t?gz$/       ? (COMPRESS_GZIP)
          : $ext =~ /(tbz|bz2?)$/ ? (COMPRESS_BZIP)
          : ();

      my ( $fh, $filename ) = tempfile( UNLINK => 1 );
      $new->write( $filename, @compress );

      # read the archive again from disk
      $new = Archive::Tar->new($filename);

      # Adjust our expectations of
      my @oldfiles = $old->list_files;
      for (my $i = 0; $i <= $#oldfiles; $i++) {
        chop $oldfiles[$i] if $directory_or_not[$i];
      }

      # compare list of files
      is_deeply(
                [ $new->list_files ],
                [ @oldfiles ],
                "$archive_name roundtrip on file names"
               );
    }
  }
}
