#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config; import Config;
}
if (!$Config{'d_fork'}) {
    skip_all("fork required to pipe");
}
else {
    plan(tests => 27);
}

my $Perl = which_perl();


$| = 1;

open(PIPE, "|-") || exec $Perl, '-pe', 'tr/YX/ko/';

printf PIPE "Xk %d - open |- || exec\n", curr_test();
next_test();
printf PIPE "oY %d -    again\n", curr_test();
next_test();
close PIPE;

{
    if (open(PIPE, "-|")) {
	while(<PIPE>) {
	    s/^not //;
	    print;
	}
	close PIPE;        # avoid zombies
    }
    else {
	printf STDOUT "not ok %d - open -|\n", curr_test();
        next_test();
        my $tnum = curr_test;
        next_test();
	exec $Perl, '-le', "print q{not ok $tnum -     again}";
    }

    # This has to be *outside* the fork
    next_test() for 1..2;

    my $raw = "abc\nrst\rxyz\r\nfoo\n";
    if (open(PIPE, "-|")) {
	$_ = join '', <PIPE>;
	(my $raw1 = $_) =~ s/not ok \d+ - //;
	my @r  = map ord, split //, $raw;
	my @r1 = map ord, split //, $raw1;
        if ($raw1 eq $raw) {
	    s/^not (ok \d+ -) .*/$1 '@r1' passes through '-|'\n/s;
	} else {
	    s/^(not ok \d+ -) .*/$1 expect '@r', got '@r1'\n/s;
	}
	print;
	close PIPE;        # avoid zombies
    }
    else {
	printf STDOUT "not ok %d - $raw", curr_test();
        exec $Perl, '-e0';	# Do not run END()...
    }

    # This has to be *outside* the fork
    next_test();

    if (open(PIPE, "|-")) {
	printf PIPE "not ok %d - $raw", curr_test();
	close PIPE;        # avoid zombies
    }
    else {
	$_ = join '', <STDIN>;
	(my $raw1 = $_) =~ s/not ok \d+ - //;
	my @r  = map ord, split //, $raw;
	my @r1 = map ord, split //, $raw1;
        if ($raw1 eq $raw) {
	    s/^not (ok \d+ -) .*/$1 '@r1' passes through '|-'\n/s;
	} else {
	    s/^(not ok \d+ -) .*/$1 expect '@r', got '@r1'\n/s;
	}
	print;
        exec $Perl, '-e0';	# Do not run END()...
    }

    # This has to be *outside* the fork
    next_test();

    SKIP: {
        skip "fork required", 2 unless $Config{d_fork};

        pipe(READER,WRITER) || die "Can't open pipe";

        if ($pid = fork) {
            close WRITER;
            while(<READER>) {
                s/^not //;
                y/A-Z/a-z/;
                print;
            }
            close READER;     # avoid zombies
        }
        else {
            die "Couldn't fork" unless defined $pid;
            close READER;
            printf WRITER "not ok %d - pipe & fork\n", curr_test;
            next_test;

            open(STDOUT,">&WRITER") || die "Can't dup WRITER to STDOUT";
            close WRITER;
            
            my $tnum = curr_test;
            next_test;
            exec $Perl, '-le', "print q{not ok $tnum -     with fh dup }";
        }

        # This has to be done *outside* the fork.
        next_test() for 1..2;
    }
} 
wait;				# Collect from $pid

pipe(READER,WRITER) || die "Can't open pipe";
close READER;

eval {
    # one platform at least appears to block SIGPIPE by default (see #122112)
    # so make sure it's unblocked.
    # The eval wrapper should ensure this does nothing if these aren't
    # implemented.
    require POSIX;
    my $mask = POSIX::SigSet->new(POSIX::SIGPIPE());
    my $old = POSIX::SigSet->new();
    POSIX::sigprocmask(POSIX::SIG_UNBLOCK(), $mask, $old);
    note "Yes, SIGPIPE was blocked" if $old->ismember(POSIX::SIGPIPE());
};

$SIG{'PIPE'} = 'broken_pipe';

sub broken_pipe {
    $SIG{'PIPE'} = 'IGNORE';       # loop preventer
    printf "ok %d - SIGPIPE\n", curr_test;
}

printf WRITER "not ok %d - SIGPIPE\n", curr_test;
close WRITER;
sleep 1;
next_test;
pass();

SKIP: {
    skip "no fcntl", 1 unless $Config{d_fcntl};
    my($r, $w);
    pipe($r, $w) || die "pipe: $!";
    my $fdr = fileno($r);
    my $fdw = fileno($w);
    fresh_perl_is(qq(
	print open(F, "<&=$fdr") ? 1 : 0, "\\n";
	print open(F, ">&=$fdw") ? 1 : 0, "\\n";
    ), "0\n0\n", {}, "pipe endpoints not inherited across exec");
}

# VMS doesn't like spawning subprocesses that are still connected to
# STDOUT.  Someone should modify these tests to work with VMS.

SKIP: {
    skip "doesn't like spawning subprocesses that are still connected", 10
      if $^O eq 'VMS';

    SKIP: {
        # POSIX-BC doesn't report failure when closing a broken pipe
        # that has pending output.  Go figure.
        skip "Won't report failure on broken pipe", 1
          if $^O eq 'posix-bc';

        local $SIG{PIPE} = 'IGNORE';
        open NIL, qq{|$Perl -e "exit 0"} or die "open failed: $!";
        sleep 5;
        if (print NIL 'foo') {
            # If print was allowed we had better get an error on close
            ok( !close NIL,     'close error on broken pipe' );
        }
        else {
            ok(close NIL,       'print failed on broken pipe');
        }
    }

    {
        # check that errno gets forced to 0 if the piped program exited 
        # non-zero
        open NIL, qq{|$Perl -e "exit 23";} or die "fork failed: $!";
        $! = 1;
        ok(!close NIL,  'close failure on non-zero piped exit');
        is($!, '',      '       errno');
        isnt($?, 0,     '       status');

	# Former skip block:
        {
            # check that status for the correct process is collected
            my $zombie;
            unless( $zombie = fork ) {
                $NO_ENDING=1;
                exit 37;
            }
            my $pipe = open *FH, "sleep 2;exit 13|" or die "Open: $!\n";
            $SIG{ALRM} = sub { return };
            alarm(1);
            is( close FH, '',   'close failure for... umm, something' );
            is( $?, 13*256,     '       status' );
            is( $!, '',         '       errno');

            my $wait = wait;
            is( $?, 37*256,     'status correct after wait' );
            is( $wait, $zombie, '       wait pid' );
            is( $!, '',         '       errno');
        }
    }
}

# Test new semantics for missing command in piped open
# 19990114 M-J. Dominus mjd@plover.com
{ local *P;
  no warnings 'pipe';
  ok( !open(P, "|    "),        'missing command in piped open input' );
  ok( !open(P, "     |"),       '                              output');
}

# check that status is unaffected by implicit close
{
    local(*NIL);
    open NIL, qq{|$Perl -e "exit 23"} or die "fork failed: $!";
    $? = 42;
    # NIL implicitly closed here
}
is($?, 42,      'status unaffected by implicit close');
$? = 0;

# check that child is reaped if the piped program can't be executed
SKIP: {
  skip "/no_such_process exists", 1 if -e "/no_such_process";
  open NIL, '/no_such_process |';
  close NIL;

  my $child = 0;
  eval {
    local $SIG{ALRM} = sub { die; };
    alarm 2;
    $child = wait;
    alarm 0;
  };

  is($child, -1, 'child reaped if piped program cannot be executed');
}

{
    # [perl #122112] refcnt: fd -1 < 0 when a signal handler dies
    # while a pipe close is waiting on a child process
    my $prog = <<PROG;
\$SIG{ALRM}=sub{die};
alarm 1;
\$Perl = "$Perl";
my \$cmd = qq(\$Perl -e "sleep 3");
my \$pid = open my \$fh, "|\$cmd" or die "\$!\n";
close \$fh;
PROG
    my $out = fresh_perl($prog, {});
    cmp_ok($out, '!~', qr/refcnt/, "no exception from PerlIO");
    # checks that that program did something rather than failing to
    # compile
    cmp_ok($out, '=~', qr/Died at/, "but we did get the exception from die");
}
