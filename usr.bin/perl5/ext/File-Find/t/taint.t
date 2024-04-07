#!./perl -T
use strict;
use lib qw( ./t/lib );

BEGIN {
    require File::Spec;
    if ($ENV{PERL_CORE}) {
        # May be doing dynamic loading while @INC is all relative
        @INC = map { $_ = File::Spec->rel2abs($_); /(.*)/; $1 } @INC;
    }
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
use Test::More;
use File::Find;
use File::Spec;
use Cwd;
use Testing qw(
    create_file_ok
    mkdir_ok
    symlink_ok
    dir_path
    file_path
    _cleanup_start
);
use Errno ();
use Config;
use File::Temp qw(tempdir);

BEGIN {
    plan(
        ${^TAINT}
        ? (tests => 48)
        : (skip_all => "A perl without taint support")
    );
}

my %Expect_File = (); # what we expect for $_
my %Expect_Name = (); # what we expect for $File::Find::name/fullname
my %Expect_Dir  = (); # what we expect for $File::Find::dir
my ($cwd, $cwd_untainted);

BEGIN {
    if ($^O ne 'VMS') {
        for (keys %ENV) { # untaint ENV
            ($ENV{$_}) = $ENV{$_} =~ /(.*)/;
        }
    }

    # Remove insecure directories from PATH
    my @path;
    my $sep = $Config{path_sep};
    foreach my $dir (split(/\Q$sep/,$ENV{'PATH'}))
    {
        ##
        ## Match the directory taint tests in mg.c::Perl_magic_setenv()
        ##
        push(@path,$dir) unless (length($dir) >= 256
                                 or
                                 substr($dir,0,1) ne "/"
                                 or
                                 (stat $dir)[2] & 002);
    }
    $ENV{'PATH'} = join($sep,@path);
}

my $symlink_exists = eval { symlink("",""); 1 };

my $test_root_dir; # where we are when this test starts
my $test_root_dir_tainted = cwd();
if ($test_root_dir_tainted =~ /^(.*)$/) {
    $test_root_dir = $1;
} else {
    die "Failed to untaint root dir of test";
}
ok($test_root_dir,"test_root_dir is set up as expected");
my $test_temp_dir = tempdir("FF_taint_t_XXXXXX",CLEANUP=>1);
ok($test_temp_dir,"test_temp_dir is set up as expected");

my $found;
find({wanted => sub { ++$found if $_ eq 'taint.t' },
                untaint => 1, untaint_pattern => qr|^(.+)$|}, File::Spec->curdir);

is($found, 1, 'taint.t found once');
$found = 0;

finddepth({wanted => sub { ++$found if $_ eq 'taint.t'; },
           untaint => 1, untaint_pattern => qr|^(.+)$|}, File::Spec->curdir);

is($found, 1, 'taint.t found once again');

my $case = 2;
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
    if (-d dir_path('for_find_taint')) {
        $need_updir = 1 if chdir(dir_path('for_find_taint'));
    }
    if (-d dir_path('fa_taint')) {
        unlink file_path('fa_taint', 'fa_ord'),
               file_path('fa_taint', 'fsl'),
               file_path('fa_taint', 'faa', 'faa_ord'),
               file_path('fa_taint', 'fab', 'fab_ord'),
               file_path('fa_taint', 'fab', 'faba', 'faba_ord'),
               file_path('fb_taint', 'fb_ord'),
               file_path('fb_taint', 'fba', 'fba_ord');
        rmdir dir_path('fa_taint', 'faa');
        rmdir dir_path('fa_taint', 'fab', 'faba');
        rmdir dir_path('fa_taint', 'fab');
        rmdir dir_path('fa_taint');
        rmdir dir_path('fb_taint', 'fba');
        rmdir dir_path('fb_taint');
    }
    if ($need_updir) {
        my $updir = $^O eq 'VMS' ? File::Spec::VMS->updir() : File::Spec->updir;
        chdir($updir);
    }
    if (-d dir_path('for_find_taint')) {
        rmdir dir_path('for_find_taint') or print "# Can't rmdir for_find_taint: $!\n";
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
    $File::Find::prune=1 if  $_ eq 'faba';
}

sub simple_wanted {
    print "# \$File::Find::dir => '$File::Find::dir'\n";
    print "# \$_ => '$_'\n";
}

# Use topdir() to specify a directory path that you want to pass to
# find/finddepth. Historically topdir() differed on Mac OS classic.

*topdir = \&dir_path;

# Use file_path_name() to specify a file path that's expected for
# $File::Find::Name (%Expect_Name). Note: When the no_chdir => 1
# option is in effect, $_ is the same as $File::Find::Name. In that
# case, also use this function to specify a file path that's expected
# for $_.
#
# Historically file_path_name differed on Mac OS classic.

*file_path_name = \&file_path;

##### Create directories, files and symlinks used in testing #####
mkdir_ok( dir_path('for_find_taint'), 0770 );
ok( chdir( dir_path('for_find_taint')), 'successful chdir() to for_find_taint' );

$cwd = cwd(); # save cwd
( $cwd_untainted ) = $cwd =~ m|^(.+)$|; # untaint it

mkdir_ok( dir_path('fa_taint'), 0770 );
mkdir_ok( dir_path('fb_taint'), 0770  );
create_file_ok( file_path('fb_taint', 'fb_ord') );
mkdir_ok( dir_path('fb_taint', 'fba'), 0770  );
create_file_ok( file_path('fb_taint', 'fba', 'fba_ord') );
SKIP: {
    skip "Creating symlink", 1, unless $symlink_exists;
    if (symlink('../fb_taint','fa_taint/fsl')) {
        pass('Created symbolic link' );
    }
    else {
        my $error = 0 + $!;
        if ($^O eq "MSWin32" &&
            ($error == &Errno::ENOSYS || $error == &Errno::EPERM)) {
            $symlink_exists = 0;
            skip "symbolic links not available", 1;
        }
        else {
            fail('Created symbolic link');
        }
    }
}
create_file_ok( file_path('fa_taint', 'fa_ord') );

mkdir_ok( dir_path('fa_taint', 'faa'), 0770  );
create_file_ok( file_path('fa_taint', 'faa', 'faa_ord') );
mkdir_ok( dir_path('fa_taint', 'fab'), 0770  );
create_file_ok( file_path('fa_taint', 'fab', 'fab_ord') );
mkdir_ok( dir_path('fa_taint', 'fab', 'faba'), 0770  );
create_file_ok( file_path('fa_taint', 'fab', 'faba', 'faba_ord') );

print "# check untainting (no follow)\n";

# untainting here should work correctly

%Expect_File = (File::Spec->curdir => 1, file_path('fsl') =>
                1,file_path('fa_ord') => 1, file_path('fab') => 1,
                file_path('fab_ord') => 1, file_path('faba') => 1,
                file_path('faa') => 1, file_path('faa_ord') => 1);
delete $Expect_File{ file_path('fsl') } unless $symlink_exists;
%Expect_Name = ();

%Expect_Dir = ( dir_path('fa_taint') => 1, dir_path('faa') => 1,
                dir_path('fab') => 1, dir_path('faba') => 1,
                dir_path('fb_taint') => 1, dir_path('fba') => 1);

delete @Expect_Dir{ dir_path('fb_taint'), dir_path('fba') } unless $symlink_exists;

File::Find::find( {wanted => \&wanted_File_Dir_prune, untaint => 1,
                   untaint_pattern => qr|^(.+)$|}, topdir('fa_taint') );

is(scalar keys %Expect_File, 0, 'Found all expected files')
    or diag "Not found " . join(" ", sort keys %Expect_File);

# don't untaint at all, should die
%Expect_File = ();
%Expect_Name = ();
%Expect_Dir  = ();
undef $@;
eval {File::Find::find( {wanted => \&simple_wanted}, topdir('fa_taint') );};
like( $@, qr|Insecure dependency|, 'Tainted directory causes death (good)' );
chdir($cwd_untainted);


# untaint pattern doesn't match, should die
undef $@;

eval {File::Find::find( {wanted => \&simple_wanted, untaint => 1,
                         untaint_pattern => qr|^(NO_MATCH)$|},
                         topdir('fa_taint') );};

like( $@, qr|is still tainted|, 'Bad untaint pattern causes death (good)' );
chdir($cwd_untainted);


# untaint pattern doesn't match, should die when we chdir to cwd
print "# check untaint_skip (No follow)\n";
undef $@;

eval {File::Find::find( {wanted => \&simple_wanted, untaint => 1,
                         untaint_skip => 1, untaint_pattern =>
                         qr|^(NO_MATCH)$|}, topdir('fa_taint') );};

print "# $@" if $@;
#$^D = 8;
like( $@, qr|insecure cwd|, 'Bad untaint pattern causes death in cwd (good)' );

chdir($cwd_untainted);


SKIP: {
    skip "Symbolic link tests", 17, unless $symlink_exists;
    print "# --- symbolic link tests --- \n";
    $FastFileTests_OK= 1;

    print "# check untainting (follow)\n";

    # untainting here should work correctly
    # no_chdir is in effect, hence we use file_path_name to specify the expected paths for %Expect_File

    %Expect_File = (file_path_name('fa_taint') => 1,
                    file_path_name('fa_taint','fa_ord') => 1,
                    file_path_name('fa_taint', 'fsl') => 1,
                    file_path_name('fa_taint', 'fsl', 'fb_ord') => 1,
                    file_path_name('fa_taint', 'fsl', 'fba') => 1,
                    file_path_name('fa_taint', 'fsl', 'fba', 'fba_ord') => 1,
                    file_path_name('fa_taint', 'fab') => 1,
                    file_path_name('fa_taint', 'fab', 'fab_ord') => 1,
                    file_path_name('fa_taint', 'fab', 'faba') => 1,
                    file_path_name('fa_taint', 'fab', 'faba', 'faba_ord') => 1,
                    file_path_name('fa_taint', 'faa') => 1,
                    file_path_name('fa_taint', 'faa', 'faa_ord') => 1);

    %Expect_Name = ();

    %Expect_Dir = (dir_path('fa_taint') => 1,
                   dir_path('fa_taint', 'faa') => 1,
                   dir_path('fa_taint', 'fab') => 1,
                   dir_path('fa_taint', 'fab', 'faba') => 1,
                   dir_path('fb_taint') => 1,
                   dir_path('fb_taint', 'fba') => 1);

    File::Find::find( {wanted => \&wanted_File_Dir, follow_fast => 1,
                       no_chdir => 1, untaint => 1, untaint_pattern =>
                       qr|^(.+)$| }, topdir('fa_taint') );

    is( scalar(keys %Expect_File), 0, 'Found all files in symlink test' );


    # don't untaint at all, should die
    undef $@;

    eval {File::Find::find( {wanted => \&simple_wanted, follow => 1},
                            topdir('fa_taint') );};

    like( $@, qr|Insecure dependency|, 'Not untainting causes death (good)' );
    chdir($cwd_untainted);

    # untaint pattern doesn't match, should die
    undef $@;

    eval {File::Find::find( {wanted => \&simple_wanted, follow => 1,
                             untaint => 1, untaint_pattern =>
                             qr|^(NO_MATCH)$|}, topdir('fa_taint') );};

    like( $@, qr|is still tainted|, 'Bat untaint pattern causes death (good)' );
    chdir($cwd_untainted);

    # untaint pattern doesn't match, should die when we chdir to cwd
    print "# check untaint_skip (Follow)\n";
    undef $@;

    eval {File::Find::find( {wanted => \&simple_wanted, untaint => 1,
                             untaint_skip => 1, untaint_pattern =>
                             qr|^(NO_MATCH)$|}, topdir('fa_taint') );};
    like( $@, qr|insecure cwd|, 'Cwd not untainted with bad pattern (good)' );

    chdir($cwd_untainted);
}
