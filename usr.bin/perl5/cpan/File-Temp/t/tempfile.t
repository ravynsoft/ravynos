#!/usr/local/bin/perl -w
# Test for File::Temp - tempfile function

use strict;
use Test::More tests => 30;
use File::Spec;
use Cwd qw/ cwd /;

# Will need to check that all files were unlinked correctly
# Set up an END block here to do it

# Arrays containing list of dirs/files to test
my (@files, @dirs, @still_there);

# And a test for files that should still be around
# These are tidied up
END {
  foreach (@still_there) {
    ($_) = /(^.*)/; # untaint for testing under taint mode
    ok( -f $_, "File $_ is still present" );
    ok( unlink( $_ ), "Unlink file" );
    ok( !(-f $_), "File is no longer present" );
  }
}

# Loop over an array hoping that the files dont exist
END { foreach (@files) { ok( !(-e $_), "File $_ should not be present" )} }

# And a test for directories
END { foreach (@dirs)  { ok( !(-d $_), "Dir $_ should not be present" )} }

# Need to make sure that the END blocks are setup before
# the ones that File::Temp configures since END blocks are evaluated
# in revers order and we need to check the files *after* File::Temp
# removes them
use File::Temp qw/ tempfile tempdir/;

# Now we start the tests properly
ok(1, "Start test");


# Tempfile
# Open tempfile in some directory, unlink at end
my ($fh, $tempfile) = tempfile(
			       UNLINK => 1,
			       SUFFIX => '.txt',
			      );

ok( (-f $tempfile), "Tempfile exists" );
# Should still be around after closing
ok( close( $fh ), "Tempfile closed" );
ok( (-f $tempfile), "Tempfile exists" );
# Check again at exit
push(@files, $tempfile);

# TEMPDIR test
# Create temp directory in current dir
my $template = 'tmpdirXXXXXX';
print "# Template: $template\n";
my $tempdir = tempdir( $template ,
		       DIR => File::Spec->curdir,
		       CLEANUP => 1,
		     );

print "# TEMPDIR: $tempdir\n";

ok( (-d $tempdir), "Local tempdir exists" );
push(@dirs, File::Spec->rel2abs($tempdir));

my $tempdir2 = tempdir( TEMPLATE => "customXXXXX",
		       DIR => File::Spec->curdir,
		       CLEANUP => 1,
		     );

print "# TEMPDIR2: $tempdir2\n";

like( $tempdir2, qr/custom/, "tempdir with TEMPLATE" );
push(@dirs, File::Spec->rel2abs($tempdir));

# Create file in the temp dir
($fh, $tempfile) = tempfile(
			    DIR => $tempdir,
			    UNLINK => 1,
			    SUFFIX => '.dat',
			   );

print "# TEMPFILE: Created $tempfile\n";

ok( (-f $tempfile), "Local temp file exists with .dat extension");
push(@files, File::Spec->rel2abs($tempfile));

# Test tempfile
# ..and again
($fh, $tempfile) = tempfile(
			    DIR => $tempdir,
			   );


ok( (-f $tempfile ), "Local tempfile in tempdir exists");
push(@files, File::Spec->rel2abs($tempfile));

# Test tempfile
# ..and another with default permissions
($fh, $tempfile) = tempfile(
			    DIR => $tempdir,
			   );

# From perlport on chmod:
#
#     (Win32) Only good for changing "owner" read-write access;
#     "group" and "other" bits are meaningless.
#
# So we don't check the full permissions -- it will be 0444 on Win32
# instead of 0400.  Instead, just check the owner bits.

is((stat($tempfile))[2] & 00700, 0600, 'created tempfile with default permissions');
push(@files, File::Spec->rel2abs($tempfile));

# Test tempfile
# ..and another with changed permissions
($fh, $tempfile) = tempfile(
			    DIR => $tempdir,
			    PERMS => 0400,
			   );

is((stat($tempfile))[2] & 00700, 0400, 'created tempfile with changed permissions');
push(@files, File::Spec->rel2abs($tempfile));

print "# TEMPFILE: Created $tempfile\n";

# and another (with template)

($fh, $tempfile) = tempfile( 'helloXXXXXXX',
			    DIR => $tempdir,
			    UNLINK => 1,
			    SUFFIX => '.dat',
			   );

print "# TEMPFILE: Created $tempfile\n";

ok( (-f $tempfile), "Local tempfile in tempdir with .dat extension exists" );
push(@files, File::Spec->rel2abs($tempfile));


# and another (with TEMPLATE)

($fh, $tempfile) = tempfile( TEMPLATE => 'goodbyeXXXXXXX',
			    DIR => $tempdir,
			    UNLINK => 1,
			    SUFFIX => '.dat',
			   );

print "# TEMPFILE: Created $tempfile\n";

ok( (-f $tempfile), "Local tempfile in tempdir with TEMPLATE" );
push(@files, File::Spec->rel2abs($tempfile));

# Create a temporary file that should stay around after
# it has been closed
($fh, $tempfile) = tempfile( 'permXXXXXXX', UNLINK => 0 );
print "# TEMPFILE: Created $tempfile\n";
ok( -f $tempfile, "Long-lived temp file" );
ok( close( $fh ), "Close long-lived temp file" );
push( @still_there, File::Spec->rel2abs($tempfile) ); # check at END

# Would like to create a temp file and just retrieve the handle
# but the test is problematic since:
#  - We dont know the filename so we cant check that it is tidied
#    correctly
#  - The unlink0 required on unix for tempfile creation will fail
#    on NFS
# Try to do what we can.
# Tempfile croaks on error so we need an eval
$fh = eval { tempfile( 'ftmpXXXXX', DIR => File::Temp::_wrap_file_spec_tmpdir() ) };

if ($fh) {

  # print something to it to make sure something is there
  ok( print($fh "Test\n"), "Write to temp file" );

  # Close it - can not check it is gone since we dont know the name
  ok( close($fh), "Close temp file" );

} else {
    SKIP: {
        skip "Skip Failed probably due to NFS", 2;
    }
}

# Create temp directory and chdir to it; it should still be removed on exit.
$tempdir = tempdir(CLEANUP => 1);

print "# TEMPDIR: $tempdir\n";

ok( (-d $tempdir), "Temp directory in temp dir" );
chdir $tempdir or die $!;
push(@dirs, File::Spec->rel2abs($tempdir));

# Now END block will execute to test the removal of directories
print "# End of tests. Execute END blocks in directory ". cwd() ."\n";

