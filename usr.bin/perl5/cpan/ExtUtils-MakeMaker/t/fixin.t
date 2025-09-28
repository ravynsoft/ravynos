#!/usr/bin/perl -w

use strict;
use warnings;

# Try to test fixin.  I say "try" because what fixin will actually do
# is highly variable from system to system.

BEGIN {
    unshift @INC, 't/lib/';
}

use File::Spec;

use Test::More tests => 30;

use Config;
use TieOut;
use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;

use ExtUtils::MakeMaker;

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir 'Big-Dummy', "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

# [rt.cpan.org 26234]
{
    local $/ = "foo";
    local $\ = "bar";
    MY->fixin("bin/program");
    is $/, "foo", '$/ not clobbered';
    is $\, "bar", '$\ not clobbered';
}


sub test_fixin {
    my($code, $test) = @_;

    my $file = "fixin_test";
    ok(open(my $fh, ">", $file), "write $file") or diag "Can't write $file: $!";
    print $fh $code;
    close $fh;

    MY->fixin($file);

    ok(open($fh, "<", $file), "read $file") or diag "Can't read $file: $!";
    my @lines = <$fh>;
    close $fh;

    $test->(@lines);

    1 while unlink $file;
    ok !-e $file, "cleaned up $file";
}


# A simple test of fixin
# On VMS, the shebang line comes after the startperl business.
my $shb_line_num = $^O eq 'VMS' ? 2 : 0;
test_fixin(<<END,
#!/foo/bar/perl -w

blah blah blah
END
    sub {
        my @lines = @_;
        unlike $lines[$shb_line_num], qr[/foo/bar/perl], "#! replaced";
        like   $lines[$shb_line_num], qr[ -w\b], "switch retained";

        # In between might be that "not running under some shell" madness.

        is $lines[-1], "blah blah blah\n", "Program text retained";
    }
);


# [rt.cpan.org 29442]
test_fixin(<<END,
#!/foo/bar/perl5.8.8 -w

blah blah blah
END

    sub {
        my @lines = @_;
        unlike $lines[$shb_line_num], qr[/foo/bar/perl5.8.8], "#! replaced";
        like   $lines[$shb_line_num], qr[ -w\b], "switch retained";

        # In between might be that "not running under some shell" madness.

        is $lines[-1], "blah blah blah\n", "Program text retained";
    }
);


# fixin shouldn't pick this up.
SKIP: {
    skip "Not relevant on VMS", 4 if $^O eq 'VMS';
    test_fixin(<<END,
#!/foo/bar/perly -w

blah blah blah
END

        sub {
            is join("", @_), <<END;
#!/foo/bar/perly -w

blah blah blah
END
        }
    );
}

SKIP: {
    eval { chmod(0755, "usrbin/interp") }
        or skip "no chmod", 8;
    skip "Not relevant on VMS or MSWin32", 8 if $^O eq 'VMS' || $^O eq 'MSWin32' || $^O eq 'cygwin';

    my $dir = getcwd();
    local $ENV{PATH} = join $Config{path_sep}, map "$dir/$_", qw(usrbin bin);

    test_fixin(<<END,
#!$dir/bin/interp

blah blah blah
END
         sub {
             is $_[0], "#!$dir/usrbin/interp\n", 'interpreter updated to one found in PATH';
         }
     );

    eval { symlink("../usrbin/interp", "bin/interp") }
        or skip "no symlinks", 4;

    test_fixin(<<END,
#!$dir/bin/interp

blah blah blah
END
         sub {
             is $_[0], "#!$dir/bin/interp\n", 'symlinked interpreter later in PATH not mangled';
         }
     );
}
