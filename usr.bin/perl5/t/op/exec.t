#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

my $vms_exit_mode = 0;

if ($^O eq 'VMS') {
    if (eval 'require VMS::Feature') {
        $vms_exit_mode = !(VMS::Feature::current("posix_exit"));
    } else {
        my $env_unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $env_posix_ex = $ENV{'PERL_VMS_POSIX_EXIT'} || '';
        my $unix_rpt = $env_unix_rpt =~ /^[ET1]/i; 
        my $posix_ex = $env_posix_ex =~ /^[ET1]/i;
        if (($unix_rpt || $posix_ex) ) {
            $vms_exit_mode = 0;
        } else {
            $vms_exit_mode = 1;
        }
    }
}


# suppress VMS whinging about bad execs.
use vmsish qw(hushed);

$| = 1;				# flush stdout

$ENV{LC_ALL}   = 'C';		# Force English error messages.
$ENV{LANGUAGE} = 'C';		# Ditto in GNU.

my $Is_VMS   = $^O eq 'VMS';
my $Is_Win32 = $^O eq 'MSWin32';

plan(tests => 41);

my $Perl = which_perl();

my $exit;
SKIP: {
    skip("bug/feature of pdksh", 2) if $^O eq 'os2';

    my $tnum = curr_test();
    $exit = system qq{$Perl -le "print q{ok $tnum - interp system(EXPR)"}};
    next_test();
    is( $exit, 0, '  exited 0' );
}

my $tnum = curr_test();
$exit = system qq{$Perl -le "print q{ok $tnum - split & direct system(EXPR)"}};
next_test();
is( $exit, 0, '  exited 0' );

# On VMS and Win32 you need the quotes around the program or it won't work.
# On Unix its the opposite.
my $quote = $Is_VMS || $Is_Win32 ? '"' : '';
$tnum = curr_test();
$exit = system $Perl, '-le', 
               "${quote}print q{ok $tnum - system(PROG, LIST)}${quote}";
next_test();
is( $exit, 0, '  exited 0' );


# Some basic piped commands.  Some OS's have trouble with "helpfully"
# putting newlines on the end of piped output.  So we split this into
# newline insensitive and newline sensitive tests.
my $echo_out = `$Perl -e "print 'ok'" | $Perl -le "print <STDIN>"`;
$echo_out =~ s/\n\n/\n/g;
is( $echo_out, "ok\n", 'piped echo emulation');

{
    # here we check if extra newlines are going to be slapped on
    # piped output.
    local $TODO = 'VMS sticks newlines on everything' if $Is_VMS;

    is( scalar `$Perl -e "print 'ok'"`,
        "ok", 'no extra newlines on ``' );

    is( scalar `$Perl -e "print 'ok'" | $Perl -e "print <STDIN>"`, 
        "ok", 'no extra newlines on pipes');

    is( scalar `$Perl -le "print 'ok'" | $Perl -le "print <STDIN>"`, 
        "ok\n\n", 'doubled up newlines');

    is( scalar `$Perl -e "print 'ok'" | $Perl -le "print <STDIN>"`, 
        "ok\n", 'extra newlines on inside pipes');

    is( scalar `$Perl -le "print 'ok'" | $Perl -e "print <STDIN>"`, 
        "ok\n", 'extra newlines on outgoing pipes');

    {
	local($/) = \2;       
	$out = runperl(prog => 'print q{1234}');
	is($out, "1234", 'ignore $/ when capturing output in scalar context');
    }
}


is( system(qq{$Perl -e "exit 0"}), 0,     'Explicit exit of 0' );

my $exit_one = $vms_exit_mode ? 4 << 8 : 1 << 8;
is( system(qq{$Perl "-I../lib" -e "use vmsish qw(hushed); exit 1"}), $exit_one,
    'Explicit exit of 1' );

$rc = system { "lskdfj" } "lskdfj";
unless( ok($rc == 255 << 8 or $rc == -1 or $rc == 256 or $rc == 512) ) {
    print "# \$rc == $rc\n";
}

unless ( ok( $! == 2  or  $! =~ /\bno\b.*\bfile/i or  
             $! == 13 or  $! =~ /permission denied/i or
             $! == 20 or  $! =~ /not a directory/i or   # If PATH component is
                                                        # a non-directory
             $! == 22 or  $! =~ /invalid argument/i  ) ) {
    diag sprintf "\$! eq %d, '%s'\n", $!, $!;
}


is( `$Perl -le "print 'ok'"`,   "ok\n",     'basic ``' );
is( <<`END`,                    "ok\n",     '<<`HEREDOC`' );
$Perl -le "print 'ok'"
END

is( <<~`END`,                   "ok\n",     '<<~`HEREDOC`' );
  $Perl -le "print 'ok'"
  END

{
    sub rpecho { qq($Perl -le "print '$_[0]'") }
    is scalar(readpipe(rpecho("b"))), "b\n",
	"readpipe with one argument in scalar context";
    is join(",", "a", readpipe(rpecho("b")), "c"), "a,b\n,c",
	"readpipe with one argument in list context";
    local $_ = rpecho("f");
    is scalar(readpipe), "f\n",
	"readpipe default argument in scalar context";
    is join(",", "a", readpipe, "c"), "a,f\n,c",
	"readpipe default argument in list context";
    sub rpechocxt {
	rpecho(wantarray ? "list" : defined(wantarray) ? "scalar" : "void");
    }
    is scalar(readpipe(rpechocxt())), "scalar\n",
	"readpipe argument context in scalar context";
    is join(",", "a", readpipe(rpechocxt()), "b"), "a,scalar\n,b",
	"readpipe argument context in list context";
    foreach my $args ("(\$::p,\$::q)", "((\$::p,\$::q))") {
	foreach my $lvalue ("my \$r", "my \@r") {
	    eval("$lvalue = readpipe$args if 0");
	    like $@, qr/\AToo many arguments for /;
	}
    }
}

package o {
    use subs "readpipe";
    sub readpipe { pop }
    ::is `${\"hello"}`, 'hello',
         'overridden `` interpolates [perl #115330]';
    ::is <<`119827`, "ls\n",
l${\"s"}
119827
        '<<`` respects overrides and interpolates [perl #119827]';
}

TODO: {
    my $tnum = curr_test();
    if( $^O =~ /Win32/ ) {
        print "not ok $tnum - exec failure doesn't terminate process " .
              "# TODO Win32 exec failure waits for user input\n";
        next_test();
        last TODO;
    }

    ok( !exec("lskdjfalksdjfdjfkls"), 
        "exec failure doesn't terminate process");
}

{
    local $! = 0;
    ok !exec(), 'empty exec LIST fails';
    ok $! == 2 || $! =~ qr/\bno\b.*\bfile\b/i, 'errno = ENOENT'
        or diag sprintf "\$! eq %d, '%s'\n", $!, $!;

}
{
    local $! = 0;
    my $err = $!;
    ok !(exec {""} ()), 'empty exec PROGRAM LIST fails';
    ok $! == 2 || $! =~ qr/\bno\b.*\bfile\b/i, 'errno = ENOENT'
        or diag sprintf "\$! eq %d, '%s'\n", $!, $!;
}

package CountRead {
    sub TIESCALAR { bless({ n => 0 }, $_[0]) }
    sub FETCH { ++$_[0]->{n} }
}
my $cr;
tie $cr, "CountRead";
my $exit_statement = "exit(\$ARGV[0] eq '1' ? 0 : 1)";
$exit_statement = qq/"$exit_statement"/ if $^O eq 'VMS';
is system($^X, "-e", $exit_statement, $cr), 0,
    "system args have magic processed exactly once";
is tied($cr)->{n}, 1, "system args have magic processed before fork";

$exit_statement = "exit(\$ARGV[0] eq \$ARGV[1] ? 0 : 1)";
$exit_statement = qq/"$exit_statement"/ if $^O eq 'VMS';
is system($^X, "-e", $exit_statement, "$$", $$), 0,
    "system args have magic processed before fork";

my $test = curr_test();
exec $Perl, '-le', qq{${quote}print 'ok $test - exec PROG, LIST'${quote}};
fail("This should never be reached if the exec() worked");
