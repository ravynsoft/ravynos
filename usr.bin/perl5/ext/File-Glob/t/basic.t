#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bFile\/Glob\b/i) {
        print "1..0\n";
        exit 0;
    }
}
use strict;
use Test::More tests => 49;
BEGIN {use_ok('File::Glob', ':glob')};
use Cwd ();

my $vms_unix_rpt = 0;
my $vms_efs = 0;
my $vms_mode = 0;
if ($^O eq 'VMS') {
    if (eval 'require VMS::Feature') {
        $vms_unix_rpt = VMS::Feature::current("filename_unix_report");
        $vms_efs = VMS::Feature::current("efs_charset");
    } else {
        my $unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $efs_charset = $ENV{'DECC$EFS_CHARSET'} || '';
        $vms_unix_rpt = $unix_rpt =~ /^[ET1]/i;
        $vms_efs = $efs_charset =~ /^[ET1]/i;
    }
    $vms_mode = 1 unless ($vms_unix_rpt);
}


# look for the contents of the current directory
# try it in a directory that doesn't get modified during testing,
# so parallel testing won't give us race conditions. t/base/ seems
# fairly static

chdir 'base' or die "chdir base: $!";
$ENV{PATH} = "/bin";
delete @ENV{qw(BASH_ENV CDPATH ENV IFS)};
my @correct = ();
if (opendir(D, ".")) {
   @correct = grep { !/^\./ } sort readdir(D);
   closedir D;
}

is(
    File::Glob->can('glob'),
    undef,
    'Did not find glob() function in File::Glob',
);

chdir '..' or die "chdir .. $!";

# look up the user's home directory
# should return a list with one item, and not set ERROR
my @a;

SKIP: {
    my ($name, $home);
    skip $^O, 1 if $^O eq 'MSWin32' || $^O eq 'VMS'
	|| $^O eq 'os2';
    skip "Can't find user for $>: $@", 1 unless eval {
	($name, $home) = (getpwuid($>))[0,7];
	1;
    };
    skip "$> has no home directory", 1
	unless defined $home && defined $name && -d $home;

    @a = bsd_glob("~$name", GLOB_TILDE);

    if (GLOB_ERROR) {
        fail(GLOB_ERROR);
    } else {
        is_deeply (\@a, [$home],
            "GLOB_TILDE expands patterns that start with '~' to user name home directories"
        );
    }
}
# check plain tilde expansion
{
    my $tilde_check = sub {
        my @a = bsd_glob('~');

        if (GLOB_ERROR) {
            fail(GLOB_ERROR);
        } else {
            is_deeply (\@a, [$_[0]], join ' - ', 'tilde expansion', @_ > 1 ? $_[1] : ());
        }
    };
    my $passwd_home = eval { (getpwuid($>))[7] };

    TODO: {
        local $TODO = 'directory brackets look like pattern brackets to glob' if $^O eq 'VMS';
        local $ENV{HOME};
        delete $ENV{HOME};
        local $ENV{USERPROFILE};
        delete $ENV{USERPROFILE};
        $tilde_check->(defined $passwd_home ? $passwd_home : q{~}, 'no environment');
    }

    SKIP: {
        skip 'MSWin32 only', 1 if $^O ne 'MSWin32';
        local $ENV{HOME};
        delete $ENV{HOME};
        local $ENV{USERPROFILE};
        $ENV{USERPROFILE} = 'sweet win32 home';
        $tilde_check->(defined $passwd_home ? $passwd_home : $ENV{USERPROFILE}, 'USERPROFILE');
    }

    TODO: {
        local $TODO = 'directory brackets look like pattern brackets to glob' if $^O eq 'VMS';
        my $home = exists $ENV{HOME} ? $ENV{HOME}
        : eval { getpwuid($>); 1 } ? (getpwuid($>))[7]
        : $^O eq 'MSWin32' && exists $ENV{USERPROFILE} ? $ENV{USERPROFILE}
        : q{~};
        $tilde_check->($home);
    }
}

# check backslashing
# should return a list with one item, and not set ERROR
@a = bsd_glob('TEST', GLOB_QUOTE);
if (GLOB_ERROR) {
    fail(GLOB_ERROR);
} else {
    is_deeply(\@a, ['TEST'], "GLOB_QUOTE works as expected");
}

# check nonexistent checks
# should return an empty list
# XXX since errfunc is NULL on win32, this test is not valid there
@a = bsd_glob("asdfasdf", 0);
SKIP: {
    skip $^O, 1 if $^O eq 'MSWin32';
    is_deeply(\@a, [], "bsd_glob() works as expected for unmatched pattern and 0 flag");
}

# check bad protections
# should return an empty list, and set ERROR
SKIP: {
    skip $^O, 2 if $^O eq 'MSWin32'
        or $^O eq 'os2' or $^O eq 'VMS' or $^O eq 'cygwin';
    skip "AFS", 2 if Cwd::cwd() =~ m#^$Config{'afsroot'}#s;
    skip "running as root", 2 if not $>;

    my $dir = "pteerslo";
    mkdir $dir, 0;
    @a = bsd_glob("$dir/*", GLOB_ERR);
    rmdir $dir;
    local $TODO = 'hit VOS bug posix-956' if $^O eq 'vos';

    isnt(GLOB_ERROR, 0, "GLOB_ERROR is not 0");
    is_deeply(\@a, [], "Got empty list as expected");
}

# check for csh style globbing
@a = bsd_glob('{a,b}', GLOB_BRACE | GLOB_NOMAGIC);
is_deeply(\@a, ['a', 'b'], "Check for csh-style globbing");

@a = bsd_glob(
    '{TES*,doesntexist*,a,b}',
    GLOB_BRACE | GLOB_NOMAGIC | ($^O eq 'VMS' ? GLOB_NOCASE : 0)
);

# Working on t/TEST often causes this test to fail because it sees Emacs temp
# and RCS files.  Filter them out, and .pm files too, and patch temp files.
@a = grep !/(,v$|~$|\.(pm|ori?g|rej)$)/, @a;
@a = (grep !/test.pl/, @a) if $^O eq 'VMS';

map { $_  =~ s/test\.?/TEST/i } @a if $^O eq 'VMS';
print "# @a\n";

is_deeply(\@a, ['TEST', 'a', 'b'], "Got list of 3 elements, including 'TEST'");

# "~" should expand to $ENV{HOME}
{
    local $ENV{HOME} = "sweet home";
    @a = bsd_glob('~', GLOB_TILDE | GLOB_NOMAGIC);
    is_deeply(\@a, [$ENV{HOME}], "~ expands to envvar \$HOME");
}

# GLOB_ALPHASORT (default) should sort alphabetically regardless of case
mkdir "pteerslo", 0777 or die "mkdir 'pteerslo', 0777:  $!";
chdir "pteerslo" or die "chdir 'pteerslo' $!";

my @f_names = qw(Ax.pl Bx.pl Cx.pl aY.pl bY.pl cY.pl);
my @f_alpha = qw(Ax.pl aY.pl Bx.pl bY.pl Cx.pl cY.pl);
if ('a' lt 'A') { # EBCDIC char sets sort lower case before UPPER
    @f_names = sort(@f_names);
}
if ($^O eq 'VMS') { # VMS is happily caseignorant
    @f_alpha = qw(ax.pl ay.pl bx.pl by.pl cx.pl cy.pl);
    @f_names = @f_alpha;
}

for (@f_names) {
    open T, '>', $_ or die "Couldn't write to '$_': $!";
    close T or die "Couldn't close '$_': $!";
}

my $pat = "*.pl";

my @g_names = bsd_glob($pat, 0);
print "# f_names = @f_names\n";
print "# g_names = @g_names\n";
is_deeply(\@g_names, \@f_names, "Got expected case-sensitive list of filenames");

my @g_alpha = bsd_glob($pat);
print "# f_alpha = @f_alpha\n";
print "# g_alpha = @g_alpha\n";
is_deeply(\@g_alpha, \@f_alpha, "Got expected case-insensitive list of filenames");

unlink @f_names;
chdir "..";
rmdir "pteerslo";

# this can panic if PL_glob_index gets passed as flags to bsd_glob
<*>; <*>;
pass("Don't panic");

{
    use File::Temp qw(tempdir);
    use File::Spec qw();

    my($dir) = tempdir(CLEANUP => 1)
        or die "Could not create temporary directory";
    for my $file (qw(a_dej a_ghj a_qej)) {
        open my $fh, ">", File::Spec->catfile($dir, $file)
            or die "Could not create file $dir/$file: $!";
        close $fh;
    }
    my $cwd = Cwd::cwd();
    chdir $dir
        or die "Could not chdir to $dir: $!";
    my(@glob_files) = glob("a*{d[e]}j");
    chdir $cwd
        or die "Could not chdir back to $cwd: $!";
    local $TODO = "home-made glob doesn't do regexes" if $^O eq 'VMS';
    is_deeply(\@glob_files, ['a_dej'],
        "Got expected list: metacharacters and character class in pattern");
}

# This used to segfault.
my $i = bsd_glob('*', GLOB_ALTDIRFUNC);
is(&File::Glob::GLOB_ERROR, 0, "Successfuly ignored unsupported flag");

package frimpy; # get away from the glob override, so we can test csh_glob,
use Test::More;  # which is perl's default

# In case of PERL_EXTERNAL_GLOB:
use subs 'glob';
BEGIN { *glob = \&File::Glob::csh_glob }

is +(glob "a'b'")[0], (<a'b' c>)[0], "a'b' with and without spaces";
is <a"b">, 'ab', 'a"b" without spaces';
is_deeply [<a"b" c>], [qw<ab c>], 'a"b" without spaces';
is_deeply [<\\* .\\*>], [<\\*>,<.\\*>], 'backslashes with(out) spaces';
like <\\ >, qr/^\\? \z/, 'final escaped space';
is <a"b>, 'a"b', 'unmatched quote';
is < a"b >, 'a"b', 'unmatched quote with surrounding spaces';
is glob('a\"b'), 'a"b', '\ before quote *only* escapes quote';
is glob(q"a\'b"), "a'b", '\ before single quote *only* escapes quote';
is glob('"a\"b c\"d"'), 'a"b c"d', 'before \" within "..."';
is glob(q"'a\'b c\'d'"), "a'b c'd", q"before \' within '...'";


package bsdglob;  # for testing the :bsd_glob export tag

use File::Glob ':bsd_glob';
use Test::More;
for (qw[
        GLOB_ABEND
	GLOB_ALPHASORT
        GLOB_ALTDIRFUNC
        GLOB_BRACE
        GLOB_CSH
        GLOB_ERR
        GLOB_ERROR
        GLOB_LIMIT
        GLOB_MARK
        GLOB_NOCASE
        GLOB_NOCHECK
        GLOB_NOMAGIC
        GLOB_NOSORT
        GLOB_NOSPACE
        GLOB_QUOTE
        GLOB_TILDE
        bsd_glob
    ]) {
    ok (exists &$_, qq':bsd_glob exports $_');
}
is <a b>, 'a b', '<a b> under :bsd_glob';
is <"a" "b">, '"a" "b"', '<"a" "b"> under :bsd_glob';
is_deeply [<a b>], [q<a b>], '<> in list context under :bsd_glob';
