#!./perl

# Test that various IO functions don't try to treat PVBMs as
# filehandles. Most of these will segfault perl if they fail.

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib));
}

BEGIN { $| = 1 }

plan(28);

sub PVBM () { 'foo' }
{ my $dummy = index 'foo', PVBM }

{
    my $which;
    {
        package Tie;

        sub TIEHANDLE { $which = 'TIEHANDLE' }
        sub TIESCALAR { $which = 'TIESCALAR' }
    }
    my $pvbm = PVBM;
    
    tie $pvbm, 'Tie';
    is ($which, 'TIESCALAR', 'PVBM gets TIESCALAR');
}

{
    my $pvbm = PVBM;
    ok (scalar eval { untie $pvbm; 1 }, 'untie(PVBM) doesn\'t segfault');
    ok (scalar eval { tied $pvbm; 1  }, 'tied(PVBM) doesn\'t segfault');
}

{
    my $pvbm = PVBM;

    ok (scalar eval { pipe $pvbm, PIPE; }, 'pipe(PVBM, ) succeeds');
    close foo;
    close PIPE;
    ok (scalar eval { pipe PIPE, $pvbm;  }, 'pipe(, PVBM) succeeds');
    close foo;
    close PIPE;
    ok (!eval { pipe \$pvbm, PIPE;  }, 'pipe(PVBM ref, ) fails');
    ok (!eval { pipe PIPE, \$pvbm;  }, 'pipe(, PVBM ref) fails');

    ok (!eval { truncate $pvbm, 0 }, 'truncate(PVBM) fails');
    ok (!eval { truncate \$pvbm, 0}, 'truncate(PVBM ref) fails');

    ok (!eval { stat $pvbm }, 'stat(PVBM) fails');
    ok (!eval { stat \$pvbm }, 'stat(PVBM ref) fails');

    ok (!eval { lstat $pvbm }, 'lstat(PVBM) fails');
    ok (!eval { lstat \$pvbm }, 'lstat(PVBM ref) fails');

    ok (!eval { chdir $pvbm }, 'chdir(PVBM) fails');
    ok (!eval { chdir \$pvbm }, 'chdir(pvbm ref) fails');

    ok (!eval { close $pvbm }, 'close(PVBM) fails');
    ok (!eval { close $pvbm }, 'close(PVBM ref) fails');

    ok (!eval { chmod 0600, $pvbm }, 'chmod(PVBM) fails');
    ok (!eval { chmod 0600, \$pvbm }, 'chmod(PVBM ref) fails');

    SKIP: {
        skip('chown() not implemented on Win32', 2) if $^O eq 'MSWin32';
        ok (!eval { chown 0, 0, $pvbm }, 'chown(PVBM) fails');
        ok (!eval { chown 0, 0, \$pvbm }, 'chown(PVBM ref) fails');
    }

    ok (!eval { utime 0, 0, $pvbm }, 'utime(PVBM) fails');
    ok (!eval { utime 0, 0, \$pvbm }, 'utime(PVBM ref) fails');

    ok (!eval { <$pvbm> }, '<PVBM> fails');
    ok (!eval { readline $pvbm }, 'readline(PVBM) fails');
    ok (!eval { readline \$pvbm }, 'readline(PVBM ref) fails');

    ok (!eval { open $pvbm, '<', 'none.such' }, 'open(PVBM) fails');
    ok (!eval { open \$pvbm, '<', 'none.such', }, 'open(PVBM ref) fails');
}
