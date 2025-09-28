BEGIN { chdir 't' if -d 't' }

use Test::More      'no_plan';
use File::Basename  'basename';
use strict;
use lib '../lib';

my $NO_UNLINK   = @ARGV ? 1 : 0;

my $Class       = 'Archive::Tar';
my $FileClass   = $Class . '::File';

use_ok( $Class );
use_ok( $FileClass );

### bug #13636
### tests for @longlink behaviour on files that have a / at the end
### of their shortened path, making them appear to be directories
{   ok( 1,                      "Testing bug 13636" );

    ### dont use the prefix, otherwise A::T will not use @longlink
    ### encoding style
    local $Archive::Tar::DO_NOT_USE_PREFIX = 1;
    local $Archive::Tar::DO_NOT_USE_PREFIX = 1;

    my $dir =   'Catalyst-Helper-Controller-Scaffold-HTML-Template-0_03/' .
                'lib/Catalyst/Helper/Controller/Scaffold/HTML/';
    my $file =  'Template.pm';
    my $out =   $$ . '.tar';

    ### first create the file
    {   my $tar = $Class->new;

        isa_ok( $tar, $Class,   "   Object" );
        ok( $tar->add_data( $dir.$file => $$ ),
                                "       Added long file" );

        ok( $tar->write($out),  "       File written to $out" );
    }

    ### then read it back in
    {   my $tar = $Class->new;
        isa_ok( $tar, $Class,   "   Object" );
        ok( $tar->read( $out ), "       Read in $out again" );

        my @files = $tar->get_files;
        is( scalar(@files), 1,  "       Only 1 entry found" );

        my $entry = shift @files;
        ok( $entry->is_file,    "       Entry is a file" );
        is( $entry->name, $dir.$file,
                                "       With the proper name" );
    }

    ### remove the file
    unless( $NO_UNLINK ) { 1 while unlink $out }
}

### bug #14922
### There's a bug in Archive::Tar that causes a file like: foo/foo.txt
### to be stored in the tar file as: foo/.txt
### XXX could not be reproduced in 1.26 -- leave test to be sure
{   ok( 1,                      "Testing bug 14922" );

    my $dir     = $$ . '/';
    my $file    = $$ . '.txt';
    my $out     = $$ . '.tar';

    ### first create the file
    {   my $tar = $Class->new;

        isa_ok( $tar, $Class,   "   Object" );
        ok( $tar->add_data( $dir.$file => $$ ),
                                "       Added long file" );

        ok( $tar->write($out),  "       File written to $out" );
    }

    ### then read it back in
    {   my $tar = $Class->new;
        isa_ok( $tar, $Class,   "   Object" );
        ok( $tar->read( $out ), "       Read in $out again" );

        my @files = $tar->get_files;
        is( scalar(@files), 1,  "       Only 1 entry found" );

        my $entry = shift @files;
        ok( $entry->is_file,    "       Entry is a file" );
        is( $entry->full_path, $dir.$file,
                                "       With the proper name" );
    }

    ### remove the file
    unless( $NO_UNLINK ) { 1 while unlink $out }
}

### bug #30380: directory traversal vulnerability in Archive-Tar
### Archive::Tar allowed files to be extracted to a dir outside
### it's cwd(), effectively allowing you to overwrite any files
### on the system, given the right permissions.
{   ok( 1,                      "Testing bug 30880" );

    my $tar = $Class->new;
    isa_ok( $tar, $Class,       "   Object" );

    ### absolute paths are already taken care of. Only relative paths
    ### matter
    my $in_file     = basename($0);
    my $out_file    = '../' . $in_file . "_$$";

    ok( $tar->add_files( $in_file ),
                                "       Added '$in_file'" );

    ok( $tar->chmod( $in_file, '1777'),
                                "       chmod 177 $in_file" );

    ok( $tar->chown( $in_file, 'root' ),
                                "       chown to root" );

    ok( $tar->chown( $in_file, 'root', 'root' ),
                                "       chown to root:root" );

    ok( $tar->rename( $in_file, $out_file ),
                                "       Renamed to '$out_file'" );

    ### first, test with strict extract permissions on
    {   local $Archive::Tar::INSECURE_EXTRACT_MODE = 0;

        ### we quell the error on STDERR
        local $Archive::Tar::WARN = 0;
        local $Archive::Tar::WARN = 0;

        ok( 1,                  "   Extracting in secure mode" );

        ok( ! $tar->extract_file( $out_file ),
                                "       File not extracted" );
        ok( ! -e $out_file,     "       File '$out_file' does not exist" );

        ok( $tar->error,        "       Error message stored" );
        like( $tar->error, qr/attempting to leave/,
                                "           Proper violation detected" );
    }

    ### now disable those
    {   local $Archive::Tar::INSECURE_EXTRACT_MODE = 1;
        ok( 1,                  "   Extracting in insecure mode" );

        ok( $tar->extract_file( $out_file ),
                                "       File extracted" );
        ok( -e $out_file,       "       File '$out_file' exists" );

        ### and clean up
        unless( $NO_UNLINK ) { 1 while unlink $out_file };
    }
}

### bug #43513: [PATCH] Accept wrong checksums from SunOS and HP-UX tar
### like GNU tar does. See here for details:
### http://www.gnu.org/software/tar/manual/tar.html#SEC139
SKIP: {
    skip "File contains an alien character set", 5 if ord "A" != 65;

    ok( 1,                      "Testing bug 43513" );

    my $src = File::Spec->catfile( qw[src header signed.tar] );
    my $tar = $Class->new;

    isa_ok( $tar, $Class,       "   Object" );
    ok( $tar->read( $src ),     "   Read non-Posix file with signed Checksum" );

    for my $file ( $tar->get_files ) {
        ok( $file,              "       File object retrieved" );
        ok( $file->validate,    "           File validates" );
    }
}

### return error properly on corrupted archives
### Addresses RT #44680: Improve error reporting on short corrupted archives
{   ok( 1,                      "Testing bug 44680" );

    {   ### XXX whitebox test -- resetting the error string
        no warnings 'once';
        $Archive::Tar::error = "";
    }

    my $src = File::Spec->catfile( qw[src short b] );
    my $tar = $Class->new;

    isa_ok( $tar, $Class,       "   Object" );


    ### we quell the error on STDERR
    local $Archive::Tar::WARN = 0;

    ok( !$tar->read( $src ),    "   No files in the corrupted archive" );
    like( $tar->error, qr/enough bytes/,
                                "       Expected error reported" );
}

### bug #78030
### tests for symlinks with relative paths
### seen on MSWin32
{   ok( 1,                      "Testing bug 78030" );
		my $archname = 'tmp-symlink.tar.gz';
		{	#build archive
			unlink $archname if -e $archname;
			local $Archive::Tar::DO_NOT_USE_PREFIX = 1;
			my $t=Archive::Tar->new;
			my $f = $t->add_data( 'tmp/a/b/link.txt', '',
				{
					linkname => '../c/ori.txt',
					type     => 2,
				} );
			#why doesn't it keep my wish?
			$f->{name}   = 'tmp/a/b/link.txt';
			$f->{prefix} = '';
			$t->add_data( 'tmp/a/c/ori.txt', 'test case' );
			$t->write( $archname, 1 );
		}

    { #use case 1 - in memory extraction
			my $t=Archive::Tar->new;
			$t->read( $archname );
			my $r = eval{ $t->extract };
			ok( $r && !$@,            "   In memory extraction/symlinks" );
			ok((stat 'tmp/a/b/link.txt')[7] == 9,
			                          "       Linked content" ) unless $r;
			clean_78030();
		}

		{ #use case 2 - iter extraction
		  #$DB::single = 2;
			my $next=Archive::Tar->iter( $archname, 1 );
			my $failed = 0;
			#use Data::Dumper;
			while(my $f = $next->() ){
			#  print "\$f = ", Dumper( $f ), $/;
				eval{ $f->extract } or $failed++;
			}
			ok( !$failed,             "   From disk extraction/symlinks" );
			ok((stat 'tmp/a/b/link.txt')[7] == 9,
			                          "       Linked content" ) unless $failed;
		}

    #remove tmp files
		sub clean_78030{
			unlink for ('tmp/a/c/ori.txt', 'tmp/a/b/link.txt');
			rmdir for ('tmp/a/c', 'tmp/a/b', 'tmp/a', 'tmp');
		}
		clean_78030();
		unlink $archname;
}

### bug 97748
### retain leading '/' for absolute pathnames.
{   ok( 1,                      "Testing bug 97748" );
	my $path= '/absolute/path';
	my $tar = $Class->new;
	isa_ok( $tar, $Class,       "   Object" );
	my $file;

	ok( $file = $tar->add_data( $path, '' ),
		"       Added $path" );

	ok( $file->full_path eq $path,
		"	Paths mismatch <" . $file->full_path . "> ne <$path>" );
}

### bug 103279
### retain trailing whitespace on filename
{
  ok( 1,                      "Testing bug 103279" );
	my $tar = $Class->new;
	isa_ok( $tar, $Class,       "   Object" );
	ok( $tar->add_data( 'white_space   ', '' ),
				    "   Add file <white_space   > containing filename with trailing whitespace");
	ok( $tar->extract(),        "	Extract filename with trailing whitespace" );
  SKIP: {
    skip "Windows tries to be clever", 1 if $^O eq 'MSWin32';
	  ok( ! -e 'white_space',     "	<white_space> should not exist" );
  }
	ok( -e 'white_space   ',    "	<white_space   > should exist" );
	unlink foreach ('white_space   ', 'white_space');
}
