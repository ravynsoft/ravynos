#!./perl
use strict;
use Cwd;

my $warn_msg;

BEGIN {
    require File::Spec;
    if ($ENV{PERL_CORE}) {
        # May be doing dynamic loading while @INC is all relative
        @INC = map { $_ = File::Spec->rel2abs($_); /(.*)/; $1 } @INC;
    }
    $SIG{'__WARN__'} = sub { $warn_msg = $_[0]; warn "# $_[0]"; };

    if ($^O eq 'MSWin32' || $^O eq 'cygwin' || $^O eq 'VMS') {
        # This is a hack - at present File::Find does not produce native names
        # on Win32 or VMS, so force File::Spec to use Unix names.
        # must be set *before* importing File::Find
        require File::Spec::Unix;
        @File::Spec::ISA = 'File::Spec::Unix';
    }
    require File::Find;
    import File::Find;
}

my $symlink_exists = eval { symlink("",""); 1 };

use Test::More;
use lib qw( ./t/lib );
use Testing qw(
    create_file_ok
    mkdir_ok
    symlink_ok
    dir_path
    file_path
    _cleanup_start
);
use Errno ();
use File::Temp qw(tempdir);

my %Expect_File = (); # what we expect for $_
my %Expect_Name = (); # what we expect for $File::Find::name/fullname
my %Expect_Dir  = (); # what we expect for $File::Find::dir
my (@files);

my $test_root_dir = cwd();
ok($test_root_dir,"We were able to determine our starting directory");
my $test_temp_dir = tempdir("FF_find_t_XXXXXX",CLEANUP=>1);
ok($test_temp_dir,"We were able to set up a temp directory");


# Uncomment this to see where File::Find is chdir-ing to.  Helpful for
# debugging its little jaunts around the filesystem.
# BEGIN {
#     use Cwd;
#     *CORE::GLOBAL::chdir = sub ($) {
#         my($file, $line) = (caller)[1,2];
#
#         printf "# cwd:      %s\n", cwd();
#         print "# chdir: @_ from $file at $line\n";
#         my($return) = CORE::chdir($_[0]);
#         printf "# newcwd:   %s\n", cwd();
#
#         return $return;
#     };
# }

##### Sanity checks #####
# Do find() and finddepth() work correctly with an empty list of
# directories?
{
    ok(eval { find(\&noop_wanted); 1 },
       "'find' successfully returned for an empty list of directories");

    ok(eval { finddepth(\&noop_wanted); 1 },
       "'finddepth' successfully returned for an empty list of directories");
}

# Do find() and finddepth() work correctly in the directory
# from which we start?  (Test presumes the presence of 'find.t' in same
# directory as this test file.)

my $count_found = 0;
find({wanted => sub { ++$count_found if $_ eq 'find.t'; } },
   File::Spec->curdir);
is($count_found, 1, "'find' found exactly 1 file named 'find.t'");

$count_found = 0;
finddepth({wanted => sub { ++$count_found if $_ eq 'find.t'; } },
    File::Spec->curdir);
is($count_found, 1, "'finddepth' found exactly 1 file named 'find.t'");

my $FastFileTests_OK = 0;

my $chdir_error = "";
chdir($test_temp_dir)
    or $chdir_error = "Failed to chdir to '$test_temp_dir': $!";
is($chdir_error,"","chdir to temp dir '$test_temp_dir' successful")
    or die $chdir_error;

sub cleanup {
    # the following chdirs into $test_root_dir/$test_temp_dir but
    # handles various possible edge case errors cleanly. If it returns
    # false then we bail out of the cleanup.
    _cleanup_start($test_root_dir, $test_temp_dir)
        or return;

    my $need_updir = 0;
    if (-d dir_path('for_find')) {
        $need_updir = 1 if chdir(dir_path('for_find'));
    }
    if (-d dir_path('fa')) {
    unlink file_path('fa', 'fa_ord'),
           file_path('fa', 'fsl'),
           file_path('fa', 'faa', 'faa_ord'),
           file_path('fa', 'fab', 'fab_ord'),
           file_path('fa', 'fab', 'faba', 'faba_ord'),
           file_path('fa', 'fac', 'faca'),
           file_path('fb', 'fb_ord'),
           file_path('fb', 'fba', 'fba_ord'),
           file_path('fb', 'fbc', 'fbca'),
           file_path('fa', 'fax', 'faz'),
           file_path('fa', 'fay');
    rmdir dir_path('fa', 'faa');
    rmdir dir_path('fa', 'fab', 'faba');
    rmdir dir_path('fa', 'fab');
    rmdir dir_path('fa', 'fac');
    rmdir dir_path('fa', 'fax');
    rmdir dir_path('fa');
    rmdir dir_path('fb', 'fba');
    rmdir dir_path('fb', 'fbc');
    rmdir dir_path('fb');
    }
    if (-d dir_path('fc')) {
        unlink (
            file_path('fc', 'fca', 'match_alpha'),
            file_path('fc', 'fca', 'match_beta'),
            file_path('fc', 'fcb', 'match_gamma'),
            file_path('fc', 'fcb', 'delta'),
            file_path('fc', 'fcc', 'match_epsilon'),
            file_path('fc', 'fcc', 'match_zeta'),
            file_path('fc', 'fcc', 'eta'),
        );
        rmdir dir_path('fc', 'fca');
        rmdir dir_path('fc', 'fcb');
        rmdir dir_path('fc', 'fcc');
        rmdir dir_path('fc');
    }
    if ($need_updir) {
        my $updir = $^O eq 'VMS' ? File::Spec::VMS->updir() : File::Spec->updir;
        chdir($updir);
    }
    if (-d dir_path('for_find')) {
        rmdir dir_path('for_find') or print "# Can't rmdir for_find: $!\n";
    }
    chdir($test_root_dir) or die "Failed to chdir to '$test_root_dir': $!";
}

END {
    cleanup();
}

sub wanted_File_Dir {
    print "# \$File::Find::dir => '$File::Find::dir'\t\$_ => '$_'\n";
    s#\.$## if ($^O eq 'VMS' && $_ ne '.'); #
    s/(.dir)?$//i if ($^O eq 'VMS' && -d _);
    ok( $Expect_File{$_}, "found $_ for \$_, as expected" );
    if ( $FastFileTests_OK ) {
        delete $Expect_File{$_}
          unless ( $Expect_Dir{$_} && ! -d _ );
    }
    else {
        delete $Expect_File{$_}
          unless ( $Expect_Dir{$_} && ! -d $_ );
    }
}

sub wanted_File_Dir_prune {
    &wanted_File_Dir;
    $File::Find::prune = 1 if  $_ eq 'faba';
}

sub wanted_Name {
    my $n = $File::Find::name;
    $n =~ s#\.$## if ($^O eq 'VMS' && $n ne '.'); #
    print "# \$File::Find::name => '$n'\n";
    my $i = rindex($n,'/');
    my $OK = exists($Expect_Name{$n});
    if ( $OK ) {
        $OK= exists($Expect_Name{substr($n,0,$i)})  if $i >= 0;
    }
    ok( $OK, "found $n for \$File::Find::name, as expected" );
    delete $Expect_Name{$n};
}

sub wanted_File {
    print "# \$_ => '$_'\n";
    s#\.$## if ($^O eq 'VMS' && $_ ne '.'); #
    my $i = rindex($_,'/');
    my $OK = exists($Expect_File{ $_});
    if ( $OK ) {
        $OK= exists($Expect_File{ substr($_,0,$i)})  if $i >= 0;
    }
    ok( $OK, "found $_ for \$_, as expected" );
    delete $Expect_File{ $_};
}

sub simple_wanted {
    print "# \$File::Find::dir => '$File::Find::dir'\n";
    print "# \$_ => '$_'\n";
}

sub noop_wanted {}

sub my_preprocess {
    @files = @_;
    print "# --preprocess--\n";
    print "#   \$File::Find::dir => '$File::Find::dir' \n";
    foreach my $file (@files) {
        $file =~ s/\.(dir)?$//i if $^O eq 'VMS';
        print "#   $file \n";
        delete $Expect_Dir{ $File::Find::dir }->{$file};
    }
    print "# --end preprocess--\n";
    is(scalar(keys %{$Expect_Dir{ $File::Find::dir }}), 0,
        "my_preprocess: got 0, as expected");
    if (scalar(keys %{$Expect_Dir{ $File::Find::dir }}) == 0) {
        delete $Expect_Dir{ $File::Find::dir }
    }
    return @files;
}

sub my_postprocess {
    print "# postprocess: \$File::Find::dir => '$File::Find::dir' \n";
    delete $Expect_Dir{ $File::Find::dir};
}

# Use topdir() to specify a directory path that you want to pass to
# find/finddepth. Historically topdir() differed on Mac OS classic.

*topdir = \&dir_path;

# Use file_path_name() to specify a file path that is expected for
# $File::Find::Name (%Expect_Name). Note: When the no_chdir => 1
# option is in effect, $_ is the same as $File::Find::Name. In that
# case, also use this function to specify a file path that is expected
# for $_.
#
# Historically file_path_name differed on Mac OS classic.

*file_path_name = \&file_path;

##### Create directories, files and symlinks used in testing #####
mkdir_ok( dir_path('for_find'), 0770 );
ok( chdir( dir_path('for_find')), "Able to chdir to 'for_find'")
    or die("Unable to chdir to 'for_find'");

my @testing_basenames = ( qw| fb_ord fba_ord fa_ord faa_ord fab_ord faba_ord | );

mkdir_ok( dir_path('fa'), 0770 );
mkdir_ok( dir_path('fb'), 0770  );
create_file_ok( file_path('fb', $testing_basenames[0]) );
mkdir_ok( dir_path('fb', 'fba'), 0770  );
create_file_ok( file_path('fb', 'fba', $testing_basenames[1]) );
if ($symlink_exists) {
    if (symlink('../fb','fa/fsl')) {
        pass("able to symlink from ../fb to fa/fsl");
    }
    else {
        if ($^O eq "MSWin32" && ($! == &Errno::ENOSYS || $! == &Errno::EPERM)) {
            $symlink_exists = 0;
        }
        else {
            fail("able to symlink from ../fb to fa/fsl");
        }
    }
}
create_file_ok( file_path('fa', $testing_basenames[2]) );

mkdir_ok( dir_path('fa', 'faa'), 0770  );
create_file_ok( file_path('fa', 'faa', $testing_basenames[3]) );
mkdir_ok( dir_path('fa', 'fab'), 0770  );
create_file_ok( file_path('fa', 'fab', $testing_basenames[4]) );
mkdir_ok( dir_path('fa', 'fab', 'faba'), 0770  );
create_file_ok( file_path('fa', 'fab', 'faba', $testing_basenames[5]) );

##### RT #122547 #####
# Do find() and finddepth() correctly warn on invalid options?
##### RT #133771 #####
# When running tests in parallel, avoid clash with tests in
# ext/File-Find/t/taint by moving into the temporary testing directory
# before testing for warnings on invalid options.

my %tb = map { $_ => 1 } @testing_basenames;

{
    my $bad_option = 'foobar';
    my $second_bad_option = 'really_foobar';

    $::count_tb = 0;
    local $SIG{__WARN__} = sub { $warn_msg = $_[0]; };
    {
        find(
            {
                wanted => sub { s#\.$## if ($^O eq 'VMS' && $_ ne '.');
                                ++$::count_tb if $tb{$_};
                              },
                $bad_option => undef,
            },
            File::Spec->curdir
        );
    };
    like($warn_msg, qr/Invalid option/s, "Got warning for invalid option");
    like($warn_msg, qr/$bad_option/s, "Got warning for $bad_option");
    is($::count_tb, scalar(@testing_basenames), "count_tb incremented");
    undef $warn_msg;

    $::count_tb = 0;
    {
        finddepth(
            {
                wanted => sub { s#\.$## if ($^O eq 'VMS' && $_ ne '.');
                                ++$::count_tb if $tb{$_};
                              },
                $bad_option => undef,
                $second_bad_option => undef,
            },
            File::Spec->curdir
        );
    };
    like($warn_msg, qr/Invalid option/s, "Got warning for invalid option");
    like($warn_msg, qr/$bad_option/s, "Got warning for $bad_option");
    like($warn_msg, qr/$second_bad_option/s, "Got warning for $second_bad_option");
    is($::count_tb, scalar(@testing_basenames), "count_tb incremented");
    undef $warn_msg;
}

##### Basic tests for find() #####
# Set up list of files we expect to find.
# Run find(), removing a file from the list once we have found it.
# The list should be empty once we are done.

%Expect_File = (File::Spec->curdir => 1, file_path('fsl') => 1,
                file_path('fa_ord') => 1, file_path('fab') => 1,
                file_path('fab_ord') => 1, file_path('faba') => 1,
                file_path('faa') => 1, file_path('faa_ord') => 1);

delete $Expect_File{ file_path('fsl') } unless $symlink_exists;
%Expect_Name = ();

%Expect_Dir = ( dir_path('fa') => 1, dir_path('faa') => 1,
                dir_path('fab') => 1, dir_path('faba') => 1,
                dir_path('fb') => 1, dir_path('fba') => 1);

delete @Expect_Dir{ dir_path('fb'), dir_path('fba') } unless $symlink_exists;
File::Find::find( {wanted => \&wanted_File_Dir_prune}, topdir('fa') );
is( scalar(keys %Expect_File), 0, "COMPLETE: Basic test of find()" );

##### Re-entrancy #####

print "# check re-entrancy\n";

%Expect_File = (File::Spec->curdir => 1, file_path('fsl') => 1,
                file_path('fa_ord') => 1, file_path('fab') => 1,
                file_path('fab_ord') => 1, file_path('faba') => 1,
                file_path('faa') => 1, file_path('faa_ord') => 1);

delete $Expect_File{ file_path('fsl') } unless $symlink_exists;
%Expect_Name = ();

%Expect_Dir = ( dir_path('fa') => 1, dir_path('faa') => 1,
                dir_path('fab') => 1, dir_path('faba') => 1,
                dir_path('fb') => 1, dir_path('fba') => 1);

delete @Expect_Dir{ dir_path('fb'), dir_path('fba') } unless $symlink_exists;

File::Find::find( {wanted => sub { wanted_File_Dir_prune();
                                    File::Find::find( {wanted => sub
                                    {} }, File::Spec->curdir ); } },
                                    topdir('fa') );

is( scalar(keys %Expect_File), 0, "COMPLETE: Test of find() for re-entrancy" );

##### 'no_chdir' option #####
# no_chdir is in effect, hence we use file_path_name to specify the expected paths for %Expect_File

%Expect_File = (file_path_name('fa') => 1,
        file_path_name('fa', 'fsl') => 1,
        file_path_name('fa', 'fa_ord') => 1,
        file_path_name('fa', 'fab') => 1,
        file_path_name('fa', 'fab', 'fab_ord') => 1,
        file_path_name('fa', 'fab', 'faba') => 1,
        file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
        file_path_name('fa', 'faa') => 1,
        file_path_name('fa', 'faa', 'faa_ord') => 1,);

delete $Expect_File{ file_path_name('fa', 'fsl') } unless $symlink_exists;
%Expect_Name = ();

%Expect_Dir = (dir_path('fa') => 1,
        dir_path('fa', 'faa') => 1,
        dir_path('fa', 'fab') => 1,
        dir_path('fa', 'fab', 'faba') => 1,
        dir_path('fb') => 1,
        dir_path('fb', 'fba') => 1);

delete @Expect_Dir{ dir_path('fb'), dir_path('fb', 'fba') }
    unless $symlink_exists;

File::Find::find( {wanted => \&wanted_File_Dir, no_chdir => 1},
          topdir('fa') );
is( scalar(keys %Expect_File), 0, "COMPLETE: Test of 'no_chdir' option" );

##### Test for $File::Find::name #####

%Expect_File = ();

%Expect_Name = (File::Spec->curdir => 1,
        file_path_name('.', 'fa') => 1,
        file_path_name('.', 'fa', 'fsl') => 1,
        file_path_name('.', 'fa', 'fa_ord') => 1,
        file_path_name('.', 'fa', 'fab') => 1,
        file_path_name('.', 'fa', 'fab', 'fab_ord') => 1,
        file_path_name('.', 'fa', 'fab', 'faba') => 1,
        file_path_name('.', 'fa', 'fab', 'faba', 'faba_ord') => 1,
        file_path_name('.', 'fa', 'faa') => 1,
        file_path_name('.', 'fa', 'faa', 'faa_ord') => 1,
        file_path_name('.', 'fb') => 1,
        file_path_name('.', 'fb', 'fba') => 1,
        file_path_name('.', 'fb', 'fba', 'fba_ord') => 1,
        file_path_name('.', 'fb', 'fb_ord') => 1);

delete $Expect_Name{ file_path('.', 'fa', 'fsl') } unless $symlink_exists;
%Expect_Dir = ();
File::Find::finddepth( {wanted => \&wanted_Name}, File::Spec->curdir );
is( scalar(keys %Expect_Name), 0, "COMPLETE: Test for \$File::Find::name" );


##### #####
# no_chdir is in effect, hence we use file_path_name to specify the
# expected paths for %Expect_File

%Expect_File = (File::Spec->curdir => 1,
        file_path_name('.', 'fa') => 1,
        file_path_name('.', 'fa', 'fsl') => 1,
        file_path_name('.', 'fa', 'fa_ord') => 1,
        file_path_name('.', 'fa', 'fab') => 1,
        file_path_name('.', 'fa', 'fab', 'fab_ord') => 1,
        file_path_name('.', 'fa', 'fab', 'faba') => 1,
        file_path_name('.', 'fa', 'fab', 'faba', 'faba_ord') => 1,
        file_path_name('.', 'fa', 'faa') => 1,
        file_path_name('.', 'fa', 'faa', 'faa_ord') => 1,
        file_path_name('.', 'fb') => 1,
        file_path_name('.', 'fb', 'fba') => 1,
        file_path_name('.', 'fb', 'fba', 'fba_ord') => 1,
        file_path_name('.', 'fb', 'fb_ord') => 1);

delete $Expect_File{ file_path_name('.', 'fa', 'fsl') } unless $symlink_exists;
%Expect_Name = ();
%Expect_Dir = ();

File::Find::finddepth( {wanted => \&wanted_File, no_chdir => 1},
             File::Spec->curdir );

is( scalar(keys %Expect_File), 0,
    "COMPLETE: Equivalency of \$_ and \$File::Find::Name with 'no_chdir'" );

##### #####

print "# check preprocess\n";
%Expect_File = ();
%Expect_Name = ();
%Expect_Dir = (
         File::Spec->curdir                 => {fa => 1, fb => 1},
         dir_path('.', 'fa')                => {faa => 1, fab => 1, fa_ord => 1},
         dir_path('.', 'fa', 'faa')         => {faa_ord => 1},
         dir_path('.', 'fa', 'fab')         => {faba => 1, fab_ord => 1},
         dir_path('.', 'fa', 'fab', 'faba') => {faba_ord => 1},
         dir_path('.', 'fb')                => {fba => 1, fb_ord => 1},
         dir_path('.', 'fb', 'fba')         => {fba_ord => 1}
         );

File::Find::find( {wanted => \&noop_wanted,
         preprocess => \&my_preprocess}, File::Spec->curdir );

is( scalar(keys %Expect_Dir), 0, "Got no files, as expected" );

##### #####

print "# check postprocess\n";
%Expect_File = ();
%Expect_Name = ();
%Expect_Dir = (
         File::Spec->curdir                 => 1,
         dir_path('.', 'fa')                => 1,
         dir_path('.', 'fa', 'faa')         => 1,
         dir_path('.', 'fa', 'fab')         => 1,
         dir_path('.', 'fa', 'fab', 'faba') => 1,
         dir_path('.', 'fb')                => 1,
         dir_path('.', 'fb', 'fba')         => 1
         );

File::Find::find( {wanted => \&noop_wanted,
         postprocess => \&my_postprocess}, File::Spec->curdir );

is( scalar(keys %Expect_Dir), 0, "Got no files, as expected" );

##### #####
{
    print "# checking argument localization\n";

    ### this checks the fix of perlbug [19977] ###
    my @foo = qw( a b c d e f );
    my %pre = map { $_ => } @foo;

    File::Find::find( sub {  } , 'fa' ) for @foo;
    delete $pre{$_} for @foo;

    is( scalar(keys %pre), 0, "Got no files, as expected" );
}

##### #####
# see thread starting
# http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2004-02/msg00351.html
{
    print "# checking that &_ and %_ are still accessible and that\n",
    "# tie magic on \$_ is not triggered\n";

    my $true_count;
    my $sub = 0;
    sub _ {
        ++$sub;
    }
    my $tie_called = 0;

    package Foo;
    sub STORE {
        ++$tie_called;
    }
    sub FETCH {return 'N'};
    sub TIESCALAR {bless []};
    package main;

    is( scalar(keys %_), 0, "Got no files, as expected" );
    my @foo = 'n';
    tie $foo[0], "Foo";

    File::Find::find( sub { $true_count++; $_{$_}++; &_; } , 'fa' ) for @foo;
    untie $_;

    is( $tie_called, 0, "Got no files tie_called, as expected" );
    is( scalar(keys %_), $true_count, "Got true count, as expected" );
    is( $sub, $true_count, "Got true count, as expected" );
    is( scalar( @foo), 1, "Got one file, as expected" );
    is( $foo[0], 'N', "Got 'N', as expected" );
}

##### #####
if ( $symlink_exists ) {
    print "# --- symbolic link tests --- \n";
    $FastFileTests_OK= 1;

    # 'follow', 'follow_fast' and 'follow_skip' options only apply when a
    # platform supports symlinks.

    ##### #####

    # Verify that File::Find::find will call wanted even if the topdir
    # is a symlink to a directory, and it should not follow the link
    # unless follow is set, which it is not in this case
    %Expect_File = ( file_path('fsl') => 1 );
    %Expect_Name = ();
    %Expect_Dir = ();
    File::Find::find( {wanted => \&wanted_File_Dir}, topdir('fa', 'fsl') );
    is( scalar(keys %Expect_File), 0,
        "COMPLETE: top dir can be symlink to dir; link not followed without 'follow' option" );

    ##### #####

    %Expect_File = (File::Spec->curdir => 1, file_path('fa_ord') => 1,
                    file_path('fsl') => 1, file_path('fb_ord') => 1,
                    file_path('fba') => 1, file_path('fba_ord') => 1,
                    file_path('fab') => 1, file_path('fab_ord') => 1,
                    file_path('faba') => 1, file_path('faa') => 1,
                    file_path('faa_ord') => 1);

    %Expect_Name = ();

    %Expect_Dir = (File::Spec->curdir => 1, dir_path('fa') => 1,
                   dir_path('faa') => 1, dir_path('fab') => 1,
                   dir_path('faba') => 1, dir_path('fb') => 1,
                   dir_path('fba') => 1);

    File::Find::find( {wanted => \&wanted_File_Dir_prune,
               follow_fast => 1}, topdir('fa') );

    is( scalar(keys %Expect_File), 0,
        "COMPLETE: test of 'follow_fast' option: \$_ case" );

    ##### #####

    # no_chdir is in effect, hence we use file_path_name to specify
    # the expected paths for %Expect_File

    %Expect_File = (file_path_name('fa') => 1,
            file_path_name('fa', 'fa_ord') => 1,
            file_path_name('fa', 'fsl') => 1,
            file_path_name('fa', 'fsl', 'fb_ord') => 1,
            file_path_name('fa', 'fsl', 'fba') => 1,
            file_path_name('fa', 'fsl', 'fba', 'fba_ord') => 1,
            file_path_name('fa', 'fab') => 1,
            file_path_name('fa', 'fab', 'fab_ord') => 1,
            file_path_name('fa', 'fab', 'faba') => 1,
            file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
            file_path_name('fa', 'faa') => 1,
            file_path_name('fa', 'faa', 'faa_ord') => 1);

    %Expect_Name = ();

    %Expect_Dir = (dir_path('fa') => 1,
            dir_path('fa', 'faa') => 1,
            dir_path('fa', 'fab') => 1,
            dir_path('fa', 'fab', 'faba') => 1,
            dir_path('fb') => 1,
            dir_path('fb', 'fba') => 1);

    File::Find::find( {wanted => \&wanted_File_Dir, follow_fast => 1,
               no_chdir => 1}, topdir('fa') );

    is( scalar(keys %Expect_File), 0,
        "COMPLETE: Test of 'follow_fast' and 'no_chdir' options together: \$_ case" );

    ##### #####

    %Expect_File = ();

    %Expect_Name = (file_path_name('fa') => 1,
            file_path_name('fa', 'fa_ord') => 1,
            file_path_name('fa', 'fsl') => 1,
            file_path_name('fa', 'fsl', 'fb_ord') => 1,
            file_path_name('fa', 'fsl', 'fba') => 1,
            file_path_name('fa', 'fsl', 'fba', 'fba_ord') => 1,
            file_path_name('fa', 'fab') => 1,
            file_path_name('fa', 'fab', 'fab_ord') => 1,
            file_path_name('fa', 'fab', 'faba') => 1,
            file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
            file_path_name('fa', 'faa') => 1,
            file_path_name('fa', 'faa', 'faa_ord') => 1);

    %Expect_Dir = ();

    File::Find::finddepth( {wanted => \&wanted_Name,
            follow_fast => 1}, topdir('fa') );

    is( scalar(keys %Expect_Name), 0,
        "COMPLETE: test of 'follow_fast' option: \$File::Find::name case" );

    ##### #####

    # no_chdir is in effect, hence we use file_path_name to specify
    # the expected paths for %Expect_File

    %Expect_File = (file_path_name('fa') => 1,
            file_path_name('fa', 'fa_ord') => 1,
            file_path_name('fa', 'fsl') => 1,
            file_path_name('fa', 'fsl', 'fb_ord') => 1,
            file_path_name('fa', 'fsl', 'fba') => 1,
            file_path_name('fa', 'fsl', 'fba', 'fba_ord') => 1,
            file_path_name('fa', 'fab') => 1,
            file_path_name('fa', 'fab', 'fab_ord') => 1,
            file_path_name('fa', 'fab', 'faba') => 1,
            file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
            file_path_name('fa', 'faa') => 1,
            file_path_name('fa', 'faa', 'faa_ord') => 1);

    %Expect_Name = ();
    %Expect_Dir = ();

    File::Find::finddepth( {wanted => \&wanted_File, follow_fast => 1,
            no_chdir => 1}, topdir('fa') );

    is( scalar(keys %Expect_File), 0,
        "COMPLETE: Test of 'follow_fast' and 'no_chdir' options together: \$File::Find::name case" );

    ##### #####

    print "# check dangling symbolic links\n";
    mkdir_ok( dir_path('dangling_dir'), 0770 );
    symlink_ok( dir_path('dangling_dir'), file_path('dangling_dir_sl'),
        "Check dangling directory" );
    rmdir dir_path('dangling_dir');
    create_file_ok(file_path('dangling_file'));
    symlink_ok('../dangling_file','fa/dangling_file_sl',
        "Check dangling file" );
    unlink file_path('dangling_file');

    {
        # these tests should also emit a warning
    use warnings;

        %Expect_File = (File::Spec->curdir => 1,
            file_path('dangling_file_sl') => 1,
            file_path('fa_ord') => 1,
            file_path('fsl') => 1,
            file_path('fb_ord') => 1,
            file_path('fba') => 1,
            file_path('fba_ord') => 1,
            file_path('fab') => 1,
            file_path('fab_ord') => 1,
            file_path('faba') => 1,
            file_path('faba_ord') => 1,
            file_path('faa') => 1,
            file_path('faa_ord') => 1);

        %Expect_Name = ();
        %Expect_Dir = ();
        undef $warn_msg;

        File::Find::find( {wanted => \&wanted_File, follow => 1,
               dangling_symlinks =>
                   sub { $warn_msg = "$_[0] is a dangling symbolic link" }
                           },
                           topdir('dangling_dir_sl'), topdir('fa') );

        is( scalar(keys %Expect_File), 0,
            "COMPLETE: test of 'follow' and 'dangling_symlinks' options" );
        like( $warn_msg, qr/dangling_file_sl is a dangling symbolic link/,
            "Got expected warning message re dangling symbolic link" );
        unlink file_path('fa', 'dangling_file_sl'),
            file_path('dangling_dir_sl');

    }

    ##### #####

    print "# check recursion\n";
    symlink_ok('../faa','fa/faa/faa_sl');
    undef $@;
    eval {File::Find::find( {wanted => \&simple_wanted, follow => 1,
                             no_chdir => 1}, topdir('fa') ); };
    like(
        $@,
        qr{for_find[:/]fa[:/]faa[:/]faa_sl is a recursive symbolic link}i,
        "Got expected error message for recursive symbolic link"
    );
    unlink file_path('fa', 'faa', 'faa_sl');


    print "# check follow_skip (file)\n";
    symlink_ok('./fa_ord','fa/fa_ord_sl');
    undef $@;

    eval {File::Find::finddepth( {wanted => \&simple_wanted,
                                  follow => 1,
                                  follow_skip => 0, no_chdir => 1},
                                  topdir('fa') );};

    like(
        $@,
        qr{for_find[:/]fa[:/]fa_ord encountered a second time}i,
        "'follow_skip==0': got error message when file encountered a second time"
    );

    ##### #####

    # no_chdir is in effect, hence we use file_path_name to specify
    # the expected paths for %Expect_File

    %Expect_File = (file_path_name('fa') => 1,
            file_path_name('fa', 'fa_ord') => 2,
            # We may encounter the symlink first
            file_path_name('fa', 'fa_ord_sl') => 2,
            file_path_name('fa', 'fsl') => 1,
            file_path_name('fa', 'fsl', 'fb_ord') => 1,
            file_path_name('fa', 'fsl', 'fba') => 1,
            file_path_name('fa', 'fsl', 'fba', 'fba_ord') => 1,
            file_path_name('fa', 'fab') => 1,
            file_path_name('fa', 'fab', 'fab_ord') => 1,
            file_path_name('fa', 'fab', 'faba') => 1,
            file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
            file_path_name('fa', 'faa') => 1,
            file_path_name('fa', 'faa', 'faa_ord') => 1);

    %Expect_Name = ();

    %Expect_Dir = (dir_path('fa') => 1,
            dir_path('fa', 'faa') => 1,
            dir_path('fa', 'fab') => 1,
            dir_path('fa', 'fab', 'faba') => 1,
            dir_path('fb') => 1,
            dir_path('fb','fba') => 1);

    File::Find::finddepth( {wanted => \&wanted_File_Dir, follow => 1,
                           follow_skip => 1, no_chdir => 1},
                           topdir('fa') );
    is( scalar(keys %Expect_File), 0,
        "COMPLETE: Test of 'follow', 'follow_skip==1' and 'no_chdir' options" );
    unlink file_path('fa', 'fa_ord_sl');

    ##### #####
    print "# check follow_skip (directory)\n";
    symlink_ok('./faa','fa/faa_sl');
    undef $@;

    eval {File::Find::find( {wanted => \&simple_wanted, follow => 1,
                            follow_skip => 0, no_chdir => 1},
                            topdir('fa') );};

    like(
        $@,
        qr{for_find[:/]fa[:/]faa[:/]? encountered a second time}i,
        "'follow_skip==0': got error message when directory encountered a second time"
    );


    undef $@;

    eval {File::Find::find( {wanted => \&simple_wanted, follow => 1,
                            follow_skip => 1, no_chdir => 1},
                            topdir('fa') );};

    like(
        $@,
        qr{for_find[:/]fa[:/]faa[:/]? encountered a second time}i,
        "'follow_skip==1': got error message when directory encountered a second time"
     );

    ##### #####

    # no_chdir is in effect, hence we use file_path_name to specify
    # the expected paths for %Expect_File

    %Expect_File = (file_path_name('fa') => 1,
            file_path_name('fa', 'fa_ord') => 1,
            file_path_name('fa', 'fsl') => 1,
            file_path_name('fa', 'fsl', 'fb_ord') => 1,
            file_path_name('fa', 'fsl', 'fba') => 1,
            file_path_name('fa', 'fsl', 'fba', 'fba_ord') => 1,
            file_path_name('fa', 'fab') => 1,
            file_path_name('fa', 'fab', 'fab_ord') => 1,
            file_path_name('fa', 'fab', 'faba') => 1,
            file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
            file_path_name('fa', 'faa') => 1,
            file_path_name('fa', 'faa', 'faa_ord') => 1,
            # We may actually encounter the symlink first.
            file_path_name('fa', 'faa_sl') => 1,
            file_path_name('fa', 'faa_sl', 'faa_ord') => 1);

    %Expect_Name = ();

    %Expect_Dir = (dir_path('fa') => 1,
            dir_path('fa', 'faa') => 1,
            dir_path('fa', 'fab') => 1,
            dir_path('fa', 'fab', 'faba') => 1,
            dir_path('fb') => 1,
            dir_path('fb', 'fba') => 1);

    File::Find::find( {wanted => \&wanted_File_Dir, follow => 1,
               follow_skip => 2, no_chdir => 1}, topdir('fa') );

    ##### #####

    # If we encountered the symlink first, then the entries corresponding to
    # the real name remain, if the real name first then the symlink
    my @names = sort keys %Expect_File;
    is( scalar(@names), 1,
        "'follow_skip==2'" );
    # Normalise both to the original name
    s/_sl// foreach @names;
    is(
        $names[0],
        file_path_name('fa', 'faa', 'faa_ord'),
        "Got file_path_name, as expected"
    );
    unlink file_path('fa', 'faa_sl');

}

##### Win32 checks  - [perl #41555] #####

if ($^O eq 'MSWin32') {
    require File::Spec::Win32;
    my ($volume) = File::Spec::Win32->splitpath($test_root_dir, 1);
    print STDERR "VOLUME = $volume\n";

    ##### #####

    # with chdir
    %Expect_File = (File::Spec->curdir => 1,
                    file_path('fsl') => 1,
                    file_path('fa_ord') => 1,
                    file_path('fab') => 1,
                    file_path('fab_ord') => 1,
                    file_path('faba') => 1,
                    file_path('faba_ord') => 1,
                    file_path('faa') => 1,
                    file_path('faa_ord') => 1);

    delete $Expect_File{ file_path('fsl') } unless $symlink_exists;
    %Expect_Name = ();

    %Expect_Dir = (dir_path('fa') => 1,
                   dir_path('faa') => 1,
                   dir_path('fab') => 1,
                   dir_path('faba') => 1,
                   dir_path('fb') => 1,
                   dir_path('fba') => 1);

    $FastFileTests_OK = 0;
    File::Find::find( {wanted => \&wanted_File_Dir}, topdir('fa'));
    is( scalar(keys %Expect_File), 0, "Got no files, as expected" );

    ##### #####

    # no_chdir
    %Expect_File = ($volume . file_path_name('fa') => 1,
                    $volume . file_path_name('fa', 'fsl') => 1,
                    $volume . file_path_name('fa', 'fa_ord') => 1,
                    $volume . file_path_name('fa', 'fab') => 1,
                    $volume . file_path_name('fa', 'fab', 'fab_ord') => 1,
                    $volume . file_path_name('fa', 'fab', 'faba') => 1,
                    $volume . file_path_name('fa', 'fab', 'faba', 'faba_ord') => 1,
                    $volume . file_path_name('fa', 'faa') => 1,
                    $volume . file_path_name('fa', 'faa', 'faa_ord') => 1);


    delete $Expect_File{ $volume . file_path_name('fa', 'fsl') } unless $symlink_exists;
    %Expect_Name = ();

    %Expect_Dir = ($volume . dir_path('fa') => 1,
                   $volume . dir_path('fa', 'faa') => 1,
                   $volume . dir_path('fa', 'fab') => 1,
                   $volume . dir_path('fa', 'fab', 'faba') => 1);

    File::Find::find( {wanted => \&wanted_File_Dir, no_chdir => 1}, $volume . topdir('fa'));
    is( scalar(keys %Expect_File), 0, "Got no files, as expected" );
}


##### Issue 68260 #####

if ($symlink_exists) {
    print "# BUG  68260\n";
    mkdir_ok(dir_path ('fa', 'fac'), 0770);
    mkdir_ok(dir_path ('fb', 'fbc'), 0770);
    create_file_ok(file_path ('fa', 'fac', 'faca'));
    symlink_ok('..////../fa/fac/faca', 'fb/fbc/fbca',
        "RT 68260: able to symlink");

    use warnings;
    my $dangling_symlink;
    local $SIG {__WARN__} = sub {
        local $" = " ";         # "
        $dangling_symlink ++ if "@_" =~ /dangling symbolic link/;
    };

    File::Find::find (
        {
            wanted            => sub {1;},
            follow            => 1,
            follow_skip       => 2,
            dangling_symlinks => 1,
        },
        File::Spec -> curdir
    );

    ok(!$dangling_symlink, "Found no dangling symlink");
}

if ($symlink_exists) {  # perl #120388
    print "# BUG  120388\n";
    mkdir_ok(dir_path ('fa', 'fax'), 0770);
    create_file_ok(file_path ('fa', 'fax', 'faz'));
    symlink_ok( file_path ('..', 'fa', 'fax', 'faz'), file_path ('fa', 'fay') );
    my @seen;
    File::Find::find( {wanted => sub {
        if (/^fa[yz]$/) {
            push @seen, $_;
            ok(-e $File::Find::fullname,
                "file identified by 'fullname' exists");
            my $subdir = file_path qw/for_find fa fax faz/;
            like(
                $File::Find::fullname,
                qr/\Q$subdir\E$/,
                "fullname matches expected path"
            );
        }
    }, follow => 1}, topdir('fa'));
    # make sure "fay"(symlink) found before "faz"(real file);
    # otherwise test invalid
    is(join(',', @seen), 'fay,faz',
        "symlink found before real file, as expected");
}

##### Issue 59750 #####

print "# RT 59750\n";
mkdir_ok( dir_path('fc'), 0770 );
mkdir_ok( dir_path('fc', 'fca'), 0770 );
mkdir_ok( dir_path('fc', 'fcb'), 0770 );
mkdir_ok( dir_path('fc', 'fcc'), 0770 );
create_file_ok( file_path('fc', 'fca', 'match_alpha') );
create_file_ok( file_path('fc', 'fca', 'match_beta') );
create_file_ok( file_path('fc', 'fcb', 'match_gamma') );
create_file_ok( file_path('fc', 'fcb', 'delta') );
create_file_ok( file_path('fc', 'fcc', 'match_epsilon') );
create_file_ok( file_path('fc', 'fcc', 'match_zeta') );
create_file_ok( file_path('fc', 'fcc', 'eta') );

my @files_from_mixed = ();
sub wantmatch {
    if ( $File::Find::name =~ m/match/ ) {
        push @files_from_mixed, $_;
        print "# \$_ => '$_'\n";
    }
}
find( \&wantmatch, (
    dir_path('fc', 'fca'),
    dir_path('fc', 'fcb'),
    dir_path('fc', 'fcc'),
) );
is( scalar(@files_from_mixed), 5,
    "Prepare test for RT #59750: got 5 'match' files as expected" );

@files_from_mixed = ();
find( \&wantmatch, (
    dir_path('fc', 'fca'),
    dir_path('fc', 'fcb'),
    file_path('fc', 'fcc', 'match_epsilon'),
    file_path('fc', 'fcc', 'eta'),
) );
is( scalar(@files_from_mixed), 4,
    "Can mix directories and (non-directory) files in list of directories searched by wanted()" );

##### More Win32 checks#####

if ($^O eq 'MSWin32') {
    # Check F:F:f correctly handles a root directory path.
    # Rather than processing the entire drive (!), simply test that the
    # first file passed to the wanted routine is correct and then bail out.
    $test_root_dir =~ /^(\w:)/ or die "expected a drive: $test_root_dir";
    my $drive = $1;

    # Determine the file in the root directory which would be
    # first if processed in sorted order. Create one if necessary.
    my $expected_first_file;
    opendir(my $ROOT_DIR, "/") or die "cannot opendir /: $!\n";
    foreach my $f (sort readdir $ROOT_DIR) {
        if (-f "/$f") {
            $expected_first_file = $f;
            last;
        }
    }
    closedir $ROOT_DIR;
  SKIP:
    {
        my $created_file;
        unless (defined $expected_first_file) {
            $expected_first_file = '__perl_File_Find_test.tmp';
            open(F, ">", "/$expected_first_file") && close(F)
                or skip "cannot create file in root directory: $!", 8;
            $created_file = 1;
        }

        # Run F:F:f with/without no_chdir for each possible style of root path.
        # NB. If HOME were "/", then an inadvertent chdir('') would fluke the
        # expected result, so ensure it is something else:
        local $ENV{HOME} = $test_root_dir;
        foreach my $no_chdir (0, 1) {
            foreach my $root_dir ("/", "\\", "$drive/", "$drive\\") {
                eval {
                    File::Find::find({
                        'no_chdir' => $no_chdir,
                            'preprocess' => sub { return sort @_ },
                            'wanted' => sub {
                                -f or return; # the first call is for $root_dir itself.
                                my $got = $File::Find::name;
                                (my $exp = "$root_dir$expected_first_file") =~ s|\\|/|g;
                                print "# no_chdir=$no_chdir $root_dir '$got'\n";
                                is($got, $exp,
                                   "Win32: Run 'find' with 'no_chdir' set to $no_chdir" );
                                die "done"; # do not process the entire drive!
                        },
                    }, $root_dir);
                };
                # If F:F:f did not die "done" then it did not Check() either.
                unless ($@ and $@ =~ /done/) {
                    print "# no_chdir=$no_chdir $root_dir ",
                        ($@ ? "error: $@" : "no files found"), "\n";
                    ok(0, "Win32: 0");
                }
            }
        }
        if ($created_file) {
            unlink("/$expected_first_file")
                or warn "can't unlink /$expected_first_file: $!\n";
        }
    }
}

{
    local $@;
    eval { File::Find::find( 'foobar' ); };
    like($@, qr/no &wanted subroutine given/,
        "find() correctly died for lack of &wanted via either coderef or hashref");
}

{
    local $@;
    eval { File::Find::find( { follow => 1 } ); };
    like($@, qr/no &wanted subroutine given/,
        "find() correctly died for lack of &wanted via hashref");
}

{
    local $@;
    eval { File::Find::find( { wanted => 1 } ); };
    like($@, qr/no &wanted subroutine given/,
        "find() correctly died: lack of coderef as value of 'wanted' element");
}

{
    local $@;
    my $wanted = sub { print "hello world\n"; };
    eval { File::Find::find( $wanted, ( undef ) ); };
    like($@, qr/invalid top directory/,
        "find() correctly died due to undefined top directory");
}
done_testing();
