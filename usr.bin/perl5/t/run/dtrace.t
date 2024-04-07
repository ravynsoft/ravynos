#!./perl

my $Perl;
my $dtrace;

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';

    skip_all_without_config("usedtrace");

    $dtrace = $Config::Config{dtrace};

    $Perl = which_perl();

    `$dtrace -V` or skip_all("$dtrace unavailable");

    my $result = `$dtrace -qnBEGIN -c'$Perl -e 1' 2>&1`;
    $? && skip_all("Apparently can't probe using $dtrace (perhaps you need root?): $result");
}

use strict;
use warnings;
use IPC::Open2;

plan(tests => 9);

dtrace_like(
    '1',
    'BEGIN { trace(42+666) }',
    qr/708/,
    'really running DTrace',
);

dtrace_like(
    'package My;
        sub outer { Your::inner() }
     package Your;
        sub inner { }
     package Other;
        My::outer();
        Your::inner();',

    'sub-entry { printf("-> %s::%s at %s line %d!\n", copyinstr(arg3), copyinstr(arg0), copyinstr(arg1), arg2) }
     sub-return { printf("<- %s::%s at %s line %d!\n", copyinstr(arg3), copyinstr(arg0), copyinstr(arg1), arg2) }',

     qr/-> My::outer at - line 2!
-> Your::inner at - line 4!
<- Your::inner at - line 4!
<- My::outer at - line 2!
-> Your::inner at - line 4!
<- Your::inner at - line 4!/,

    'traced multiple function calls',
);

dtrace_like(
    '1',
    'phase-change { printf("%s -> %s; ", copyinstr(arg1), copyinstr(arg0)) }',
    qr/START -> RUN; RUN -> DESTRUCT;/,
    'phase changes of a simple script',
);

# this code taken from t/opbasic/magic_phase.t which tests all of the
# transitions of ${^GLOBAL_PHASE}. instead of printing (which will
# interact nondeterministically with the DTrace output), we increment
# an unused variable for side effects
dtrace_like(<< 'MAGIC_OP',
    my $x = 0;
    BEGIN { $x++ }
    CHECK { $x++ }
    INIT  { $x++ }
    sub Moo::DESTROY { $x++ }

    my $tiger = bless {}, Moo::;

    sub Kooh::DESTROY { $x++ }

    our $affe = bless {}, Kooh::;

    END { $x++ }
MAGIC_OP

    'phase-change { printf("%s -> %s; ", copyinstr(arg1), copyinstr(arg0)) }',

     qr/START -> CHECK; CHECK -> INIT; INIT -> RUN; RUN -> END; END -> DESTRUCT;/,

     'phase-changes in a script that exercises all of ${^GLOBAL_PHASE}',
);

dtrace_like(<< 'PHASES',
    my $x = 0;
    sub foo { $x++ }
    sub bar { $x++ }
    sub baz { $x++ }

    INIT { foo() }
    bar();
    END { baz() }
PHASES

    '
    BEGIN { starting = 1 }

    phase-change                            { phase    = arg0 }
    phase-change /copyinstr(arg0) == "RUN"/ { starting = 0 }
    phase-change /copyinstr(arg0) == "END"/ { ending   = 1 }

    sub-entry /copyinstr(arg0) != copyinstr(phase) && (starting || ending)/ {
        printf("%s during %s; ", copyinstr(arg0), copyinstr(phase));
    }
    ',

     qr/foo during INIT; baz during END;/,

     'make sure sub-entry and phase-change interact well',
);

dtrace_like(<< 'PERL_SCRIPT',
    my $tmp = "foo";
    $tmp =~ s/f/b/;
    chop $tmp;
PERL_SCRIPT
    << 'D_SCRIPT',
    op-entry { printf("op-entry <%s>\n", copyinstr(arg0)) }
D_SCRIPT
    [
        qr/op-entry <subst>/,
        qr/op-entry <schop>/,
    ],
    'basic op probe',
);

dtrace_like(<< 'PERL_SCRIPT',
    BEGIN {@INC = '../lib'}
    use strict;
    require HTTP::Tiny;
    do "./run/dtrace.pl";
PERL_SCRIPT
    << 'D_SCRIPT',
    loading-file { printf("loading-file <%s>\n", copyinstr(arg0)) }
    loaded-file  { printf("loaded-file <%s>\n", copyinstr(arg0)) }
D_SCRIPT
    [
        # the original test made sure that each file generated a loading-file then a loaded-file,
        # but that had a race condition when the kernel would push the perl process onto a different
        # CPU, so the DTrace output would appear out of order
        qr{loading-file <strict\.pm>.*loading-file <HTTP/Tiny\.pm>.*loading-file <run/dtrace\.pl>}s,
        qr{loaded-file <strict\.pm>.*loaded-file <HTTP/Tiny\.pm>.*loaded-file <run/dtrace\.pl>}s,
    ],
    'loading-file, loaded-file probes',
);

sub dtrace_like {
    my $perl     = shift;
    my $probes   = shift;
    my $expected = shift;
    my $name     = shift;

    my ($reader, $writer);

    my $pid = open2($reader, $writer,
        $dtrace,
        '-q',
        '-n', 'BEGIN { trace("ready!\n") }', # necessary! see below
        '-n', $probes,
        '-c', $Perl,
    );

    # wait until DTrace tells us that it is initialized
    # otherwise our probes won't properly fire
    chomp(my $throwaway = <$reader>);
    $throwaway eq "ready!" or die "Unexpected 'ready!' result from DTrace: $throwaway";

    # now we can start executing our perl
    print $writer $perl;
    close $writer;

    # read all the dtrace results back in
    local $/;
    my $result = <$reader>;

    # make sure that dtrace is all done and successful
    waitpid($pid, 0);
    my $child_exit_status = $? >> 8;
    die "Unexpected error from DTrace: $result"
        if $child_exit_status != 0;

    if (ref($expected) eq 'ARRAY') {
        like($result, $_, $name) for @$expected;
    }
    else {
        like($result, $expected, $name);
    }
}

