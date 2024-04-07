#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

use strict;
use warnings;
use Config;

delete $ENV{PERLDB_OPTS};

BEGIN {
    if (! -c "/dev/null") {
        print "1..0 # Skip: no /dev/null\n";
        exit 0;
    }

    my $dev_tty = '/dev/tty';
    $dev_tty = 'TT:' if ($^O eq 'VMS');
    if (! -c $dev_tty) {
        print "1..0 # Skip: no $dev_tty\n";
        exit 0;
    }
    if ($ENV{PERL5DB}) {
        print "1..0 # Skip: \$ENV{PERL5DB} is already set to '$ENV{PERL5DB}'\n";
        exit 0;
    }
    $ENV{PERL_RL} = 'Perl'; # Suppress system Term::ReadLine::Gnu
}

my $rc_filename = '.perldb';

sub rc {
    open my $rc_fh, '>', $rc_filename
        or die $!;
    print {$rc_fh} @_;
    close ($rc_fh);

    # overly permissive perms gives "Must not source insecure rcfile"
    # and hangs at the DB(1> prompt
    chmod 0644, $rc_filename;
}

sub _slurp
{
    my $filename = shift;

    open my $in, '<', $filename
        or die "Cannot open '$filename' for slurping - $!";

    local $/;
    my $contents = <$in>;

    close($in);

    return $contents;
}

my $out_fn = 'db.out';

sub _out_contents
{
    return _slurp($out_fn);
}


# Test for Proxy constants
{
    rc(
        <<'EOF',

&parse_options("NonStop=0 ReadLine=0 TTY=db.out");

sub afterinit {
    push(@DB::typeahead,
        'm main->s1',
        'q',
    );
}

EOF
    );

    my $output = runperl(switches => [ '-d' ], stderr => 1, progfile => '../lib/perl5db/t/proxy-constants');
    is($output, "", "proxy constant subroutines");
}

# [perl #66110] Call a subroutine inside a regex
{
    local $ENV{PERLDB_OPTS} = "ReadLine=0 NonStop=1";
    my $output = runperl(switches => [ '-d' ], stderr => 1, progfile => '../lib/perl5db/t/rt-66110');
    like($output, qr/\bAll tests successful\.$/, "[perl #66110]");
}
# [ perl #116769] Frame=2
{
    local $ENV{PERLDB_OPTS} = "frame=2 nonstop";
    my $output = runperl( switches => [ '-d' ], prog => 'print qq{success\n}' );
    is( $?, 0, '[perl #116769] frame=2 does not crash debugger, exit == 0' );
    is( $output, "success\n" , '[perl #116769] code is run' );
}
# [ perl #116771] autotrace
{
    local $ENV{PERLDB_OPTS} = "autotrace nonstop";
    my $output = runperl( switches => [ '-d' ], prog => 'print qq{success\n}' );
    is( $?, 0, '[perl #116771] autotrace does not crash debugger, exit == 0' );
    is( $output, "success\n" , '[perl #116771] code is run' );
}
# [ perl #41461] Frame=2 noTTY
{
    local $ENV{PERLDB_OPTS} = "frame=2 noTTY nonstop";
    rc('');
    my $output = runperl( switches => [ '-d' ], prog => 'print qq{success\n}' );
    is( $?, 0, '[perl #41461] frame=2 noTTY does not crash debugger, exit == 0' );
    is( $output, "success\n" , '[perl #41461] code is run' );
}

package DebugWrap;

sub new {
    my $class = shift;

    my $self = bless {}, $class;

    $self->_init(@_);

    return $self;
}

sub _cmds {
    my $self = shift;

    if (@_) {
        $self->{_cmds} = shift;
    }

    return $self->{_cmds};
}

sub _prog {
    my $self = shift;

    if (@_) {
        $self->{_prog} = shift;
    }

    return $self->{_prog};
}

sub _output {
    my $self = shift;

    if (@_) {
        $self->{_output} = shift;
    }

    return $self->{_output};
}

sub _include_t
{
    my $self = shift;

    if (@_)
    {
        $self->{_include_t} = shift;
    }

    return $self->{_include_t};
}

sub _stderr_val
{
    my $self = shift;

    if (@_)
    {
        $self->{_stderr_val} = shift;
    }

    return $self->{_stderr_val};
}

sub field
{
    my $self = shift;

    if (@_)
    {
        $self->{field} = shift;
    }

    return $self->{field};
}

sub _switches
{
    my $self = shift;

    if (@_)
    {
        $self->{_switches} = shift;
    }

    return $self->{_switches};
}

sub _contents
{
    my $self = shift;

    if (@_)
    {
        $self->{_contents} = shift;
    }

    return $self->{_contents};
}

# object for prog temporary file
sub _tempprog
{
    my $self = shift;

    if (@_)
    {
        $self->{_tempprog} = shift;
    }

    return $self->{_tempprog};
}

sub _init
{
    my ($self, $args) = @_;

    my $cmds = $args->{cmds};

    if (ref($cmds) ne 'ARRAY') {
        die "cmds must be an array of commands.";
    }

    $self->_cmds($cmds);

    my $prog = $args->{prog};

    if (ref($prog) eq 'SCALAR') {
        use File::Temp;
        my $fh = File::Temp->new;
        $self->_tempprog($fh);
        print $fh $$prog;
        $prog = $fh->filename;
    }
    elsif (ref($prog) ne '' or !defined($prog)) {
        die "prog should be a path to a program file.";
    }

    $self->_prog($prog);

    $self->_include_t($args->{include_t} ? 1 : 0);

    $self->_stderr_val(exists($args->{stderr}) ? $args->{stderr} : 1);

    if (exists($args->{switches}))
    {
        $self->_switches($args->{switches});
    }

    $self->_run();

    return;
}

sub _quote
{
    my ($self, $str) = @_;

    $str =~ s/(["\@\$\\])/\\$1/g;
    $str =~ s/\n/\\n/g;
    $str =~ s/\r/\\r/g;

    return qq{"$str"};
}

sub _run {
    my $self = shift;

    my $rc = qq{&parse_options("NonStop=0 TTY=db.out");\n};

    $rc .= join('',
        map { "$_\n"}
        (q#sub afterinit {#,
         q#push (@DB::typeahead,#,
         (map { $self->_quote($_) . "," } @{$self->_cmds()}),
         q#);#,
         q#}#,
        )
    );

    # I guess two objects like that cannot be used at the same time.
    # Oh well.
    ::rc($rc);

    my $output =
        ::runperl(
            switches =>
            [
                ($self->_switches ? (@{$self->_switches()}) : ('-d')),
                ($self->_include_t ? ('-I', '../lib/perl5db/t') : ())
            ],
            (defined($self->_stderr_val())
                ? (stderr => $self->_stderr_val())
                : ()
            ),
            progfile => $self->_prog()
        );

    $self->_output($output);

    $self->_contents(::_out_contents());

    return;
}

sub get_output
{
    return shift->_output();
}

sub output_like {
    my ($self, $re, $msg) = @_;

    local $::Level = $::Level + 1;
    ::like($self->_output(), $re, $msg);
}

sub output_unlike {
    my ($self, $re, $msg) = @_;

    local $::Level = $::Level + 1;
    ::unlike($self->_output(), $re, $msg);
}

sub get_contents {
    return shift->_contents();
}

sub contents_like {
    my ($self, $re, $msg) = @_;

    local $::Level = $::Level + 1;
    ::like($self->_contents(), $re, $msg);
}

sub contents_unlike {
    my ($self, $re, $msg) = @_;

    local $::Level = $::Level + 1;
    ::unlike($self->_contents(), $re, $msg);
}

=head1 NAME

DebugWrap - wrapper to execute code under the debugger and examine the
results.

=head1 SYNOPSIS

    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                # list of commands supplied to the debugger
            ],
            prog => 'filename_of_code_to_debug.pl',
            # and some optional arguments
        }
    );

    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                # list of commands supplied to the debugger
            ],
            prog => \<<'EOS',
    # perl code to debug
    EOS
            # and some optional arguments
        }
    );

    # test the output from the program being debugged
    $wrapper->output_like(qr/.../, "describe the test");
    $wrapper->output_unlike(qr/.../, "describe the test");
    my $output = $wrapper->get_output; # for more sophisticated checks

    # test the output from the debugger
    $wrapper->contents_like(qr/.../, "describe the test");
    $wrapper->contents_unlike(qr/.../, "describe the test");
    my $contents = $wrapper->get_contents; # for more sophisticated checks

=head1 DESCRIPTION

DebugWrap is a simple class used when testing the Perl debugger that
executes a set of debugger commands against a program under the
debugger and provides some simple methods to examine the results.

It is not installed to your system.

=head2 Creating a DebugWrap object

The constructor new() accepts a hash of arguments, with the following
possible members:

=over

=item cmds

An array of commands to execute, one command per element.  Required.

=item prog

Either the name of a perl program to test under the debugger, or a
reference to a scalar containing the text of the program to test.
Required.

=item stderr

If this is a true value capture standard error, which is the default.
Optional.

=item include_t

Add F<lib/perl5db/t> to the perl search path, as with C<-I>

=item switches

An arrayref of switches to supply to perl.  This should include the
C<-d> switch needed to invoke the debugger.  If C<switches> is not
supplied then C<-d> only is supplied.  The C<-I> for C<include_t> is
added after these switches.

=back

=head2 Other methods

The other methods intended for test usage are:

=over

=item $wrapper->get_contents

Fetch the debugger output from the debugger run.  This does not
include the output from the program under test.

=item $wrapper->contents_like($re, $test_name)

Test that the debugger output matches the given regular expression
object (as with qr//).

Equivalent to:

  like($wrapper->get_contents, $re, $test_name);

=item $wrapper->contents_unlike($re, $test_name)

Test that the debugger output does not match the given regular
expression object (as with qr//).

Equivalent to:

  unlike($wrapper->get_contents, $re, $test_name);

=item $wrapper->get_output

Fetch the program output from the debugger run.  This does not include
the output from the debugger itself, it does include the output
generated by C<valgrind> or ASAN, assuming you haven't disabled
capturing stderr.

=item $wrapper->output_like($re, $test_name);

Test that the program output matches the given regular expression
object (as with qr//).

Equivalent to:

  like($wrapper->get_output, $re, $test_name);

=item $wrapper->output_unlike($re, $test_name);

Test that the program output does not match the given regular
expression object (as with qr//).

Equivalent to:

  unlike($wrapper->get_output, $re, $test_name);

=back

=cut

package main;

{
    local $ENV{PERLDB_OPTS} = "ReadLine=0";
    my $target = '../lib/perl5db/t/eval-line-bug';
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 23',
                'n',
                'n',
                'n',
                'c', # line 23
                'n',
                "p \@{'main::_<$target'}",
                'q',
            ],
            prog => $target,
        }
    );
    $wrapper->contents_like(
        qr/sub factorial/,
        'The ${main::_<filename} variable in the debugger was not destroyed',
    );
}

sub _calc_generic_wrapper
{
    my $args = shift;

    my $extra_opts = delete($args->{extra_opts});
    $extra_opts ||= '';
    local $ENV{PERLDB_OPTS} = "ReadLine=0" . $extra_opts;
    return DebugWrap->new(
        {
            cmds => delete($args->{cmds}),
            prog => delete($args->{prog}),
            %$args,
        }
    );
}

sub _calc_new_var_wrapper
{
    my ($args) = @_;
    return _calc_generic_wrapper(
        {
            cmds =>
            [
                'b 23',
                'c',
                '$new_var = "Foo"',
                'x "new_var = <$new_var>\\n"',
                'q',
            ],
            %$args,
        }
    );
}

sub _calc_threads_wrapper
{
    my $args = shift;

    return _calc_new_var_wrapper(
        {
            switches => [ '-dt', ],
            stderr => 1,
            %$args
        }
    );
}

{
    _calc_new_var_wrapper({ prog => '../lib/perl5db/t/eval-line-bug'})
        ->contents_like(
            qr/new_var = <Foo>/,
            "no strict 'vars' in evaluated lines.",
        );
}

{
    _calc_new_var_wrapper(
        {
            prog => '../lib/perl5db/t/lvalue-bug',
            stderr => undef(),
        },
    )->output_like(
            qr/foo is defined/,
             'lvalue subs work in the debugger',
         );
}

{
    _calc_new_var_wrapper(
        {
            prog =>  '../lib/perl5db/t/symbol-table-bug',
            extra_opts => "NonStop=1",
            stderr => undef(),
        }
    )->output_like(
        qr/Undefined symbols 0/,
        'there are no undefined values in the symbol table',
    );
}

SKIP:
{
    if ( $Config{usethreads} ) {
        skip('This perl has threads, skipping non-threaded debugger tests');
    }
    else {
        my $error = 'This Perl not built to support threads';
        _calc_threads_wrapper(
            {
                prog => '../lib/perl5db/t/eval-line-bug',
            }
        )->output_like(
            qr/\Q$error\E/,
            'Perl debugger correctly complains that it was not built with threads',
        );
    }
}

SKIP:
{
    if ( $Config{usethreads} ) {
        _calc_threads_wrapper(
            {
                prog =>  '../lib/perl5db/t/symbol-table-bug',
            }
        )->output_like(
            qr/Undefined symbols 0/,
            'there are no undefined values in the symbol table when running with thread support',
        );
    }
    else {
        skip("This perl is not threaded, skipping threaded debugger tests");
    }
}

# Test [perl #61222]
{
    local $ENV{PERLDB_OPTS};
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'm Pie',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-61222',
        }
    );

    $wrapper->contents_unlike(qr/INCORRECT/, "[perl #61222]");
}

sub _calc_trace_wrapper
{
    my ($args) = @_;

    return _calc_generic_wrapper(
        {
            cmds =>
            [
                't 2',
                'c',
                'q',
            ],
            %$args,
        }
    );
}

# [perl 104168] level option for tracing
{
    my $wrapper = _calc_trace_wrapper({ prog =>  '../lib/perl5db/t/rt-104168' });
    $wrapper->contents_like(qr/level 2/, "[perl #104168] - level 2 appears");
    $wrapper->contents_unlike(qr/baz/, "[perl #104168] - no 'baz'");
}

# taint tests
if (!exists($Config{taint_support}) || $Config{taint_support})
{
    my $wrapper = _calc_trace_wrapper(
        {
            prog => '../lib/perl5db/t/taint',
            extra_opts => ' NonStop=1',
            switches => [ '-d', '-T', ],
        }
    );

    my $output = $wrapper->get_output();
    chomp $output if $^O eq 'VMS'; # newline guaranteed at EOF
    is($output, '[$^X][done]', "taint");
}

# Testing that we can set a line in the middle of the file.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b ../lib/perl5db/t/MyModule.pm:12',
                'c',
                q/do { use IO::Handle; STDOUT->autoflush(1); print "Var=$var\n"; }/,
                'c',
                'q',
            ],
            include_t => 1,
            prog => '../lib/perl5db/t/filename-line-breakpoint'
        }
    );

    $wrapper->output_like(qr/
        ^Var=Bar$
            .*
        ^In\ MyModule\.$
            .*
        ^In\ Main\ File\.$
            .*
        /msx,
        "Can set breakpoint in a line in the middle of the file.");
}

# Testing that we can set a breakpoint
{
    my $wrapper = DebugWrap->new(
        {
            prog => '../lib/perl5db/t/breakpoint-bug',
            cmds =>
            [
                'b 6',
                'c',
                q/do { use IO::Handle; STDOUT->autoflush(1); print "X={$x}\n"; }/,
                'c',
                'q',
            ],
        },
    );

    $wrapper->output_like(
        qr/X=\{Two\}/msx,
        "Can set breakpoint in a line."
    );
}

# Testing that we can disable a breakpoint at a numeric line.
{
    my $wrapper = DebugWrap->new(
        {
            prog =>  '../lib/perl5db/t/disable-breakpoints-1',
            cmds =>
            [
                'b 7',
                'b 11',
                'disable 7',
                'c',
                q/print "X={$x}\n";/,
                'c',
                'q',
            ],
        }
    );

    $wrapper->output_like(qr/X=\{SecondVal\}/ms,
        "Can set breakpoint in a line.");
}

# Testing that we can re-enable a breakpoint at a numeric line.
{
    my $wrapper = DebugWrap->new(
        {
            prog =>  '../lib/perl5db/t/disable-breakpoints-2',
            cmds =>
            [
                'b 8',
                'b 24',
                'disable 24',
                'c',
                'enable 24',
                'c',
                q/print "X={$x}\n";/,
                'c',
                'q',
            ],
        },
    );

    $wrapper->output_like(
        qr/
        X=\{SecondValOneHundred\}
        /msx,
        "Can set breakpoint in a line."
    );
}
# clean up.

# Disable and enable for breakpoints on outer files.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 10',
                'b ../lib/perl5db/t/EnableModule.pm:14',
                'disable ../lib/perl5db/t/EnableModule.pm:14',
                'c',
                'enable ../lib/perl5db/t/EnableModule.pm:14',
                'c',
                q/print "X={$x}\n";/,
                'c',
                'q',
            ],
            prog =>  '../lib/perl5db/t/disable-breakpoints-3',
            include_t => 1,
        }
    );

    $wrapper->output_like(qr/
        X=\{SecondValTwoHundred\}
        /msx,
        "Can set breakpoint in a line.");
}

# Testing that the prompt with the information appears.
{
    my $wrapper = DebugWrap->new(
        {
            cmds => ['q'],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr/
        ^main::\([^\)]*\bdisable-breakpoints-1:2\):\n
        2:\s+my\ \$x\ =\ "One";\n
        /msx,
        "Prompt should display the first line of code.");
}

# Testing that R (restart) and "B *" work.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 13',
                'c',
                'B *',
                'b 9',
                'R',
                'c',
                q/print "X={$x};dummy={$dummy}\n";/,
                'q',
            ],
            prog =>  '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->output_like(qr/
        X=\{FirstVal\};dummy=\{1\}
        /msx,
        "Restart and delete all breakpoints work properly.");
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c 15',
                q/print "X={$x}\n";/,
                'c',
                'q',
            ],
            prog =>  '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->output_like(qr/
        X=\{ThirdVal\}
        /msx,
        "'c line_num' is working properly.");
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                'n',
                'b . $exp > 200',
                'c',
                q/print "Exp={$exp}\n";/,
                'q',
            ],
            prog => '../lib/perl5db/t/break-on-dot',
        }
    );

    $wrapper->output_like(qr/
        Exp=\{256\}
        /msx,
        "'b .' is working correctly.");
}

# Testing that the prompt with the information appears inside a subroutine call.
# See https://rt.perl.org/rt3/Ticket/Display.html?id=104820
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c back',
                'q',
            ],
            prog => '../lib/perl5db/t/with-subroutine',
        }
    );

    $wrapper->contents_like(
        qr/
        ^main::back\([^\)\n]*\bwith-subroutine:15\):[\ \t]*\n
        ^15:\s*print\ "hello\ back\\n";
        /msx,
        "Prompt should display the line of code inside a subroutine.");
}

# Checking that the p command works.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'p "<<<" . (4*6) . ">>>"',
                'q',
            ],
            prog => '../lib/perl5db/t/with-subroutine',
        }
    );

    $wrapper->contents_like(
        qr/<<<24>>>/,
        "p command works.");
}

# Tests for x.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/x {500 => 600}/,
                'q',
            ],
            prog => '../lib/perl5db/t/with-subroutine',
        }
    );

    $wrapper->contents_like(
        # qr/^0\s+HASH\([^\)]+\)\n\s+500 => 600\n/,
        qr/^0\s+HASH\([^\)]+\)\n\s+500 => 600\n/ms,
        "x command test."
    );
}

# Tests for x with @_
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 10',
                'c',
                'x @_',
                'q',
            ],
            prog => '../lib/perl5db/t/test-passing-at-underscore-to-x-etc',
        }
    );

    $wrapper->contents_like(
        # qr/^0\s+HASH\([^\)]+\)\n\s+500 => 600\n/,
        qr/Arg1.*?Capsula.*GreekHumor.*Socrates/ms,
        q/x command test with '@_'./,
    );
}

# Tests for mutating @_
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 10',
                'c',
                'shift(@_)',
                'print "\n\n\n(((" . join(",", @_) . ")))\n\n\n"',
                'q',
            ],
            prog => '../lib/perl5db/t/test-passing-at-underscore-to-x-etc',
        }
    );

    $wrapper->output_like(
        qr/^\(\(\(Capsula,GreekHumor,Socrates\)\)\)$/ms,
        q/Mutating '@_'./,
    );
}

# Tests for x with AutoTrace=1.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                'o AutoTrace=1',
                # So it may fail.
                q/x "failure"/,
                q/x \$x/,
                'q',
            ],
            prog => '../lib/perl5db/t/with-subroutine',
        }
    );

    $wrapper->contents_like(
        # qr/^0\s+HASH\([^\)]+\)\n\s+500 => 600\n/,
        qr/^0\s+SCALAR\([^\)]+\)\n\s+-> 'hello world'\n/ms,
        "x after AutoTrace=1 command is working."
    );
}

# Tests for "T" (stack trace).
{
    my $prog_fn = '../lib/perl5db/t/rt-104168';
    my $wrapper = DebugWrap->new(
        {
            prog => $prog_fn,
            cmds =>
            [
                'c baz',
                'T',
                'q',
            ],
        }
    );
    my $re_text = join('',
        map {
        sprintf(
            "%s = %s\\(\\) called from file " .
            "'" . quotemeta($prog_fn) . "' line %s\\n",
            (map { quotemeta($_) } @$_)
            )
        }
        (
            ['.', 'main::baz', 14,],
            ['.', 'main::bar', 9,],
            ['.', 'main::foo', 6],
        )
    );
    $wrapper->contents_like(
        # qr/^0\s+HASH\([^\)]+\)\n\s+500 => 600\n/,
        qr/^$re_text/ms,
        "T command test."
    );
}

# Test for s.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 9',
                'c',
                's',
                q/print "X={$x};dummy={$dummy}\n";/,
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1'
        }
    );

    $wrapper->output_like(qr/
        X=\{SecondVal\};dummy=\{1\}
        /msx,
        'test for s - single step',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                'n',
                'b . $exp > 200',
                'c',
                q/print "Exp={$exp}\n";/,
                'q',
            ],
            prog => '../lib/perl5db/t/break-on-dot'
        }
    );

    $wrapper->output_like(qr/
        Exp=\{256\}
        /msx,
        "'b .' is working correctly.");
}

{
    my $prog_fn = '../lib/perl5db/t/rt-104168';
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                's',
                'q',
            ],
            prog => $prog_fn,
        }
    );

    $wrapper->contents_like(
        qr/
        ^main::foo\([^\)\n]*\brt-104168:9\):[\ \t]*\n
        ^9:\s*bar\(\);
        /msx,
        'Test for the s command.',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                's uncalled_subroutine()',
                'c',
                'q',
            ],

            prog => '../lib/perl5db/t/uncalled-subroutine'}
    );

    $wrapper->output_like(
        qr/<1,2,3,4,5>\n/,
        'uncalled_subroutine was called after s EXPR()',
        );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n uncalled_subroutine()',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/uncalled-subroutine',
        }
    );

    $wrapper->output_like(
        qr/<1,2,3,4,5>\n/,
        'uncalled_subroutine was called after n EXPR()',
        );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b fact',
                'c',
                'c',
                'c',
                'n',
                'print "<$n>"',
                'q',
            ],
            prog => '../lib/perl5db/t/fact',
        }
    );

    $wrapper->output_like(
        qr/<3>/,
        'b subroutine works fine',
    );
}

# Test for n with lvalue subs
DebugWrap->new({
    cmds =>
    [
        'n', 'print "<$x>\n"',
        'n', 'print "<$x>\n"',
        'q',
    ],
    prog => '../lib/perl5db/t/lsub-n',
})->output_like(
    qr/<1>\n<11>\n/,
    'n steps over lvalue subs',
);

# Test for 'M' (module list).
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'M',
                'q',
            ],
            prog => '../lib/perl5db/t/load-modules'
        }
    );

    $wrapper->contents_like(
        qr[Scalar/Util\.pm],
        'M (module list) works fine',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 14',
                'c',
                '$flag = 1;',
                'r',
                'print "Var=$var\n";',
                'q',
            ],
            prog => '../lib/perl5db/t/test-r-statement',
        }
    );

    $wrapper->output_like(
        qr/
            ^Foo$
                .*?
            ^Bar$
                .*?
            ^Var=Test$
        /msx,
        'r statement is working properly.',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l',
                'q',
            ],
            prog => '../lib/perl5db/t/test-l-statement-1',
        }
    );

    $wrapper->contents_like(
        qr/
            ^1==>\s+\$x\ =\ 1;\n
            2:\s+print\ "1\\n";\n
            3\s*\n
            4:\s+\$x\ =\ 2;\n
            5:\s+print\ "2\\n";\n
        /msx,
        'l statement is working properly (test No. 1).',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l',
                q/# After l 1/,
                'l',
                q/# After l 2/,
                '-',
                q/# After -/,
                'q',
            ],
            prog => '../lib/perl5db/t/test-l-statement-1',
        }
    );

    my $first_l_out = qr/
        1==>\s+\$x\ =\ 1;\n
        2:\s+print\ "1\\n";\n
        3\s*\n
        4:\s+\$x\ =\ 2;\n
        5:\s+print\ "2\\n";\n
        6\s*\n
        7:\s+\$x\ =\ 3;\n
        8:\s+print\ "3\\n";\n
        9\s*\n
        10:\s+\$x\ =\ 4;\n
    /msx;

    my $second_l_out = qr/
        11:\s+print\ "4\\n";\n
        12\s*\n
        13:\s+\$x\ =\ 5;\n
        14:\s+print\ "5\\n";\n
        15\s*\n
        16:\s+\$x\ =\ 6;\n
        17:\s+print\ "6\\n";\n
        18\s*\n
        19:\s+\$x\ =\ 7;\n
        20:\s+print\ "7\\n";\n
    /msx;
    $wrapper->contents_like(
        qr/
            ^$first_l_out
            [^\n]*?DB<\d+>\ \#\ After\ l\ 1\n
            [\ \t]*\n
            [^\n]*?DB<\d+>\ l\s*\n
            $second_l_out
            [^\n]*?DB<\d+>\ \#\ After\ l\ 2\n
            [\ \t]*\n
            [^\n]*?DB<\d+>\ -\s*\n
            $first_l_out
            [^\n]*?DB<\d+>\ \#\ After\ -\n
        /msx,
        'l followed by l and then followed by -',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'v',
                'q',
            ],
            prog => '../lib/perl5db/t/test-l-statement-1',
        }
       );
    $wrapper->contents_like(
        qr/
          1==>\s+\$x\ =\ 1;\n
          2:\s+print\ "1\\n";\n
          3\s+\n
          4:\s+\$x\ =\ 2;\n
          5:\s+print\ "2\\n";\n
          6\s*\n
          7:\s+\$x\ =\ 3;\n
          /msx,
        "test plain v"
        );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'v 10',
                'q',
            ],
            prog => '../lib/perl5db/t/test-l-statement-1',
        }
       );

    $wrapper->contents_like(
        qr/
          7:\s+\$x\ =\ 3;\n
          8:\s+print\ "3\\n";\n
          9\s*\n
          10:\s+\$x\ =\ 4;\n
          11:\s+print\ "4\\n";\n
          12\s*\n
          13:\s+\$x\ =\ 5;\n
          14:\s+print\ "5\\n";\n
          15\s*\n
          16:\s+\$x\ =\ 6;\n
          /msx,
        "test v with line"
        );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l fact',
                'q',
            ],
            prog => '../lib/perl5db/t/test-l-statement-2',
        }
    );

    my $first_l_out = qr/
        6\s+sub\ fact\ \{\n
        7:\s+my\ \$n\ =\ shift;\n
        8:\s+if\ \(\$n\ >\ 1\)\ \{\n
        9:\s+return\ \$n\ \*\ fact\(\$n\ -\ 1\);
    /msx;

    $wrapper->contents_like(
        qr/
            DB<1>\s+l\ fact\n
            $first_l_out
        /msx,
        'l subroutine_name',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b fact',
                'c',
                # Repeat several times to avoid @typeahead problems.
                '.',
                '.',
                '.',
                '.',
                'q',
            ],
            prog => '../lib/perl5db/t/test-l-statement-2',
        }
    );

    my $line_out = qr /
        ^main::fact\([^\n]*?:7\):\n
        ^7:\s+my\ \$n\ =\ shift;\n
    /msx;

    $wrapper->contents_like(
        qr/
            $line_out
            auto\(-\d+\)\s+DB<\d+>\s+\.\n
            $line_out
        /msx,
        'Test the "." command',
    );
}

# Testing that the f command works.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'f ../lib/perl5db/t/MyModule.pm',
                'b 12',
                'c',
                q/do { use IO::Handle; STDOUT->autoflush(1); print "Var=$var\n"; }/,
                'c',
                'q',
            ],
            include_t => 1,
            prog => '../lib/perl5db/t/filename-line-breakpoint'
        }
    );

    $wrapper->output_like(qr/
        ^Var=Bar$
            .*
        ^In\ MyModule\.$
            .*
        ^In\ Main\ File\.$
            .*
        /msx,
        "f command is working.",
    );
}

# We broke the /pattern/ command because apparently the CORE::eval-s inside
# lib/perl5db.pl cannot handle lexical variable properly. So we now fix this
# bug.
#
# TODO :
#
# 1. Go over the rest of the "eval"s in lib/perl5db.t and see if they cause
# problems.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                '/for/',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->contents_like(
        qr/12: \s* for\ my\ \$q\ \(1\ \.\.\ 10\)\ \{/msx,
        "/pat/ command is working and found a match.",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 22',
                'c',
                '?for?',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->contents_like(
        qr/12: \s* for\ my\ \$q\ \(1\ \.\.\ 10\)\ \{/msx,
        "?pat? command is working and found a match.",
    );
}

# Test the L command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 6',
                'b 13 ($q == 5)',
                'L',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->contents_like(
        qr#
        ^\S*?eval-line-bug:\n
        \s*6:\s*my\ \$i\ =\ 5;\n
        \s*break\ if\ \(1\)\n
        \s*13:\s*\$i\ \+=\ \$q;\n
        \s*break\ if\ \(\(\$q\ ==\ 5\)\)\n
        #msx,
        "L command is listing breakpoints",
    );
}

# Test the L command for watch expressions.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'w (5+6)',
                'L',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->contents_like(
        qr#
        ^Watch-expressions:\n
        \s*\(5\+6\)\n
        #msx,
        "L command is listing watch expressions",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'w (5+6)',
                'w (11*23)',
                'W (5+6)',
                'L',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->contents_like(
        qr#
        ^Watch-expressions:\n
        \s*\(11\*23\)\n
        ^auto\(
        #msx,
        "L command is not listing deleted watch expressions",
    );
}

# Test the L command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 6',
                'a 13 print $i',
                'L',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->contents_like(
        qr#
        ^\S*?eval-line-bug:\n
        \s*6:\s*my\ \$i\ =\ 5;\n
        \s*break\ if\ \(1\)\n
        \s*13:\s*\$i\ \+=\ \$q;\n
        \s*action:\s+print\ \$i\n
        #msx,
        "L command is listing actions and breakpoints",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'S',
                'q',
            ],
            prog =>  '../lib/perl5db/t/rt-104168',
        }
    );

    $wrapper->contents_like(
        qr#
        ^main::bar\n
        main::baz\n
        main::foo\n
        #msx,
        "S command - 1",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'S ^main::ba',
                'q',
            ],
            prog =>  '../lib/perl5db/t/rt-104168',
        }
    );

    $wrapper->contents_like(
        qr#
        ^main::bar\n
        main::baz\n
        auto\(
        #msx,
        "S command with regex",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'S !^main::ba',
                'q',
            ],
            prog =>  '../lib/perl5db/t/rt-104168',
        }
    );

    $wrapper->contents_unlike(
        qr#
        ^main::ba
        #msx,
        "S command with negative regex",
    );

    $wrapper->contents_like(
        qr#
        ^main::foo\n
        #msx,
        "S command with negative regex - what it still matches",
    );
}

# Test the 'a' command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'a 13 print "\nVar<Q>=$q\n"',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    my $nl = $^O eq 'VMS' ? "" : "\\\n";
    $wrapper->output_like(qr#
        \nVar<Q>=1$nl
        \nVar<Q>=2$nl
        \nVar<Q>=3
        #msx,
        "a command is working",
    );
}

# Test the 'a' command with no line number.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                q/a print "Hello " . (3 * 4) . "\n";/,
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/test-a-statement-1',
        }
    );

    $wrapper->output_like(qr#
        (?:^Hello\ 12\n.*?){4}
        #msx,
        "a command with no line number is working",
    );
}

# Test the 'A' command
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'a 13 print "\nVar<Q>=$q\n"',
                'A 13',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->output_like(
        qr#\A\z#msx, # The empty string.
        "A command (for removing actions) is working",
    );
}

# Test the 'A *' command
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'a 6 print "\nFail!\n"',
                'a 13 print "\nVar<Q>=$q\n"',
                'A *',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/eval-line-bug',
        }
    );

    $wrapper->output_like(
        qr#\A\z#msx, # The empty string.
        "'A *' command (for removing all actions) is working",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                'w $foo',
                'c',
                'print "\nIDX=<$idx>\n"',
                'q',
            ],
            prog => '../lib/perl5db/t/test-w-statement-1',
        }
    );


    $wrapper->contents_like(qr#
        \$foo\ changed:\n
        \s+old\ value:\s+'1'\n
        \s+new\ value:\s+'2'\n
        #msx,
        'w command - watchpoint changed',
    );
    $wrapper->output_like(qr#
        \nIDX=<20>\n
        #msx,
        "w command - correct output from IDX",
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                'w $foo',
                'W $foo',
                'c',
                'print "\nIDX=<$idx>\n"',
                'q',
            ],
            prog => '../lib/perl5db/t/test-w-statement-1',
        }
    );

    $wrapper->contents_unlike(qr#
        \$foo\ changed:
        #msx,
        'W command - watchpoint was deleted',
    );

    $wrapper->output_like(qr#
        \nIDX=<>\n
        #msx,
        "W command - stopped at end.",
    );
}

# Test the W * command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'n',
                'w $foo',
                'w ($foo*$foo)',
                'W *',
                'c',
                'print "\nIDX=<$idx>\n"',
                'q',
            ],
            prog => '../lib/perl5db/t/test-w-statement-1',
        }
    );

    $wrapper->contents_unlike(qr#
        \$foo\ changed:
        #msx,
        '"W *" command - watchpoint was deleted',
    );

    $wrapper->output_like(qr#
        \nIDX=<>\n
        #msx,
        '"W *" command - stopped at end.',
    );
}

# Test the 'o' command (without further arguments).
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o',
                'q',
            ],
            prog => '../lib/perl5db/t/test-w-statement-1',
        }
    );

    $wrapper->contents_like(qr#
        ^\s*warnLevel\ =\ '1'\n
        #msx,
        q#"o" command (without arguments) displays warnLevel#,
    );

    $wrapper->contents_like(qr#
        ^\s*signalLevel\ =\ '1'\n
        #msx,
        q#"o" command (without arguments) displays signalLevel#,
    );

    $wrapper->contents_like(qr#
        ^\s*dieLevel\ =\ '1'\n
        #msx,
        q#"o" command (without arguments) displays dieLevel#,
    );

    $wrapper->contents_like(qr#
        ^\s*hashDepth\ =\ 'N/A'\n
        #msx,
        q#"o" command (without arguments) displays hashDepth#,
    );
}

# Test the 'o' query command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o hashDepth? signalLevel?',
                'q',
            ],
            prog => '../lib/perl5db/t/test-w-statement-1',
        }
    );

    $wrapper->contents_unlike(qr#warnLevel#,
        q#"o" query command does not display warnLevel#,
    );

    $wrapper->contents_like(qr#
        ^\s*signalLevel\ =\ '1'\n
        #msx,
        q#"o" query command displays signalLevel#,
    );

    $wrapper->contents_unlike(qr#dieLevel#,
        q#"o" query command does not display dieLevel#,
    );

    $wrapper->contents_like(qr#
        ^\s*hashDepth\ =\ 'N/A'\n
        #msx,
        q#"o" query command displays hashDepth#,
    );
}

# Test the 'o' set command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o signalLevel=0',
                'o',
                'q',
            ],
            prog => '../lib/perl5db/t/test-w-statement-1',
        }
    );

    $wrapper->contents_like(qr/
        ^\s*(signalLevel\ =\ '0'\n)
        .*?
        ^\s*\1
        /msx,
        q#o set command works#,
    );

    $wrapper->contents_like(qr#
        ^\s*hashDepth\ =\ 'N/A'\n
        #msx,
        q#o set command - hashDepth#,
    );
}

# Test the '<' and "< ?" commands.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/< print "\nX=<$x>\n"/,
                q/b 7/,
                q/< ?/,
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr/
        ^pre-perl\ commands:\n
        \s*<\ --\ print\ "\\nX=<\$x>\\n"\n
        /msx,
        q#Test < and < ? commands - contents.#,
    );

    $wrapper->output_like(qr#
        ^X=<FirstVal>\n
        #msx,
        q#Test < and < ? commands - output.#,
    );
}

# Test the '< *' command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/< print "\nX=<$x>\n"/,
                q/b 7/,
                q/< */,
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->output_unlike(qr/FirstVal/,
        q#Test the '< *' command.#,
    );
}

# Test the '>' and "> ?" commands.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/$::foo = 500;/,
                q/> print "\nFOO=<$::foo>\n"/,
                q/b 7/,
                q/> ?/,
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr/
        ^post-perl\ commands:\n
        \s*>\ --\ print\ "\\nFOO=<\$::foo>\\n"\n
        /msx,
        q#Test > and > ? commands - contents.#,
    );

    $wrapper->output_like(qr#
        ^FOO=<500>\n
        #msx,
        q#Test > and > ? commands - output.#,
    );
}

# Test the '> *' command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/> print "\nFOO=<$::foo>\n"/,
                q/b 7/,
                q/> */,
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->output_unlike(qr/FOO=/,
        q#Test the '> *' command.#,
    );
}

# Test the < and > commands together
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/$::lorem = 0;/,
                q/< $::lorem += 10;/,
                q/> print "\nLOREM=<$::lorem>\n"/,
                q/b 7/,
                q/b 5/,
                'c',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->output_like(qr#
        ^LOREM=<10>\n
        #msx,
        q#Test < and > commands. #,
    );
}

# Test the { ? and { [command] commands.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                '{ ?',
                '{ l',
                '{ ?',
                q/b 5/,
                q/c/,
                q/q/,
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^No\ pre-debugger\ actions\.\n
        .*?
        ^pre-debugger\ commands:\n
        \s+\{\ --\ l\n
        .*?
        ^5==>b\s+\$x\ =\ "FirstVal";\n
        6\s*\n
        7:\s+\$dummy\+\+;\n
        8\s*\n
        9:\s+\$x\ =\ "SecondVal";\n

        #msx,
        'Test the pre-prompt debugger commands',
    );
}

# Test the { * command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                '{ q',
                '{ *',
                q/b 5/,
                q/c/,
                q/print (("One" x 5), "\n");/,
                q/q/,
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^All\ \{\ actions\ cleared\.\n
        #msx,
        'Test the { * command',
    );

    $wrapper->output_like(qr/OneOneOneOneOne/,
        '{ * test - output is OK.',
    );
}

# Test the ! command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l 3-5',
                '!',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        (^3:\s+my\ \$dummy\ =\ 0;\n
        4\s*\n
        5:\s+\$x\ =\ "FirstVal";)\n
        .*?
        ^l\ 3-5\n
        \1
        #msx,
        'Test the ! command (along with l 3-5)',
    );
}

# Test the ! -number command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l 3-5',
                'l 2',
                '! -1',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        (^3:\s+my\ \$dummy\ =\ 0;\n
        4\s*\n
        5:\s+\$x\ =\ "FirstVal";)\n
        .*?
        ^2==\>\s+my\ \$x\ =\ "One";\n
        .*?
        ^l\ 3-5\n
        \1
        #msx,
        'Test the ! -n command (along with l)',
    );
}

# Test the 'source' command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'source ../lib/perl5db/t/source-cmd-test.perldb',
                # If we have a 'q' here, then the typeahead will override the
                # input, and so it won't be reached - solution:
                # put a q inside the .perldb commands.
                # ( This may be a bug or a misfeature. )
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^3:\s+my\ \$dummy\ =\ 0;\n
        4\s*\n
        5:\s+\$x\ =\ "FirstVal";\n
        6\s*\n
        7:\s+\$dummy\+\+;\n
        8\s*\n
        9:\s+\$x\ =\ "SecondVal";\n
        10\s*\n
        #msx,
        'Test the source command (along with l)',
    );
}

# Test the 'source' command being traversed from withing typeahead.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'source ../lib/perl5db/t/source-cmd-test-no-q.perldb',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^3:\s+my\ \$dummy\ =\ 0;\n
        4\s*\n
        5:\s+\$x\ =\ "FirstVal";\n
        6\s*\n
        7:\s+\$dummy\+\+;\n
        8\s*\n
        9:\s+\$x\ =\ "SecondVal";\n
        10\s*\n
        #msx,
        'Test the source command inside a typeahead',
    );
}

# Test the 'H -number' command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l 1-10',
                'l 5-10',
                'x "Hello World"',
                'l 1-5',
                'b 3',
                'x (20+4)',
                'H -7',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^\d+:\s+H\ -7\n
        \d+:\s+x\ \(20\+4\)\n
        \d+:\s+b\ 3\n
        \d+:\s+l\ 1-5\n
        \d+:\s+x\ "Hello\ World"\n
        \d+:\s+l\ 5-10\n
        \d+:\s+l\ 1-10\n
        #msx,
        'Test the H -num command',
    );
}

# Add a test for H (without arguments)
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'l 1-10',
                'l 5-10',
                'x "Hello World"',
                'l 1-5',
                'b 3',
                'x (20+4)',
                'H',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^\d+:\s+x\ \(20\+4\)\n
        \d+:\s+b\ 3\n
        \d+:\s+l\ 1-5\n
        \d+:\s+x\ "Hello\ World"\n
        \d+:\s+l\ 5-10\n
        \d+:\s+l\ 1-10\n
        #msx,
        'Test the H command (without a number.)',
    );
}

{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                '= quit q',
                '= foobar l',
                '= .hello print "hellox\n"',
                '= -goodbye print "goodbyex\n"',
                'foobar',
                '.hello',
                '-goodbye',
                'quit',
            ],
            prog => '../lib/perl5db/t/test-l-statement-1',
        }
    );

    $wrapper->contents_like(
        qr/
            ^1==>\s+\$x\ =\ 1;\n
            2:\s+print\ "1\\n";\n
            3\s*\n
            4:\s+\$x\ =\ 2;\n
            5:\s+print\ "2\\n";\n
        /msx,
        'Test the = (command alias) command.',
       );
    $wrapper->output_like(qr/hellox.*goodbyex/xs,
                          "check . and - can start alias name");
}

# Test the m statement.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'm main',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        ^via\ UNIVERSAL:\ DOES$
        #msx,
        "Test m for main - 1",
    );

    $wrapper->contents_like(qr#
        ^via\ UNIVERSAL:\ can$
        #msx,
        "Test m for main - 2",
    );
}

# Test the m statement.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 41',
                'c',
                'm $obj',
                'q',
            ],
            prog => '../lib/perl5db/t/test-m-statement-1',
        }
    );

    $wrapper->contents_like(qr#^greet$#ms,
        "Test m for obj - 1",
    );

    $wrapper->contents_like(qr#^via UNIVERSAL: can$#ms,
        "Test m for obj - 1",
    );
}

# Test the M command.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'M',
                'q',
            ],
            prog => '../lib/perl5db/t/test-m-statement-1',
        }
    );

    $wrapper->contents_like(qr#
        ^'strict\.pm'\ =>\ '\d+\.\d+\ from
        #msx,
        "Test M",
    );

}

# Test the recallCommand option.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o recallCommand=%',
                'l 3-5',
                'l 2',
                '% -1',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr#
        (^3:\s+my\ \$dummy\ =\ 0;\n
        4\s*\n
        5:\s+\$x\ =\ "FirstVal";)\n
        .*?
        ^2==\>\s+my\ \$x\ =\ "One";\n
        .*?
        ^l\ 3-5\n
        \1
        #msx,
        'Test the o recallCommand option',
    );
}

# Test the dieLevel option
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/o dieLevel='1'/,
                q/c/,
                'q',
            ],
            prog => '../lib/perl5db/t/test-dieLevel-option-1',
        }
    );

    $wrapper->output_like(qr#
        ^This\ program\ dies\.\ at\ \S+\ line\ 18\N*\.\n
        .*?
        ^\s+main::baz\(\)\ called\ at\ \S+\ line\ 13\n
        \s+main::bar\(\)\ called\ at\ \S+\ line\ 7\n
        \s+main::foo\(\)\ called\ at\ \S+\ line\ 21\n
        #msx,
        'Test the o dieLevel option',
    );
}

# Test the warnLevel option
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q/o warnLevel='1'/,
                q/c/,
                'q',
            ],
            prog => '../lib/perl5db/t/test-warnLevel-option-1',
        }
    );

    $wrapper->contents_like(qr#
        ^This\ is\ not\ a\ warning\.\ at\ \S+\ line\ 18\N*\.\n
        .*?
        ^\s+main::baz\(\)\ called\ at\ \S+\ line\ 13\n
        \s+main::bar\(\)\ called\ at\ \S+\ line\ 25\n
        \s+main::myfunc\(\)\ called\ at\ \S+\ line\ 28\n
        #msx,
        'Test the o warnLevel option',
    );
}

# Test the t command
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                't',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr/
        ^main::\([^:]+:15\):\n
        15:\s+\$dummy\+\+;\n
        main::\([^:]+:17\):\n
        17:\s+\$x\ =\ "FourthVal";\n
        /msx,
        'Test the t command (without a number.)',
    );
}

# Test the o AutoTrace command
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o AutoTrace',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/disable-breakpoints-1',
        }
    );

    $wrapper->contents_like(qr/
        ^main::\([^:]+:15\):\n
        15:\s+\$dummy\+\+;\n
        main::\([^:]+:17\):\n
        17:\s+\$x\ =\ "FourthVal";\n
        /msx,
        'Test the o AutoTrace command',
    );
}

# Test the t command with function calls
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                't',
                'b 18',
                'c',
                'x ["foo"]',
                'x ["bar"]',
                'q',
            ],
            prog => '../lib/perl5db/t/test-warnLevel-option-1',
        }
    );

    $wrapper->contents_like(qr/
        ^main::\([^:]+:28\):\n
        28:\s+myfunc\(\);\n
        auto\(-\d+\)\s+DB<1>\s+t\n
        Trace\ =\ on\n
        auto\(-\d+\)\s+DB<1>\s+b\ 18\n
        auto\(-\d+\)\s+DB<2>\s+c\n
        main::myfunc\([^:]+:25\):\n
        25:\s+bar\(\);\n
        /msx,
        'Test the t command with function calls.',
    );
}

# Test the o AutoTrace command with function calls
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o AutoTrace',
                'b 18',
                'c',
                'x ["foo"]',
                'x ["bar"]',
                'q',
            ],
            prog => '../lib/perl5db/t/test-warnLevel-option-1',
        }
    );

    $wrapper->contents_like(qr/
        ^main::\([^:]+:28\):\n
        28:\s+myfunc\(\);\n
        auto\(-\d+\)\s+DB<1>\s+o\ AutoTrace\n
        \s+AutoTrace\s+=\s+'1'\n
        auto\(-\d+\)\s+DB<2>\s+b\ 18\n
        auto\(-\d+\)\s+DB<3>\s+c\n
        main::myfunc\([^:]+:25\):\n
        25:\s+bar\(\);\n
        /msx,
        'Test the o AutoTrace command with function calls.',
    );
}

# Test the final message.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/test-warnLevel-option-1',
        }
    );

    $wrapper->contents_like(qr/
        ^Debugged\ program\ terminated\.
        /msx,
        'Test the final "Debugged program terminated" message.',
    );
}

# Test the o inhibit_exit=0 command
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o inhibit_exit=0',
                'n',
                'n',
                'n',
                'n',
                'q',
            ],
            prog => '../lib/perl5db/t/test-warnLevel-option-1',
        }
    );

    $wrapper->contents_unlike(qr/
        ^Debugged\ program\ terminated\.
        /msx,
        'Test the o inhibit_exit=0 command.',
    );
}

# Test the o PrintRet=1 option
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o PrintRet=1',
                'b 29',
                'c',
                q/$x = 's';/,
                'b 10',
                'c',
                'r',
                'q',
            ],
            prog => '../lib/perl5db/t/test-PrintRet-option-1',
        }
    );

    $wrapper->contents_like(
        qr/scalar context return from main::return_scalar: 20024/,
        "Test o PrintRet=1",
    );
}

# Test the o PrintRet=0 option
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o PrintRet=0',
                'b 29',
                'c',
                q/$x = 's';/,
                'b 10',
                'c',
                'r',
                'q',
            ],
            prog => '../lib/perl5db/t/test-PrintRet-option-1',
        }
    );

    $wrapper->contents_unlike(
        qr/scalar context/,
        "Test o PrintRet=0",
    );
}

# Test the o PrintRet=1 option in list context
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o PrintRet=1',
                'b 29',
                'c',
                q/$x = 'l';/,
                'b 17',
                'c',
                'r',
                'q',
            ],
            prog => '../lib/perl5db/t/test-PrintRet-option-1',
        }
    );

    $wrapper->contents_like(
        qr/list context return from main::return_list:\n0\s*'Foo'\n1\s*'Bar'\n2\s*'Baz'\n/,
        "Test o PrintRet=1 in list context",
    );
}

# Test the o PrintRet=0 option in list context
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o PrintRet=0',
                'b 29',
                'c',
                q/$x = 'l';/,
                'b 17',
                'c',
                'r',
                'q',
            ],
            prog => '../lib/perl5db/t/test-PrintRet-option-1',
        }
    );

    $wrapper->contents_unlike(
        qr/list context/,
        "Test o PrintRet=0 in list context",
    );
}

# Test the o PrintRet=1 option in void context
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o PrintRet=1',
                'b 29',
                'c',
                q/$x = 'v';/,
                'b 24',
                'c',
                'r',
                'q',
            ],
            prog => '../lib/perl5db/t/test-PrintRet-option-1',
        }
    );

    $wrapper->contents_like(
        qr/void context return from main::return_void/,
        "Test o PrintRet=1 in void context",
    );
}

# Test the o PrintRet=1 option in void context
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'o PrintRet=0',
                'b 29',
                'c',
                q/$x = 'v';/,
                'b 24',
                'c',
                'r',
                'q',
            ],
            prog => '../lib/perl5db/t/test-PrintRet-option-1',
        }
    );

    $wrapper->contents_unlike(
        qr/void context/,
        "Test o PrintRet=0 in void context",
    );
}

# Test the o frame option.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                # This is to avoid getting the "Debugger program terminated"
                # junk that interferes with the normal output.
                'o inhibit_exit=0',
                'b 10',
                'c',
                'o frame=255',
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/test-frame-option-1',
        }
    );

    $wrapper->contents_like(
        qr/
            in\s*\.=main::my_other_func\(3,\ 1200\)\ from.*?
            out\s*\.=main::my_other_func\(3,\ 1200\)\ from
        /msx,
        "Test o PrintRet=0 in void context",
    );
}

{ # test t expr
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                # This is to avoid getting the "Debugger program terminated"
                # junk that interferes with the normal output.
                'o inhibit_exit=0',
                't fact(3)',
                'q',
            ],
            prog => '../lib/perl5db/t/fact',
        }
    );

    $wrapper->contents_like(
        qr/
	    (?:^main::fact.*return\ \$n\ \*\ fact\(\$n\ -\ 1\);.*)
        /msx,
        "Test t expr",
    );
}

# Test the w for lexical variables expression.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                # This is to avoid getting the "Debugger program terminated"
                # junk that interferes with the normal output.
                'w $exp',
                'n',
                'n',
                'n',
                'n',
                'q',
            ],
            prog => '../lib/perl5db/t/break-on-dot',
        }
    );

    $wrapper->contents_like(
        qr/
\s+old\ value:\s+'1'\n
\s+new\ value:\s+'2'\n
        /msx,
        "Test w for lexical values.",
    );
}

# perl 5 RT #121509 regression bug.
# “perl debugger doesn't save starting dir to restart from”
# Thanks to Linda Walsh for reporting it.
{
    use File::Temp qw/tempdir/;

    my $temp_dir = tempdir( CLEANUP => 1 );

    local $ENV{__PERLDB_TEMP_DIR} = $temp_dir;
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                # This is to avoid getting the "Debugger program terminated"
                # junk that interferes with the normal output.
                'b _after_chdir',
                'c',
                'R',
                'b _finale',
                'c',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'n',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-121509-restart-after-chdir',
        }
    );

    $wrapper->output_like(
        qr/
In\ _finale\ No\ 1
    .*?
In\ _finale\ No\ 2
    .*?
In\ _finale\ No\ 3
        /msx,
        "Test that the debugger chdirs to the initial directory after a restart.",
    );
}
# Test the perldoc command
# We don't actually run the program, but we need to provide one to the wrapper.
SKIP:
{
    $^O eq "linux"
        or skip "man errors aren't especially portable", 1;
    -x '/usr/bin/man'
        or skip "man command seems to be missing", 1;
    local $ENV{LANG} = "C";
    local $ENV{LC_MESSAGES} = "C";
    local $ENV{LC_ALL} = "C";
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'perldoc perlrules',
                'q',
            ],
            prog => '../lib/perl5db/t/fact',
        }
    );

    $wrapper->output_like(
        qr/No (?:manual )?entry for perlrules/,
        'perldoc command works fine',
    );
}

# [perl #71678] debugger bug in evaluation of user actions ('a' command)
# Still evaluated after the script finishes.
{
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                q#a 9 print " \$arg = $arg\n"#,
                'c 9',
                's',
                'q',
            ],
            prog => '../lib/perl5db/t/test-a-statement-2',
            switches => [ '-dw', ],
            stderr => 1,
        }
    );

    $wrapper->contents_unlike(qr/
        Use\ of\ uninitialized\ value\ \$arg\ in\ concatenation\ [\S ]+\ or\ string\ at
        /msx,
        'Test that the a command does not emit warnings on program exit.',
    );
}

{
    # GitHub #17901
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'a 4 $s++',
                ('s') x 5,
                'x $s',
                'q'
            ],
            prog => '../lib/perl5db/t/test-a-statement-3',
            switches => [ '-d' ],
            stderr => 0,
        }
    );
    $wrapper->contents_like(
        qr/^0 +2$/m,
        'Test that the a command runs only on the given lines.',
    );
}

{
    # perl 5 RT #126735 regression bug.
    local $ENV{PERLDB_OPTS} = "NonStop=0 RemotePort=non-existent-host.tld:9001";
    my $output = runperl( stdin => "q\n", stderr => 1, switches => [ '-d' ], prog => '../lib/perl5db/t/fact' );
    like(
        $output,
        qr/^Unable to connect to remote host:/ms,
        'Tried to connect.',
    );
    unlike(
        $output,
        qr/syntax error/,
        'Can quit from the debugger after a wrong RemotePort',
    );
}

{
    # perl 5 RT #120174 - 'p' command
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 2',
                'c',
                'p@abc',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-120174',
        }
    );

    $wrapper->contents_like(
        qr/1234/,
        q/RT 120174: p command can be invoked without space after 'p'/,
    );
}

{
    # perl 5 RT #120174 - 'x' command on array
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 2',
                'c',
                'x@abc',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-120174',
        }
    );

    $wrapper->contents_like(
        qr/0\s+1\n1\s+2\n2\s+3\n3\s+4/ms,
        q/RT 120174: x command can be invoked without space after 'x' before array/,
    );
}

{
    # perl 5 RT #120174 - 'x' command on array ref
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 2',
                'c',
                'x\@abc',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-120174',
        }
    );

    $wrapper->contents_like(
        qr/\s+0\s+1\n\s+1\s+2\n\s+2\s+3\n\s+3\s+4/ms,
        q/RT 120174: x command can be invoked without space after 'x' before array ref/,
    );
}

{
    # perl 5 RT #120174 - 'x' command on hash ref
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 4',
                'c',
                'x\%xyz',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-120174',
        }
    );

    $wrapper->contents_like(
        qr/\s+'alpha'\s+=>\s+'beta'\n\s+'gamma'\s+=>\s+'delta'/ms,
        q/RT 120174: x command can be invoked without space after 'x' before hash ref/,
    );
}

{
    # gh #17660
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b 13',
                'c',
                'i Foo',
                'q',
            ],
            prog => '../lib/perl5db/t/gh-17660',
        }
    );

    $wrapper->output_unlike(
        qr/Undefined subroutine &mro::get_linear_isa/ms,
        q/mro needs to be loaded/,
       );
    $wrapper->output_like(
        qr/Foo 1.000, Bar 2.000/,
        q/check for reasonable result/,
       );
}

{
    # gh #17661
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c',
                'i $obj',
                'q',
            ],
            prog => '../lib/perl5db/t/gh-17661',
        }
    );

    $wrapper->output_like(
        qr/C5, C1, C2, C3, C4/,
        q/check for reasonable result/,
       );
}

{
    # gh #17661 related - C<l $var> where $var is lexical
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c',
                'l $x',
                'l $y',
                'q',
            ],
            prog => '../lib/perl5db/t/gh-17661b',
        }
    );

    $wrapper->contents_like(
        qr/sub bar/,
        q/check bar was listed/,
       );
    $wrapper->contents_like(
        qr/sub foo/,
        q/check foo was listed/,
       );
}

SKIP:
{
    $Config{usethreads}
      or skip "need threads to test debugging threads", 1;
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-124203',
        }
    );

    $wrapper->output_like(qr/In the thread/, "[perl #124203] the thread ran");

    $wrapper->output_like(qr/Finished/, "[perl #124203] debugger didn't deadlock");

    $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'c',
                'q',
            ],
            prog => '../lib/perl5db/t/rt-124203b',
        }
    );

    $wrapper->output_like(qr/In the thread/, "[perl #124203] the thread ran (lvalue)");

    $wrapper->output_like(qr/Finished One/, "[perl #124203] debugger didn't deadlock (lvalue)");
}

{
    # https://github.com/Perl/perl5/issues/19198
    # this isn't a debugger bug, but a bug in the way perl itself stores cop
    # information for lines
    my $wrapper = DebugWrap->new(
        {
            cmds =>
            [
                'b Test::AUTOLOAD', # this would crash on ASAN
                'c', # this would fail to stop at the breakpoint
                'q'
            ],
            prog => \<<'EOS',
package Test;

sub AUTOLOAD {
    use vars '$AUTOLOAD';
    my $sub = $AUTOLOAD;
    return 1;
}

package main;


sub test
{
    Test::test();
}

sub test_test
{
    eval { test() };
}

test_test();
EOS
           }
    );
    $wrapper->output_unlike(qr/AddressSanitizer/, "[github #19198] no bad access");
    $wrapper->contents_like(qr/^Test::AUTOLOAD\(.*?\):\s+\d+:\s+my \$sub = \$AUTOLOAD;/m,
                          "[github #19198] check we stopped correctly");
}

done_testing();

END {
    1 while unlink ($rc_filename, $out_fn);
}
