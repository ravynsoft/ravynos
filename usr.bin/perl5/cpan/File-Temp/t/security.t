#!/usr/bin/perl -w
# Test for File::Temp - Security levels

# Some of the security checking will not work on all platforms
# Test a simple open in the cwd and tmpdir foreach of the
# security levels

use Test::More tests => 12;

use strict;
use File::Spec;

# Set up END block - this needs to happen before we load
# File::Temp since this END block must be evaluated after the
# END block configured by File::Temp
my @files; # list of files to remove
END { foreach (@files) { ok( !(-e $_) )} }

use File::Temp qw/ tempfile unlink0 /;

# The high security tests must currently be skipped on some platforms
my $skipplat = ( (
		  # No sticky bits.
		  $^O eq 'MSWin32' || $^O eq 'NetWare' || $^O eq 'os2' || $^O eq 'dos' || $^O eq 'mpeix' || $^O eq 'MacOS'
		  ) ? 1 : 0 );

# Can not run high security tests in perls before 5.6.0
my $skipperl  = ($] < 5.006 ? 1 : 0 );

# Determine whether we need to skip things and why
my $skip = 0;
if ($skipplat) {
  $skip = "Not supported on this platform";
} elsif ($skipperl) {
  $skip = "Perl version must be v5.6.0 for these tests";

}

print "# We will be skipping some tests : $skip\n" if $skip;

# start off with basic checking

File::Temp->safe_level( File::Temp::STANDARD );

print "# Testing with STANDARD security...\n";

test_security();

SKIP: {
  skip $skip, 8 if $skip;

  # Try medium

  File::Temp->safe_level( File::Temp::MEDIUM );

  print "# Testing with MEDIUM security...\n";

  # Now we need to start skipping tests
  test_security();

  # Try HIGH

  File::Temp->safe_level( File::Temp::HIGH );

  print "# Testing with HIGH security...\n";

  test_security();
}

exit;

# Subroutine to open two temporary files.
# one is opened in the current dir and the other in the temp dir

sub test_security {

  # Create the tempfile
  my $template = "tmpXXXXX";
  my ($fh1, $fname1) = eval { tempfile ( $template, 
				  DIR => File::Temp::_wrap_file_spec_tmpdir(),
				  UNLINK => 1,
				);
			    };

  SKIP: {
    if (defined $fname1) {
        print "# fname1 = $fname1\n";
        ok( (-e $fname1) );
        push(@files, $fname1); # store for end block
    } elsif (File::Temp->safe_level() != File::Temp::STANDARD) {
        chomp($@);
        my $msg = File::Temp::_wrap_file_spec_tmpdir() . " possibly insecure: $@";
        skip $msg, 2; # one here and one in END
    } else {
        ok(0);
    }
  }

  SKIP: {
    # Explicitly 
    if ( $< < File::Temp->top_system_uid() ){
        skip("Skip Test inappropriate for root", 2);
        return;
    }
    my ($fh2, $fname2) = eval { tempfile ($template,  UNLINK => 1 ); };
    if (defined $fname2) {
        print "# fname2 = $fname2\n";
        ok( (-e $fname2) );
        push(@files, $fname2); # store for end block
        close($fh2);
    } elsif (File::Temp->safe_level() != File::Temp::STANDARD) {
        chomp($@);
        my $msg = "current directory possibly insecure: $@";
        skip $msg, 2; # one here and one in END
    } else {
        ok(0);
    }
  }
}

# vim: ts=2 sts=2 sw=2 et:
