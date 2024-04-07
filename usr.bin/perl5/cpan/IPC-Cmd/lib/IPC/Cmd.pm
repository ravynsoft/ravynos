package IPC::Cmd;

use strict;

BEGIN {

    use constant IS_VMS         => $^O eq 'VMS'                       ? 1 : 0;
    use constant IS_WIN32       => $^O eq 'MSWin32'                   ? 1 : 0;
    use constant IS_HPUX        => $^O eq 'hpux'                      ? 1 : 0;
    use constant IS_WIN98       => (IS_WIN32 and !Win32::IsWinNT())   ? 1 : 0;
    use constant ALARM_CLASS    => __PACKAGE__ . '::TimeOut';
    use constant SPECIAL_CHARS  => qw[< > | &];
    use constant QUOTE          => do { IS_WIN32 ? q["] : q['] };

    use Exporter    ();
    use vars        qw[ @ISA $VERSION @EXPORT_OK $VERBOSE $DEBUG
                        $USE_IPC_RUN $USE_IPC_OPEN3 $CAN_USE_RUN_FORKED $WARN
                        $INSTANCES $ALLOW_NULL_ARGS
                        $HAVE_MONOTONIC
                    ];

    $VERSION        = '1.04';
    $VERBOSE        = 0;
    $DEBUG          = 0;
    $WARN           = 1;
    $USE_IPC_RUN    = IS_WIN32 && !IS_WIN98;
    $USE_IPC_OPEN3  = not IS_VMS;
    $ALLOW_NULL_ARGS = 0;

    $CAN_USE_RUN_FORKED = 0;
    eval {
        require POSIX; POSIX->import();
        require IPC::Open3; IPC::Open3->import();
        require IO::Select; IO::Select->import();
        require IO::Handle; IO::Handle->import();
        require FileHandle; FileHandle->import();
        require Socket;
        require Time::HiRes; Time::HiRes->import();
        require Win32 if IS_WIN32;
    };
    $CAN_USE_RUN_FORKED = $@ || !IS_VMS && !IS_WIN32;

    eval {
        my $wait_start_time = Time::HiRes::clock_gettime(&Time::HiRes::CLOCK_MONOTONIC);
    };
    if ($@) {
        $HAVE_MONOTONIC = 0;
    }
    else {
        $HAVE_MONOTONIC = 1;
    }

    @ISA            = qw[Exporter];
    @EXPORT_OK      = qw[can_run run run_forked QUOTE];
}

require Carp;
use File::Spec;
use Params::Check               qw[check];
use Text::ParseWords            ();             # import ONLY if needed!
use Module::Load::Conditional   qw[can_load];
use Locale::Maketext::Simple    Style => 'gettext';

local $Module::Load::Conditional::FORCE_SAFE_INC = 1;

=pod

=head1 NAME

IPC::Cmd - finding and running system commands made easy

=head1 SYNOPSIS

    use IPC::Cmd qw[can_run run run_forked];

    my $full_path = can_run('wget') or warn 'wget is not installed!';

    ### commands can be arrayrefs or strings ###
    my $cmd = "$full_path -b theregister.co.uk";
    my $cmd = [$full_path, '-b', 'theregister.co.uk'];

    ### in scalar context ###
    my $buffer;
    if( scalar run( command => $cmd,
                    verbose => 0,
                    buffer  => \$buffer,
                    timeout => 20 )
    ) {
        print "fetched webpage successfully: $buffer\n";
    }


    ### in list context ###
    my( $success, $error_message, $full_buf, $stdout_buf, $stderr_buf ) =
            run( command => $cmd, verbose => 0 );

    if( $success ) {
        print "this is what the command printed:\n";
        print join "", @$full_buf;
    }

    ### run_forked example ###
    my $result = run_forked("$full_path -q -O - theregister.co.uk", {'timeout' => 20});
    if ($result->{'exit_code'} eq 0 && !$result->{'timeout'}) {
        print "this is what wget returned:\n";
        print $result->{'stdout'};
    }

    ### check for features
    print "IPC::Open3 available: "  . IPC::Cmd->can_use_ipc_open3;
    print "IPC::Run available: "    . IPC::Cmd->can_use_ipc_run;
    print "Can capture buffer: "    . IPC::Cmd->can_capture_buffer;

    ### don't have IPC::Cmd be verbose, ie don't print to stdout or
    ### stderr when running commands -- default is '0'
    $IPC::Cmd::VERBOSE = 0;


=head1 DESCRIPTION

IPC::Cmd allows you to run commands platform independently,
interactively if desired, but have them still work.

The C<can_run> function can tell you if a certain binary is installed
and if so where, whereas the C<run> function can actually execute any
of the commands you give it and give you a clear return value, as well
as adhere to your verbosity settings.

=head1 CLASS METHODS

=head2 $ipc_run_version = IPC::Cmd->can_use_ipc_run( [VERBOSE] )

Utility function that tells you if C<IPC::Run> is available.
If the C<verbose> flag is passed, it will print diagnostic messages
if L<IPC::Run> can not be found or loaded.

=cut


sub can_use_ipc_run     {
    my $self    = shift;
    my $verbose = shift || 0;

    ### IPC::Run doesn't run on win98
    return if IS_WIN98;

    ### if we don't have ipc::run, we obviously can't use it.
    return unless can_load(
                        modules => { 'IPC::Run' => '0.55' },
                        verbose => ($WARN && $verbose),
                    );

    ### otherwise, we're good to go
    return $IPC::Run::VERSION;
}

=head2 $ipc_open3_version = IPC::Cmd->can_use_ipc_open3( [VERBOSE] )

Utility function that tells you if C<IPC::Open3> is available.
If the verbose flag is passed, it will print diagnostic messages
if C<IPC::Open3> can not be found or loaded.

=cut


sub can_use_ipc_open3   {
    my $self    = shift;
    my $verbose = shift || 0;

    ### IPC::Open3 is not working on VMS because of a lack of fork.
    return if IS_VMS;

    ### IPC::Open3 works on every non-VMS platform, but it can't
    ### capture buffers on win32 :(
    return unless can_load(
        modules => { map {$_ => '0.0'} qw|IPC::Open3 IO::Select Symbol| },
        verbose => ($WARN && $verbose),
    );

    return $IPC::Open3::VERSION;
}

=head2 $bool = IPC::Cmd->can_capture_buffer

Utility function that tells you if C<IPC::Cmd> is capable of
capturing buffers in it's current configuration.

=cut

sub can_capture_buffer {
    my $self    = shift;

    return 1 if $USE_IPC_RUN    && $self->can_use_ipc_run;
    return 1 if $USE_IPC_OPEN3  && $self->can_use_ipc_open3;
    return;
}

=head2 $bool = IPC::Cmd->can_use_run_forked

Utility function that tells you if C<IPC::Cmd> is capable of
providing C<run_forked> on the current platform.

=head1 FUNCTIONS

=head2 $path = can_run( PROGRAM );

C<can_run> takes only one argument: the name of a binary you wish
to locate. C<can_run> works much like the unix binary C<which> or the bash
command C<type>, which scans through your path, looking for the requested
binary.

Unlike C<which> and C<type>, this function is platform independent and
will also work on, for example, Win32.

If called in a scalar context it will return the full path to the binary
you asked for if it was found, or C<undef> if it was not.

If called in a list context and the global variable C<$INSTANCES> is a true
value, it will return a list of the full paths to instances
of the binary where found in C<PATH>, or an empty list if it was not found.

=cut

sub can_run {
    my $command = shift;

    # a lot of VMS executables have a symbol defined
    # check those first
    if ( $^O eq 'VMS' ) {
        require VMS::DCLsym;
        my $syms = VMS::DCLsym->new;
        return $command if scalar $syms->getsym( uc $command );
    }

    require File::Spec;
    require ExtUtils::MakeMaker;

    my @possibles;

    if( File::Spec->file_name_is_absolute($command) ) {
        return MM->maybe_command($command);

    } else {
        for my $dir (
            File::Spec->path,
            ( IS_WIN32 ? File::Spec->curdir : () )
        ) {
            next if ! $dir || ! -d $dir;
            my $abs = File::Spec->catfile( IS_WIN32 ? Win32::GetShortPathName( $dir ) : $dir, $command);
            push @possibles, $abs if $abs = MM->maybe_command($abs);
        }
    }
    return @possibles if wantarray and $INSTANCES;
    return shift @possibles;
}

=head2 $ok | ($ok, $err, $full_buf, $stdout_buff, $stderr_buff) = run( command => COMMAND, [verbose => BOOL, buffer => \$SCALAR, timeout => DIGIT] );

C<run> takes 4 arguments:

=over 4

=item command

This is the command to execute. It may be either a string or an array
reference.
This is a required argument.

See L<"Caveats"> for remarks on how commands are parsed and their
limitations.

=item verbose

This controls whether all output of a command should also be printed
to STDOUT/STDERR or should only be trapped in buffers (NOTE: buffers
require L<IPC::Run> to be installed, or your system able to work with
L<IPC::Open3>).

It will default to the global setting of C<$IPC::Cmd::VERBOSE>,
which by default is 0.

=item buffer

This will hold all the output of a command. It needs to be a reference
to a scalar.
Note that this will hold both the STDOUT and STDERR messages, and you
have no way of telling which is which.
If you require this distinction, run the C<run> command in list context
and inspect the individual buffers.

Of course, this requires that the underlying call supports buffers. See
the note on buffers above.

=item timeout

Sets the maximum time the command is allowed to run before aborting,
using the built-in C<alarm()> call. If the timeout is triggered, the
C<errorcode> in the return value will be set to an object of the
C<IPC::Cmd::TimeOut> class. See the L<"error message"> section below for
details.

Defaults to C<0>, meaning no timeout is set.

=back

C<run> will return a simple C<true> or C<false> when called in scalar
context.
In list context, you will be returned a list of the following items:

=over 4

=item success

A simple boolean indicating if the command executed without errors or
not.

=item error message

If the first element of the return value (C<success>) was 0, then some
error occurred. This second element is the error message the command
you requested exited with, if available. This is generally a pretty
printed value of C<$?> or C<$@>. See C<perldoc perlvar> for details on
what they can contain.
If the error was a timeout, the C<error message> will be prefixed with
the string C<IPC::Cmd::TimeOut>, the timeout class.

=item full_buffer

This is an array reference containing all the output the command
generated.
Note that buffers are only available if you have L<IPC::Run> installed,
or if your system is able to work with L<IPC::Open3> -- see below).
Otherwise, this element will be C<undef>.

=item out_buffer

This is an array reference containing all the output sent to STDOUT the
command generated. The notes from L<"full_buffer"> apply.

=item error_buffer

This is an arrayreference containing all the output sent to STDERR the
command generated. The notes from L<"full_buffer"> apply.


=back

See the L<"HOW IT WORKS"> section below to see how C<IPC::Cmd> decides
what modules or function calls to use when issuing a command.

=cut

{   my @acc = qw[ok error _fds];

    ### autogenerate accessors ###
    for my $key ( @acc ) {
        no strict 'refs';
        *{__PACKAGE__."::$key"} = sub {
            $_[0]->{$key} = $_[1] if @_ > 1;
            return $_[0]->{$key};
        }
    }
}

sub can_use_run_forked {
    return $CAN_USE_RUN_FORKED eq "1";
}

sub get_monotonic_time {
    if ($HAVE_MONOTONIC) {
        return Time::HiRes::clock_gettime(&Time::HiRes::CLOCK_MONOTONIC);
    }
    else {
        return time();
    }
}

sub adjust_monotonic_start_time {
    my ($ref_vars, $now, $previous) = @_;

    # workaround only for those systems which don't have
    # Time::HiRes::CLOCK_MONOTONIC (Mac OSX in particular)
    return if $HAVE_MONOTONIC;

    # don't have previous monotonic value (only happens once
    # in the beginning of the program execution)
    return unless $previous;

    my $time_diff = $now - $previous;

    # adjust previously saved time with the skew value which is
    # either negative when clock moved back or more than 5 seconds --
    # assuming that event loop does happen more often than once
    # per five seconds, which might not be always true (!) but
    # hopefully that's ok, because it's just a workaround
    if ($time_diff > 5 || $time_diff < 0) {
        foreach my $ref_var (@{$ref_vars}) {
            if (defined($$ref_var)) {
                $$ref_var = $$ref_var + $time_diff;
            }
        }
    }
}

sub uninstall_signals {
		return unless defined($IPC::Cmd::{'__old_signals'});

		foreach my $sig_name (keys %{$IPC::Cmd::{'__old_signals'}}) {
				$SIG{$sig_name} = $IPC::Cmd::{'__old_signals'}->{$sig_name};
		}
}

# incompatible with POSIX::SigAction
#
sub install_layered_signal {
  my ($s, $handler_code) = @_;

  my %available_signals = map {$_ => 1} keys %SIG;

  Carp::confess("install_layered_signal got nonexistent signal name [$s]")
    unless defined($available_signals{$s});
  Carp::confess("install_layered_signal expects coderef")
    if !ref($handler_code) || ref($handler_code) ne 'CODE';

  $IPC::Cmd::{'__old_signals'} = {}
  		unless defined($IPC::Cmd::{'__old_signals'});
	$IPC::Cmd::{'__old_signals'}->{$s} = $SIG{$s};

  my $previous_handler = $SIG{$s};

  my $sig_handler = sub {
    my ($called_sig_name, @sig_param) = @_;

    # $s is a closure referring to real signal name
    # for which this handler is being installed.
    # it is used to distinguish between
    # real signal handlers and aliased signal handlers
    my $signal_name = $s;

    # $called_sig_name is a signal name which
    # was passed to this signal handler;
    # it doesn't equal $signal_name in case
    # some signal handlers in %SIG point
    # to other signal handler (CHLD and CLD,
    # ABRT and IOT)
    #
    # initial signal handler for aliased signal
    # calls some other signal handler which
    # should not execute the same handler_code again
    if ($called_sig_name eq $signal_name) {
      $handler_code->($signal_name);
    }

    # run original signal handler if any (including aliased)
    #
    if (ref($previous_handler)) {
      $previous_handler->($called_sig_name, @sig_param);
    }
  };

  $SIG{$s} = $sig_handler;
}

# give process a chance sending TERM,
# waiting for a while (2 seconds)
# and killing it with KILL
sub kill_gently {
  my ($pid, $opts) = @_;

  require POSIX;

  $opts = {} unless $opts;
  $opts->{'wait_time'} = 2 unless defined($opts->{'wait_time'});
  $opts->{'first_kill_type'} = 'just_process' unless $opts->{'first_kill_type'};
  $opts->{'final_kill_type'} = 'just_process' unless $opts->{'final_kill_type'};

  if ($opts->{'first_kill_type'} eq 'just_process') {
    kill(15, $pid);
  }
  elsif ($opts->{'first_kill_type'} eq 'process_group') {
    kill(-15, $pid);
  }

  my $do_wait = 1;
  my $child_finished = 0;

  my $wait_start_time = get_monotonic_time();
  my $now;
  my $previous_monotonic_value;

  while ($do_wait) {
    $previous_monotonic_value = $now;
    $now = get_monotonic_time();

    adjust_monotonic_start_time([\$wait_start_time], $now, $previous_monotonic_value);

    if ($now > $wait_start_time + $opts->{'wait_time'}) {
        $do_wait = 0;
        next;
    }

    my $waitpid = waitpid($pid, POSIX::WNOHANG);

    if ($waitpid eq -1) {
        $child_finished = 1;
        $do_wait = 0;
        next;
    }

    Time::HiRes::usleep(250000); # quarter of a second
  }

  if (!$child_finished) {
    if ($opts->{'final_kill_type'} eq 'just_process') {
      kill(9, $pid);
    }
    elsif ($opts->{'final_kill_type'} eq 'process_group') {
      kill(-9, $pid);
    }
  }
}

sub open3_run {
    my ($cmd, $opts) = @_;

    $opts = {} unless $opts;

    my $child_in = FileHandle->new;
    my $child_out = FileHandle->new;
    my $child_err = FileHandle->new;
    $child_out->autoflush(1);
    $child_err->autoflush(1);

    my $pid = open3($child_in, $child_out, $child_err, $cmd);
    Time::HiRes::usleep(1) if IS_HPUX;

    # will consider myself orphan if my ppid changes
    # from this one:
    my $original_ppid = $opts->{'original_ppid'};

    # push my child's pid to our parent
    # so in case i am killed parent
    # could stop my child (search for
    # child_child_pid in parent code)
    if ($opts->{'parent_info'}) {
      my $ps = $opts->{'parent_info'};
      print $ps "spawned $pid\n";
    }

    if ($child_in && $child_out->opened && $opts->{'child_stdin'}) {
        # If the child process dies for any reason,
        # the next write to CHLD_IN is likely to generate
        # a SIGPIPE in the parent, which is fatal by default.
        # So you may wish to handle this signal.
        #
        # from http://perldoc.perl.org/IPC/Open3.html,
        # absolutely needed to catch piped commands errors.
        #
        local $SIG{'PIPE'} = sub { 1; };

        print $child_in $opts->{'child_stdin'};
    }
    close($child_in);

    my $child_output = {
        'out' => $child_out->fileno,
        'err' => $child_err->fileno,
        $child_out->fileno => {
            'parent_socket' => $opts->{'parent_stdout'},
            'scalar_buffer' => "",
            'child_handle' => $child_out,
            'block_size' => ($child_out->stat)[11] || 1024,
          },
        $child_err->fileno => {
            'parent_socket' => $opts->{'parent_stderr'},
            'scalar_buffer' => "",
            'child_handle' => $child_err,
            'block_size' => ($child_err->stat)[11] || 1024,
          },
        };

    my $select = IO::Select->new();
    $select->add($child_out, $child_err);

    # pass any signal to the child
    # effectively creating process
    # strongly attached to the child:
    # it will terminate only after child
    # has terminated (except for SIGKILL,
    # which is specially handled)
    SIGNAL: foreach my $s (keys %SIG) {
        next SIGNAL if $s eq '__WARN__' or $s eq '__DIE__'; # Skip and don't clobber __DIE__ & __WARN__
        my $sig_handler;
        $sig_handler = sub {
            kill("$s", $pid);
            $SIG{$s} = $sig_handler;
        };
        $SIG{$s} = $sig_handler;
    }

    my $child_finished = 0;

    my $real_exit;
    my $exit_value;

    while(!$child_finished) {

        # parent was killed otherwise we would have got
        # the same signal as parent and process it same way
        if (getppid() != $original_ppid) {

          # end my process group with all the children
          # (i am the process group leader, so my pid
          # equals to the process group id)
          #
          # same thing which is done
          # with $opts->{'clean_up_children'}
          # in run_forked
          #
          kill(-9, $$);

          POSIX::_exit 1;
        }

        my $waitpid = waitpid($pid, POSIX::WNOHANG);

        # child finished, catch it's exit status
        if ($waitpid ne 0 && $waitpid ne -1) {
          $real_exit = $?;
          $exit_value = $? >> 8;
        }

        if ($waitpid eq -1) {
          $child_finished = 1;
        }


        my $ready_fds = [];
        push @{$ready_fds}, $select->can_read(1/100);

        READY_FDS: while (scalar(@{$ready_fds})) {
            my $fd = shift @{$ready_fds};
            $ready_fds = [grep {$_ ne $fd} @{$ready_fds}];

            my $str = $child_output->{$fd->fileno};
            Carp::confess("child stream not found: $fd") unless $str;

            my $data;
            my $count = $fd->sysread($data, $str->{'block_size'});

            if ($count) {
                if ($str->{'parent_socket'}) {
                    my $ph = $str->{'parent_socket'};
                    print $ph $data;
                }
                else {
                    $str->{'scalar_buffer'} .= $data;
                }
            }
            elsif ($count eq 0) {
                $select->remove($fd);
                $fd->close();
            }
            else {
                Carp::confess("error during sysread: " . $!);
            }

            push @{$ready_fds}, $select->can_read(1/100) if $child_finished;
        }

        Time::HiRes::usleep(1);
    }

    # since we've successfully reaped the child,
    # let our parent know about this.
    #
    if ($opts->{'parent_info'}) {
        my $ps = $opts->{'parent_info'};

        # child was killed, inform parent
        if ($real_exit & 127) {
          print $ps "$pid killed with " . ($real_exit & 127) . "\n";
        }

        print $ps "reaped $pid\n";
    }

    if ($opts->{'parent_stdout'} || $opts->{'parent_stderr'}) {
        return $exit_value;
    }
    else {
        return {
            'stdout' => $child_output->{$child_output->{'out'}}->{'scalar_buffer'},
            'stderr' => $child_output->{$child_output->{'err'}}->{'scalar_buffer'},
            'exit_code' => $exit_value,
            };
    }
}

=head2 $hashref = run_forked( COMMAND, { child_stdin => SCALAR, timeout => DIGIT, stdout_handler => CODEREF, stderr_handler => CODEREF} );

C<run_forked> is used to execute some program or a coderef,
optionally feed it with some input, get its return code
and output (both stdout and stderr into separate buffers).
In addition, it allows to terminate the program
if it takes too long to finish.

The important and distinguishing feature of run_forked
is execution timeout which at first seems to be
quite a simple task but if you think
that the program which you're spawning
might spawn some children itself (which
in their turn could do the same and so on)
it turns out to be not a simple issue.

C<run_forked> is designed to survive and
successfully terminate almost any long running task,
even a fork bomb in case your system has the resources
to survive during given timeout.

This is achieved by creating separate watchdog process
which spawns the specified program in a separate
process session and supervises it: optionally
feeds it with input, stores its exit code,
stdout and stderr, terminates it in case
it runs longer than specified.

Invocation requires the command to be executed or a coderef and optionally a hashref of options:

=over

=item C<timeout>

Specify in seconds how long to run the command before it is killed with SIG_KILL (9),
which effectively terminates it and all of its children (direct or indirect).

=item C<child_stdin>

Specify some text that will be passed into the C<STDIN> of the executed program.

=item C<stdout_handler>

Coderef of a subroutine to call when a portion of data is received on
STDOUT from the executing program.

=item C<stderr_handler>

Coderef of a subroutine to call when a portion of data is received on
STDERR from the executing program.

=item C<wait_loop_callback>

Coderef of a subroutine to call inside of the main waiting loop
(while C<run_forked> waits for the external to finish or fail).
It is useful to stop running external process before it ends
by itself, e.g.

  my $r = run_forked("some external command", {
	  'wait_loop_callback' => sub {
          if (condition) {
              kill(1, $$);
          }
	  },
	  'terminate_on_signal' => 'HUP',
	  });

Combined with C<stdout_handler> and C<stderr_handler> allows terminating
external command based on its output. Could also be used as a timer
without engaging with L<alarm> (signals).

Remember that this code could be called every millisecond (depending
on the output which external command generates), so try to make it
as lightweight as possible.

=item C<discard_output>

Discards the buffering of the standard output and standard errors for return by run_forked().
With this option you have to use the std*_handlers to read what the command outputs.
Useful for commands that send a lot of output.

=item C<terminate_on_parent_sudden_death>

Enable this option if you wish all spawned processes to be killed if the initially spawned
process (the parent) is killed or dies without waiting for child processes.

=back

C<run_forked> will return a HASHREF with the following keys:

=over

=item C<exit_code>

The exit code of the executed program.

=item C<timeout>

The number of seconds the program ran for before being terminated, or 0 if no timeout occurred.

=item C<stdout>

Holds the standard output of the executed command (or empty string if
there was no STDOUT output or if C<discard_output> was used; it's always defined!)

=item C<stderr>

Holds the standard error of the executed command (or empty string if
there was no STDERR output or if C<discard_output> was used; it's always defined!)

=item C<merged>

Holds the standard output and error of the executed command merged into one stream
(or empty string if there was no output at all or if C<discard_output> was used; it's always defined!)

=item C<err_msg>

Holds some explanation in the case of an error.

=back

=cut

sub run_forked {
    ### container to store things in
    my $self = bless {}, __PACKAGE__;

    if (!can_use_run_forked()) {
        Carp::carp("run_forked is not available: $CAN_USE_RUN_FORKED");
        return;
    }

    require POSIX;

    my ($cmd, $opts) = @_;
    if (ref($cmd) eq 'ARRAY') {
        $cmd = join(" ", @{$cmd});
    }

    if (!$cmd) {
        Carp::carp("run_forked expects command to run");
        return;
    }

    $opts = {} unless $opts;
    $opts->{'timeout'} = 0 unless $opts->{'timeout'};
    $opts->{'terminate_wait_time'} = 2 unless defined($opts->{'terminate_wait_time'});

    # turned on by default
    $opts->{'clean_up_children'} = 1 unless defined($opts->{'clean_up_children'});

    # sockets to pass child stdout to parent
    my $child_stdout_socket;
    my $parent_stdout_socket;

    # sockets to pass child stderr to parent
    my $child_stderr_socket;
    my $parent_stderr_socket;

    # sockets for child -> parent internal communication
    my $child_info_socket;
    my $parent_info_socket;

    socketpair($child_stdout_socket, $parent_stdout_socket, &Socket::AF_UNIX, &Socket::SOCK_STREAM, &Socket::PF_UNSPEC) ||
      Carp::confess ("socketpair: $!");
    socketpair($child_stderr_socket, $parent_stderr_socket, &Socket::AF_UNIX, &Socket::SOCK_STREAM, &Socket::PF_UNSPEC) ||
      Carp::confess ("socketpair: $!");
    socketpair($child_info_socket, $parent_info_socket, &Socket::AF_UNIX, &Socket::SOCK_STREAM, &Socket::PF_UNSPEC) ||
      Carp::confess ("socketpair: $!");

    $child_stdout_socket->autoflush(1);
    $parent_stdout_socket->autoflush(1);
    $child_stderr_socket->autoflush(1);
    $parent_stderr_socket->autoflush(1);
    $child_info_socket->autoflush(1);
    $parent_info_socket->autoflush(1);

    my $start_time = get_monotonic_time();

    my $pid;
    my $ppid = $$;
    if ($pid = fork) {

      # we are a parent
      close($parent_stdout_socket);
      close($parent_stderr_socket);
      close($parent_info_socket);

      my $flags;

      # prepare sockets to read from child

      $flags = fcntl($child_stdout_socket, POSIX::F_GETFL, 0) || Carp::confess "can't fnctl F_GETFL: $!";
      $flags |= POSIX::O_NONBLOCK;
      fcntl($child_stdout_socket, POSIX::F_SETFL, $flags) || Carp::confess "can't fnctl F_SETFL: $!";

      $flags = fcntl($child_stderr_socket, POSIX::F_GETFL, 0) || Carp::confess "can't fnctl F_GETFL: $!";
      $flags |= POSIX::O_NONBLOCK;
      fcntl($child_stderr_socket, POSIX::F_SETFL, $flags) || Carp::confess "can't fnctl F_SETFL: $!";

      $flags = fcntl($child_info_socket, POSIX::F_GETFL, 0) || Carp::confess "can't fnctl F_GETFL: $!";
      $flags |= POSIX::O_NONBLOCK;
      fcntl($child_info_socket, POSIX::F_SETFL, $flags) || Carp::confess "can't fnctl F_SETFL: $!";

  #    print "child $pid started\n";

      my $child_output = {
        $child_stdout_socket->fileno => {
          'scalar_buffer' => "",
          'child_handle' => $child_stdout_socket,
          'block_size' => ($child_stdout_socket->stat)[11] || 1024,
          'protocol' => 'stdout',
          },
        $child_stderr_socket->fileno => {
          'scalar_buffer' => "",
          'child_handle' => $child_stderr_socket,
          'block_size' => ($child_stderr_socket->stat)[11] || 1024,
          'protocol' => 'stderr',
          },
        $child_info_socket->fileno => {
          'scalar_buffer' => "",
          'child_handle' => $child_info_socket,
          'block_size' => ($child_info_socket->stat)[11] || 1024,
          'protocol' => 'info',
          },
        };

      my $select = IO::Select->new();
      $select->add($child_stdout_socket, $child_stderr_socket, $child_info_socket);

      my $child_timedout = 0;
      my $child_finished = 0;
      my $child_stdout = '';
      my $child_stderr = '';
      my $child_merged = '';
      my $child_exit_code = 0;
      my $child_killed_by_signal = 0;
      my $parent_died = 0;

      my $last_parent_check = 0;
      my $got_sig_child = 0;
      my $got_sig_quit = 0;
      my $orig_sig_child = $SIG{'CHLD'};

      $SIG{'CHLD'} = sub { $got_sig_child = get_monotonic_time(); };

      if ($opts->{'terminate_on_signal'}) {
        install_layered_signal($opts->{'terminate_on_signal'}, sub { $got_sig_quit = time(); });
      }

      my $child_child_pid;
      my $now;
      my $previous_monotonic_value;

      while (!$child_finished) {
        $previous_monotonic_value = $now;
        $now = get_monotonic_time();

        adjust_monotonic_start_time([\$start_time, \$last_parent_check, \$got_sig_child], $now, $previous_monotonic_value);

        if ($opts->{'terminate_on_parent_sudden_death'}) {
          # check for parent once each five seconds
          if ($now > $last_parent_check + 5) {
            if (getppid() eq "1") {
              kill_gently ($pid, {
                'first_kill_type' => 'process_group',
                'final_kill_type' => 'process_group',
                'wait_time' => $opts->{'terminate_wait_time'}
                });
              $parent_died = 1;
            }

            $last_parent_check = $now;
          }
        }

        # user specified timeout
        if ($opts->{'timeout'}) {
          if ($now > $start_time + $opts->{'timeout'}) {
            kill_gently ($pid, {
              'first_kill_type' => 'process_group',
              'final_kill_type' => 'process_group',
              'wait_time' => $opts->{'terminate_wait_time'}
              });
            $child_timedout = 1;
          }
        }

        # give OS 10 seconds for correct return of waitpid,
        # kill process after that and finish wait loop;
        # shouldn't ever happen -- remove this code?
        if ($got_sig_child) {
          if ($now > $got_sig_child + 10) {
            print STDERR "waitpid did not return -1 for 10 seconds after SIG_CHLD, killing [$pid]\n";
            kill (-9, $pid);
            $child_finished = 1;
          }
        }

        if ($got_sig_quit) {
          kill_gently ($pid, {
            'first_kill_type' => 'process_group',
            'final_kill_type' => 'process_group',
            'wait_time' => $opts->{'terminate_wait_time'}
            });
          $child_finished = 1;
        }

        my $waitpid = waitpid($pid, POSIX::WNOHANG);

        # child finished, catch it's exit status
        if ($waitpid ne 0 && $waitpid ne -1) {
          $child_exit_code = $? >> 8;
        }

        if ($waitpid eq -1) {
          $child_finished = 1;
        }

        my $ready_fds = [];
        push @{$ready_fds}, $select->can_read(1/100);

        READY_FDS: while (scalar(@{$ready_fds})) {
          my $fd = shift @{$ready_fds};
          $ready_fds = [grep {$_ ne $fd} @{$ready_fds}];

          my $str = $child_output->{$fd->fileno};
          Carp::confess("child stream not found: $fd") unless $str;

          my $data = "";
          my $count = $fd->sysread($data, $str->{'block_size'});

          if ($count) {
              # extract all the available lines and store the rest in temporary buffer
              if ($data =~ /(.+\n)([^\n]*)/so) {
                  $data = $str->{'scalar_buffer'} . $1;
                  $str->{'scalar_buffer'} = $2 || "";
              }
              else {
                  $str->{'scalar_buffer'} .= $data;
                  $data = "";
              }
          }
          elsif ($count eq 0) {
            $select->remove($fd);
            $fd->close();
            if ($str->{'scalar_buffer'}) {
                $data = $str->{'scalar_buffer'} . "\n";
            }
          }
          else {
            Carp::confess("error during sysread on [$fd]: " . $!);
          }

          # $data contains only full lines (or last line if it was unfinished read
          # or now new-line in the output of the child); dat is processed
          # according to the "protocol" of socket
          if ($str->{'protocol'} eq 'info') {
            if ($data =~ /^spawned ([0-9]+?)\n(.*?)/so) {
              $child_child_pid = $1;
              $data = $2;
            }
            if ($data =~ /^reaped ([0-9]+?)\n(.*?)/so) {
              $child_child_pid = undef;
              $data = $2;
            }
            if ($data =~ /^[\d]+ killed with ([0-9]+?)\n(.*?)/so) {
              $child_killed_by_signal = $1;
              $data = $2;
            }

            # we don't expect any other data in info socket, so it's
            # some strange violation of protocol, better know about this
            if ($data) {
              Carp::confess("info protocol violation: [$data]");
            }
          }
          if ($str->{'protocol'} eq 'stdout') {
            if (!$opts->{'discard_output'}) {
              $child_stdout .= $data;
              $child_merged .= $data;
            }

            if ($opts->{'stdout_handler'} && ref($opts->{'stdout_handler'}) eq 'CODE') {
              $opts->{'stdout_handler'}->($data);
            }
          }
          if ($str->{'protocol'} eq 'stderr') {
            if (!$opts->{'discard_output'}) {
              $child_stderr .= $data;
              $child_merged .= $data;
            }

            if ($opts->{'stderr_handler'} && ref($opts->{'stderr_handler'}) eq 'CODE') {
              $opts->{'stderr_handler'}->($data);
            }
          }
 
          # process may finish (waitpid returns -1) before
          # we've read all of its output because of buffering;
          # so try to read all the way it is possible to read
          # in such case - this shouldn't be too much (unless
          # the buffer size is HUGE -- should introduce
          # another counter in such case, maybe later)
          #
          push @{$ready_fds}, $select->can_read(1/100) if $child_finished;
        }

        if ($opts->{'wait_loop_callback'} && ref($opts->{'wait_loop_callback'}) eq 'CODE') {
          $opts->{'wait_loop_callback'}->();
        }

        Time::HiRes::usleep(1);
      }

      # $child_pid_pid is not defined in two cases:
      #  * when our child was killed before
      #    it had chance to tell us the pid
      #    of the child it spawned. we can do
      #    nothing in this case :(
      #  * our child successfully reaped its child,
      #    we have nothing left to do in this case
      #
      # defined $child_pid_pid means child's child
      # has not died but nobody is waiting for it,
      # killing it brutally.
      #
      if ($child_child_pid) {
        kill_gently($child_child_pid);
      }

      # in case there are forks in child which
      # do not forward or process signals (TERM) correctly
      # kill whole child process group, effectively trying
      # not to return with some children or their parts still running
      #
      # to be more accurate -- we need to be sure
      # that this is process group created by our child
      # (and not some other process group with the same pgid,
      # created just after death of our child) -- fortunately
      # this might happen only when process group ids
      # are reused quickly (there are lots of processes
      # spawning new process groups for example)
      #
      if ($opts->{'clean_up_children'}) {
        kill(-9, $pid);
      }

  #    print "child $pid finished\n";

      close($child_stdout_socket);
      close($child_stderr_socket);
      close($child_info_socket);

      my $o = {
        'stdout' => $child_stdout,
        'stderr' => $child_stderr,
        'merged' => $child_merged,
        'timeout' => $child_timedout ? $opts->{'timeout'} : 0,
        'exit_code' => $child_exit_code,
        'parent_died' => $parent_died,
        'killed_by_signal' => $child_killed_by_signal,
        'child_pgid' => $pid,
        'cmd' => $cmd,
        };

      my $err_msg = '';
      if ($o->{'exit_code'}) {
        $err_msg .= "exited with code [$o->{'exit_code'}]\n";
      }
      if ($o->{'timeout'}) {
        $err_msg .= "ran more than [$o->{'timeout'}] seconds\n";
      }
      if ($o->{'parent_died'}) {
        $err_msg .= "parent died\n";
      }
      if ($o->{'stdout'} && !$opts->{'non_empty_stdout_ok'}) {
        $err_msg .= "stdout:\n" . $o->{'stdout'} . "\n";
      }
      if ($o->{'stderr'}) {
        $err_msg .= "stderr:\n" . $o->{'stderr'} . "\n";
      }
      if ($o->{'killed_by_signal'}) {
        $err_msg .= "killed by signal [" . $o->{'killed_by_signal'} . "]\n";
      }
      $o->{'err_msg'} = $err_msg;

      if ($orig_sig_child) {
        $SIG{'CHLD'} = $orig_sig_child;
      }
      else {
        delete($SIG{'CHLD'});
      }

      uninstall_signals();

      return $o;
    }
    else {
      Carp::confess("cannot fork: $!") unless defined($pid);

      # create new process session for open3 call,
      # so we hopefully can kill all the subprocesses
      # which might be spawned in it (except for those
      # which do setsid theirselves -- can't do anything
      # with those)

      POSIX::setsid() == -1 and Carp::confess("Error running setsid: " . $!);

      if ($opts->{'child_BEGIN'} && ref($opts->{'child_BEGIN'}) eq 'CODE') {
        $opts->{'child_BEGIN'}->();
      }

      close($child_stdout_socket);
      close($child_stderr_socket);
      close($child_info_socket);

      my $child_exit_code;

      # allow both external programs
      # and internal perl calls
      if (!ref($cmd)) {
        $child_exit_code = open3_run($cmd, {
          'parent_info' => $parent_info_socket,
          'parent_stdout' => $parent_stdout_socket,
          'parent_stderr' => $parent_stderr_socket,
          'child_stdin' => $opts->{'child_stdin'},
          'original_ppid' => $ppid,
          });
      }
      elsif (ref($cmd) eq 'CODE') {
        # reopen STDOUT and STDERR for child code:
        # https://rt.cpan.org/Ticket/Display.html?id=85912
        open STDOUT, '>&', $parent_stdout_socket || Carp::confess("Unable to reopen STDOUT: $!\n");
        open STDERR, '>&', $parent_stderr_socket || Carp::confess("Unable to reopen STDERR: $!\n");

        $child_exit_code = $cmd->({
          'opts' => $opts,
          'parent_info' => $parent_info_socket,
          'parent_stdout' => $parent_stdout_socket,
          'parent_stderr' => $parent_stderr_socket,
          'child_stdin' => $opts->{'child_stdin'},
          });
      }
      else {
        print $parent_stderr_socket "Invalid command reference: " . ref($cmd) . "\n";
        $child_exit_code = 1;
      }

      close($parent_stdout_socket);
      close($parent_stderr_socket);
      close($parent_info_socket);

      if ($opts->{'child_END'} && ref($opts->{'child_END'}) eq 'CODE') {
        $opts->{'child_END'}->();
      }

      $| = 1;
      POSIX::_exit $child_exit_code;
    }
}

sub run {
    ### container to store things in
    my $self = bless {}, __PACKAGE__;

    my %hash = @_;

    ### if the user didn't provide a buffer, we'll store it here.
    my $def_buf = '';

    my($verbose,$cmd,$buffer,$timeout);
    my $tmpl = {
        verbose => { default  => $VERBOSE,  store => \$verbose },
        buffer  => { default  => \$def_buf, store => \$buffer },
        command => { required => 1,         store => \$cmd,
                     allow    => sub { !ref($_[0]) or ref($_[0]) eq 'ARRAY' },
        },
        timeout => { default  => 0,         store => \$timeout },
    };

    unless( check( $tmpl, \%hash, $VERBOSE ) ) {
        Carp::carp( loc( "Could not validate input: %1",
                         Params::Check->last_error ) );
        return;
    };

    $cmd = _quote_args_vms( $cmd ) if IS_VMS;

    ### strip any empty elements from $cmd if present
    if ( $ALLOW_NULL_ARGS ) {
      $cmd = [ grep { defined } @$cmd ] if ref $cmd;
    }
    else {
      $cmd = [ grep { defined && length } @$cmd ] if ref $cmd;
    }

    my $pp_cmd = (ref $cmd ? "@$cmd" : $cmd);
    print loc("Running [%1]...\n", $pp_cmd ) if $verbose;

    ### did the user pass us a buffer to fill or not? if so, set this
    ### flag so we know what is expected of us
    ### XXX this is now being ignored. in the future, we could add diagnostic
    ### messages based on this logic
    #my $user_provided_buffer = $buffer == \$def_buf ? 0 : 1;

    ### buffers that are to be captured
    my( @buffer, @buff_err, @buff_out );

    ### capture STDOUT
    my $_out_handler = sub {
        my $buf = shift;
        return unless defined $buf;

        print STDOUT $buf if $verbose;
        push @buffer,   $buf;
        push @buff_out, $buf;
    };

    ### capture STDERR
    my $_err_handler = sub {
        my $buf = shift;
        return unless defined $buf;

        print STDERR $buf if $verbose;
        push @buffer,   $buf;
        push @buff_err, $buf;
    };


    ### flag to indicate we have a buffer captured
    my $have_buffer = $self->can_capture_buffer ? 1 : 0;

    ### flag indicating if the subcall went ok
    my $ok;

    ### don't look at previous errors:
    local $?;
    local $@;
    local $!;

    ### we might be having a timeout set
    eval {
        local $SIG{ALRM} = sub { die bless sub {
            ALARM_CLASS .
            qq[: Command '$pp_cmd' aborted by alarm after $timeout seconds]
        }, ALARM_CLASS } if $timeout;
        alarm $timeout || 0;

        ### IPC::Run is first choice if $USE_IPC_RUN is set.
        if( !IS_WIN32 and $USE_IPC_RUN and $self->can_use_ipc_run( 1 ) ) {
            ### ipc::run handlers needs the command as a string or an array ref

            $self->_debug( "# Using IPC::Run. Have buffer: $have_buffer" )
                if $DEBUG;

            $ok = $self->_ipc_run( $cmd, $_out_handler, $_err_handler );

        ### since IPC::Open3 works on all platforms, and just fails on
        ### win32 for capturing buffers, do that ideally
        } elsif ( $USE_IPC_OPEN3 and $self->can_use_ipc_open3( 1 ) ) {

            $self->_debug("# Using IPC::Open3. Have buffer: $have_buffer")
                if $DEBUG;

            ### in case there are pipes in there;
            ### IPC::Open3 will call exec and exec will do the right thing

            my $method = IS_WIN32 ? '_open3_run_win32' : '_open3_run';

            $ok = $self->$method(
                                    $cmd, $_out_handler, $_err_handler, $verbose
                                );

        ### if we are allowed to run verbose, just dispatch the system command
        } else {
            $self->_debug( "# Using system(). Have buffer: $have_buffer" )
                if $DEBUG;
            $ok = $self->_system_run( $cmd, $verbose );
        }

        alarm 0;
    };

    ### restore STDIN after duping, or STDIN will be closed for
    ### this current perl process!
    $self->__reopen_fds( @{ $self->_fds} ) if $self->_fds;

    my $err;
    unless( $ok ) {
        ### alarm happened
        if ( $@ and ref $@ and $@->isa( ALARM_CLASS ) ) {
            $err = $@->();  # the error code is an expired alarm

        ### another error happened, set by the dispatchub
        } else {
            $err = $self->error;
        }
    }

    ### fill the buffer;
    $$buffer = join '', @buffer if @buffer;

    ### return a list of flags and buffers (if available) in list
    ### context, or just a simple 'ok' in scalar
    return wantarray
                ? $have_buffer
                    ? ($ok, $err, \@buffer, \@buff_out, \@buff_err)
                    : ($ok, $err )
                : $ok


}

sub _open3_run_win32 {
  my $self    = shift;
  my $cmd     = shift;
  my $outhand = shift;
  my $errhand = shift;

  require Socket;

  my $pipe = sub {
    socketpair($_[0], $_[1], &Socket::AF_UNIX, &Socket::SOCK_STREAM, &Socket::PF_UNSPEC)
        or return undef;
    shutdown($_[0], 1);  # No more writing for reader
    shutdown($_[1], 0);  # No more reading for writer
    return 1;
  };

  my $open3 = sub {
    local (*TO_CHLD_R,     *TO_CHLD_W);
    local (*FR_CHLD_R,     *FR_CHLD_W);
    local (*FR_CHLD_ERR_R, *FR_CHLD_ERR_W);

    $pipe->(*TO_CHLD_R,     *TO_CHLD_W    ) or die $^E;
    $pipe->(*FR_CHLD_R,     *FR_CHLD_W    ) or die $^E;
    $pipe->(*FR_CHLD_ERR_R, *FR_CHLD_ERR_W) or die $^E;

    my $pid = IPC::Open3::open3('>&TO_CHLD_R', '<&FR_CHLD_W', '<&FR_CHLD_ERR_W', @_);

    return ( $pid, *TO_CHLD_W, *FR_CHLD_R, *FR_CHLD_ERR_R );
  };

  $cmd = [ grep { defined && length } @$cmd ] if ref $cmd;
  $cmd = $self->__fix_cmd_whitespace_and_special_chars( $cmd );

  my ($pid, $to_chld, $fr_chld, $fr_chld_err) =
    $open3->( ( ref $cmd ? @$cmd : $cmd ) );

  my $in_sel  = IO::Select->new();
  my $out_sel = IO::Select->new();

  my %objs;

  $objs{ fileno( $fr_chld ) } = $outhand;
  $objs{ fileno( $fr_chld_err ) } = $errhand;
  $in_sel->add( $fr_chld );
  $in_sel->add( $fr_chld_err );

  close($to_chld);

  while ($in_sel->count() + $out_sel->count()) {
    my ($ins, $outs) = IO::Select::select($in_sel, $out_sel, undef);

    for my $fh (@$ins) {
        my $obj = $objs{ fileno($fh) };
        my $buf;
        my $bytes_read = sysread($fh, $buf, 64*1024 ); #, length($buf));
        if (!$bytes_read) {
            $in_sel->remove($fh);
        }
        else {
            $obj->( "$buf" );
        }
      }

      for my $fh (@$outs) {
      }
  }

  waitpid($pid, 0);

  ### some error occurred
  if( $? ) {
        $self->error( $self->_pp_child_error( $cmd, $? ) );
        $self->ok( 0 );
        return;
  } else {
        return $self->ok( 1 );
  }
}

sub _open3_run {
    my $self            = shift;
    my $cmd             = shift;
    my $_out_handler    = shift;
    my $_err_handler    = shift;
    my $verbose         = shift || 0;

    ### Following code are adapted from Friar 'abstracts' in the
    ### Perl Monastery (http://www.perlmonks.org/index.pl?node_id=151886).
    ### XXX that code didn't work.
    ### we now use the following code, thanks to theorbtwo

    ### define them beforehand, so we always have defined FH's
    ### to read from.
    use Symbol;
    my $kidout      = Symbol::gensym();
    my $kiderror    = Symbol::gensym();

    ### Dup the filehandle so we can pass 'our' STDIN to the
    ### child process. This stops us from having to pump input
    ### from ourselves to the childprocess. However, we will need
    ### to revive the FH afterwards, as IPC::Open3 closes it.
    ### We'll do the same for STDOUT and STDERR. It works without
    ### duping them on non-unix derivatives, but not on win32.
    my @fds_to_dup = ( IS_WIN32 && !$verbose
                            ? qw[STDIN STDOUT STDERR]
                            : qw[STDIN]
                        );
    $self->_fds( \@fds_to_dup );
    $self->__dup_fds( @fds_to_dup );

    ### pipes have to come in a quoted string, and that clashes with
    ### whitespace. This sub fixes up such commands so they run properly
    $cmd = $self->__fix_cmd_whitespace_and_special_chars( $cmd );

    ### don't stringify @$cmd, so spaces in filenames/paths are
    ### treated properly
    my $pid = eval {
        IPC::Open3::open3(
                    '<&STDIN',
                    (IS_WIN32 ? '>&STDOUT' : $kidout),
                    (IS_WIN32 ? '>&STDERR' : $kiderror),
                    ( ref $cmd ? @$cmd : $cmd ),
                );
    };

    ### open3 error occurred
    if( $@ and $@ =~ /^open3:/ ) {
        $self->ok( 0 );
        $self->error( $@ );
        return;
    };

    ### use OUR stdin, not $kidin. Somehow,
    ### we never get the input.. so jump through
    ### some hoops to do it :(
    my $selector = IO::Select->new(
                        (IS_WIN32 ? \*STDERR : $kiderror),
                        \*STDIN,
                        (IS_WIN32 ? \*STDOUT : $kidout)
                    );

    STDOUT->autoflush(1);   STDERR->autoflush(1);   STDIN->autoflush(1);
    $kidout->autoflush(1)   if UNIVERSAL::can($kidout,   'autoflush');
    $kiderror->autoflush(1) if UNIVERSAL::can($kiderror, 'autoflush');

    ### add an explicit break statement
    ### code courtesy of theorbtwo from #london.pm
    my $stdout_done = 0;
    my $stderr_done = 0;
    OUTER: while ( my @ready = $selector->can_read ) {

        for my $h ( @ready ) {
            my $buf;

            ### $len is the amount of bytes read
            my $len = sysread( $h, $buf, 4096 );    # try to read 4096 bytes

            ### see perldoc -f sysread: it returns undef on error,
            ### so bail out.
            if( not defined $len ) {
                warn(loc("Error reading from process: %1", $!));
                last OUTER;
            }

            ### check for $len. it may be 0, at which point we're
            ### done reading, so don't try to process it.
            ### if we would print anyway, we'd provide bogus information
            $_out_handler->( "$buf" ) if $len && $h == $kidout;
            $_err_handler->( "$buf" ) if $len && $h == $kiderror;

            ### Wait till child process is done printing to both
            ### stdout and stderr.
            $stdout_done = 1 if $h == $kidout   and $len == 0;
            $stderr_done = 1 if $h == $kiderror and $len == 0;
            last OUTER if ($stdout_done && $stderr_done);
        }
    }

    waitpid $pid, 0; # wait for it to die

    ### restore STDIN after duping, or STDIN will be closed for
    ### this current perl process!
    ### done in the parent call now
    # $self->__reopen_fds( @fds_to_dup );

    ### some error occurred
    if( $? ) {
        $self->error( $self->_pp_child_error( $cmd, $? ) );
        $self->ok( 0 );
        return;
    } else {
        return $self->ok( 1 );
    }
}

### Text::ParseWords::shellwords() uses unix semantics. that will break
### on win32
{   my $parse_sub = IS_WIN32
                        ? __PACKAGE__->can('_split_like_shell_win32')
                        : Text::ParseWords->can('shellwords');

    sub _ipc_run {
        my $self            = shift;
        my $cmd             = shift;
        my $_out_handler    = shift;
        my $_err_handler    = shift;

        STDOUT->autoflush(1); STDERR->autoflush(1);

        ### a command like:
        # [
        #     '/usr/bin/gzip',
        #     '-cdf',
        #     '/Users/kane/sources/p4/other/archive-extract/t/src/x.tgz',
        #     '|',
        #     '/usr/bin/tar',
        #     '-tf -'
        # ]
        ### needs to become:
        # [
        #     ['/usr/bin/gzip', '-cdf',
        #       '/Users/kane/sources/p4/other/archive-extract/t/src/x.tgz']
        #     '|',
        #     ['/usr/bin/tar', '-tf -']
        # ]


        my @command;
        my $special_chars;

        my $re = do { my $x = join '', SPECIAL_CHARS; qr/([$x])/ };
        if( ref $cmd ) {
            my $aref = [];
            for my $item (@$cmd) {
                if( $item =~ $re ) {
                    push @command, $aref, $item;
                    $aref = [];
                    $special_chars .= $1;
                } else {
                    push @$aref, $item;
                }
            }
            push @command, $aref;
        } else {
            @command = map { if( $_ =~ $re ) {
                                $special_chars .= $1; $_;
                             } else {
#                                [ split /\s+/ ]
                                 [ map { m/[ ]/ ? qq{'$_'} : $_ } $parse_sub->($_) ]
                             }
                        } split( /\s*$re\s*/, $cmd );
        }

        ### if there's a pipe in the command, *STDIN needs to
        ### be inserted *BEFORE* the pipe, to work on win32
        ### this also works on *nix, so we should do it when possible
        ### this should *also* work on multiple pipes in the command
        ### if there's no pipe in the command, append STDIN to the back
        ### of the command instead.
        ### XXX seems IPC::Run works it out for itself if you just
        ### don't pass STDIN at all.
        #     if( $special_chars and $special_chars =~ /\|/ ) {
        #         ### only add STDIN the first time..
        #         my $i;
        #         @command = map { ($_ eq '|' && not $i++)
        #                             ? ( \*STDIN, $_ )
        #                             : $_
        #                         } @command;
        #     } else {
        #         push @command, \*STDIN;
        #     }

        # \*STDIN is already included in the @command, see a few lines up
        my $ok = eval { IPC::Run::run(   @command,
                                fileno(STDOUT).'>',
                                $_out_handler,
                                fileno(STDERR).'>',
                                $_err_handler
                            )
                        };

        ### all is well
        if( $ok ) {
            return $self->ok( $ok );

        ### some error occurred
        } else {
            $self->ok( 0 );

            ### if the eval fails due to an exception, deal with it
            ### unless it's an alarm
            if( $@ and not UNIVERSAL::isa( $@, ALARM_CLASS ) ) {
                $self->error( $@ );

            ### if it *is* an alarm, propagate
            } elsif( $@ ) {
                die $@;

            ### some error in the sub command
            } else {
                $self->error( $self->_pp_child_error( $cmd, $? ) );
            }

            return;
        }
    }
}

sub _system_run {
    my $self    = shift;
    my $cmd     = shift;
    my $verbose = shift || 0;

    ### pipes have to come in a quoted string, and that clashes with
    ### whitespace. This sub fixes up such commands so they run properly
    $cmd = $self->__fix_cmd_whitespace_and_special_chars( $cmd );

    my @fds_to_dup = $verbose ? () : qw[STDOUT STDERR];
    $self->_fds( \@fds_to_dup );
    $self->__dup_fds( @fds_to_dup );

    ### system returns 'true' on failure -- the exit code of the cmd
    $self->ok( 1 );
    system( ref $cmd ? @$cmd : $cmd ) == 0 or do {
        $self->error( $self->_pp_child_error( $cmd, $? ) );
        $self->ok( 0 );
    };

    ### done in the parent call now
    #$self->__reopen_fds( @fds_to_dup );

    return unless $self->ok;
    return $self->ok;
}

{   my %sc_lookup = map { $_ => $_ } SPECIAL_CHARS;


    sub __fix_cmd_whitespace_and_special_chars {
        my $self = shift;
        my $cmd  = shift;

        ### command has a special char in it
        if( ref $cmd and grep { $sc_lookup{$_} } @$cmd ) {

            ### since we have special chars, we have to quote white space
            ### this *may* conflict with the parsing :(
            my $fixed;
            my @cmd = map { / / ? do { $fixed++; QUOTE.$_.QUOTE } : $_ } @$cmd;

            $self->_debug( "# Quoted $fixed arguments containing whitespace" )
                    if $DEBUG && $fixed;

            ### stringify it, so the special char isn't escaped as argument
            ### to the program
            $cmd = join ' ', @cmd;
        }

        return $cmd;
    }
}

### Command-line arguments (but not the command itself) must be quoted
### to ensure case preservation. Borrowed from Module::Build with adaptations.
### Patch for this supplied by Craig Berry, see RT #46288: [PATCH] Add argument
### quoting for run() on VMS
sub _quote_args_vms {
  ### Returns a command string with proper quoting so that the subprocess
  ### sees this same list of args, or if we get a single arg that is an
  ### array reference, quote the elements of it (except for the first)
  ### and return the reference.
  my @args = @_;
  my $got_arrayref = (scalar(@args) == 1
                      && UNIVERSAL::isa($args[0], 'ARRAY'))
                   ? 1
                   : 0;

  @args = split(/\s+/, $args[0]) unless $got_arrayref || scalar(@args) > 1;

  my $cmd = $got_arrayref ? shift @{$args[0]} : shift @args;

  ### Do not quote qualifiers that begin with '/' or previously quoted args.
  map { if (/^[^\/\"]/) {
          $_ =~ s/\"/""/g;     # escape C<"> by doubling
          $_ = q(").$_.q(");
        }
  }
    ($got_arrayref ? @{$args[0]}
                   : @args
    );

  $got_arrayref ? unshift(@{$args[0]}, $cmd) : unshift(@args, $cmd);

  return $got_arrayref ? $args[0]
                       : join(' ', @args);
}


### XXX this is cribbed STRAIGHT from M::B 0.30 here:
### http://search.cpan.org/src/KWILLIAMS/Module-Build-0.30/lib/Module/Build/Platform/Windows.pm:split_like_shell
### XXX this *should* be integrated into text::parsewords
sub _split_like_shell_win32 {
  # As it turns out, Windows command-parsing is very different from
  # Unix command-parsing.  Double-quotes mean different things,
  # backslashes don't necessarily mean escapes, and so on.  So we
  # can't use Text::ParseWords::shellwords() to break a command string
  # into words.  The algorithm below was bashed out by Randy and Ken
  # (mostly Randy), and there are a lot of regression tests, so we
  # should feel free to adjust if desired.

  local $_ = shift;

  my @argv;
  return @argv unless defined() && length();

  my $arg = '';
  my( $i, $quote_mode ) = ( 0, 0 );

  while ( $i < length() ) {

    my $ch      = substr( $_, $i  , 1 );
    my $next_ch = substr( $_, $i+1, 1 );

    if ( $ch eq '\\' && $next_ch eq '"' ) {
      $arg .= '"';
      $i++;
    } elsif ( $ch eq '\\' && $next_ch eq '\\' ) {
      $arg .= '\\';
      $i++;
    } elsif ( $ch eq '"' && $next_ch eq '"' && $quote_mode ) {
      $quote_mode = !$quote_mode;
      $arg .= '"';
      $i++;
    } elsif ( $ch eq '"' && $next_ch eq '"' && !$quote_mode &&
          ( $i + 2 == length()  ||
        substr( $_, $i + 2, 1 ) eq ' ' )
        ) { # for cases like: a"" => [ 'a' ]
      push( @argv, $arg );
      $arg = '';
      $i += 2;
    } elsif ( $ch eq '"' ) {
      $quote_mode = !$quote_mode;
    } elsif ( $ch eq ' ' && !$quote_mode ) {
      push( @argv, $arg ) if defined( $arg ) && length( $arg );
      $arg = '';
      ++$i while substr( $_, $i + 1, 1 ) eq ' ';
    } else {
      $arg .= $ch;
    }

    $i++;
  }

  push( @argv, $arg ) if defined( $arg ) && length( $arg );
  return @argv;
}



{   use File::Spec;
    use Symbol;

    my %Map = (
        STDOUT => [qw|>&|, \*STDOUT, Symbol::gensym() ],
        STDERR => [qw|>&|, \*STDERR, Symbol::gensym() ],
        STDIN  => [qw|<&|, \*STDIN,  Symbol::gensym() ],
    );

    ### dups FDs and stores them in a cache
    sub __dup_fds {
        my $self    = shift;
        my @fds     = @_;

        __PACKAGE__->_debug( "# Closing the following fds: @fds" ) if $DEBUG;

        for my $name ( @fds ) {
            my($redir, $fh, $glob) = @{$Map{$name}} or (
                Carp::carp(loc("No such FD: '%1'", $name)), next );

            ### MUST use the 2-arg version of open for dup'ing for
            ### 5.6.x compatibility. 5.8.x can use 3-arg open
            ### see perldoc5.6.2 -f open for details
            open $glob, $redir . fileno($fh) or (
                        Carp::carp(loc("Could not dup '$name': %1", $!)),
                        return
                    );

            ### we should re-open this filehandle right now, not
            ### just dup it
            ### Use 2-arg version of open, as 5.5.x doesn't support
            ### 3-arg version =/
            if( $redir eq '>&' ) {
                open( $fh, '>' . File::Spec->devnull ) or (
                    Carp::carp(loc("Could not reopen '$name': %1", $!)),
                    return
                );
            }
        }

        return 1;
    }

    ### reopens FDs from the cache
    sub __reopen_fds {
        my $self    = shift;
        my @fds     = @_;

        __PACKAGE__->_debug( "# Reopening the following fds: @fds" ) if $DEBUG;

        for my $name ( @fds ) {
            my($redir, $fh, $glob) = @{$Map{$name}} or (
                Carp::carp(loc("No such FD: '%1'", $name)), next );

            ### MUST use the 2-arg version of open for dup'ing for
            ### 5.6.x compatibility. 5.8.x can use 3-arg open
            ### see perldoc5.6.2 -f open for details
            open( $fh, $redir . fileno($glob) ) or (
                    Carp::carp(loc("Could not restore '$name': %1", $!)),
                    return
                );

            ### close this FD, we're not using it anymore
            close $glob;
        }
        return 1;

    }
}

sub _debug {
    my $self    = shift;
    my $msg     = shift or return;
    my $level   = shift || 0;

    local $Carp::CarpLevel += $level;
    Carp::carp($msg);

    return 1;
}

sub _pp_child_error {
    my $self    = shift;
    my $cmd     = shift or return;
    my $ce      = shift or return;
    my $pp_cmd  = ref $cmd ? "@$cmd" : $cmd;


    my $str;
    if( $ce == -1 ) {
        ### Include $! in the error message, so that the user can
        ### see 'No such file or directory' versus 'Permission denied'
        ### versus 'Cannot fork' or whatever the cause was.
        $str = "Failed to execute '$pp_cmd': $!";

    } elsif ( $ce & 127 ) {
        ### some signal
        $str = loc( "'%1' died with signal %2, %3 coredump",
               $pp_cmd, ($ce & 127), ($ce & 128) ? 'with' : 'without');

    } else {
        ### Otherwise, the command run but gave error status.
        $str = "'$pp_cmd' exited with value " . ($ce >> 8);
    }

    $self->_debug( "# Child error '$ce' translated to: $str" ) if $DEBUG;

    return $str;
}

1;

__END__

=head2 $q = QUOTE

Returns the character used for quoting strings on this platform. This is
usually a C<'> (single quote) on most systems, but some systems use different
quotes. For example, C<Win32> uses C<"> (double quote).

You can use it as follows:

  use IPC::Cmd qw[run QUOTE];
  my $cmd = q[echo ] . QUOTE . q[foo bar] . QUOTE;

This makes sure that C<foo bar> is treated as a string, rather than two
separate arguments to the C<echo> function.

=head1 HOW IT WORKS

C<run> will try to execute your command using the following logic:

=over 4

=item *

If you have C<IPC::Run> installed, and the variable C<$IPC::Cmd::USE_IPC_RUN>
is set to true (See the L<"Global Variables"> section) use that to execute
the command. You will have the full output available in buffers, interactive commands
are sure to work  and you are guaranteed to have your verbosity
settings honored cleanly.

=item *

Otherwise, if the variable C<$IPC::Cmd::USE_IPC_OPEN3> is set to true
(See the L<"Global Variables"> section), try to execute the command using
L<IPC::Open3>. Buffers will be available on all platforms,
interactive commands will still execute cleanly, and also your verbosity
settings will be adhered to nicely;

=item *

Otherwise, if you have the C<verbose> argument set to true, we fall back
to a simple C<system()> call. We cannot capture any buffers, but
interactive commands will still work.

=item *

Otherwise we will try and temporarily redirect STDERR and STDOUT, do a
C<system()> call with your command and then re-open STDERR and STDOUT.
This is the method of last resort and will still allow you to execute
your commands cleanly. However, no buffers will be available.

=back

=head1 Global Variables

The behaviour of IPC::Cmd can be altered by changing the following
global variables:

=head2 $IPC::Cmd::VERBOSE

This controls whether IPC::Cmd will print any output from the
commands to the screen or not. The default is 0.

=head2 $IPC::Cmd::USE_IPC_RUN

This variable controls whether IPC::Cmd will try to use L<IPC::Run>
when available and suitable.

=head2 $IPC::Cmd::USE_IPC_OPEN3

This variable controls whether IPC::Cmd will try to use L<IPC::Open3>
when available and suitable. Defaults to true.

=head2 $IPC::Cmd::WARN

This variable controls whether run-time warnings should be issued, like
the failure to load an C<IPC::*> module you explicitly requested.

Defaults to true. Turn this off at your own risk.

=head2 $IPC::Cmd::INSTANCES

This variable controls whether C<can_run> will return all instances of
the binary it finds in the C<PATH> when called in a list context.

Defaults to false, set to true to enable the described behaviour.

=head2 $IPC::Cmd::ALLOW_NULL_ARGS

This variable controls whether C<run> will remove any empty/null arguments
it finds in command arguments.

Defaults to false, so it will remove null arguments. Set to true to allow
them.

=head1 Caveats

=over 4

=item Whitespace and IPC::Open3 / system()

When using C<IPC::Open3> or C<system>, if you provide a string as the
C<command> argument, it is assumed to be appropriately escaped. You can
use the C<QUOTE> constant to use as a portable quote character (see above).
However, if you provide an array reference, special rules apply:

If your command contains B<special characters> (< > | &), it will
be internally stringified before executing the command, to avoid that these
special characters are escaped and passed as arguments instead of retaining
their special meaning.

However, if the command contained arguments that contained whitespace,
stringifying the command would lose the significance of the whitespace.
Therefore, C<IPC::Cmd> will quote any arguments containing whitespace in your
command if the command is passed as an arrayref and contains special characters.

=item Whitespace and IPC::Run

When using C<IPC::Run>, if you provide a string as the C<command> argument,
the string will be split on whitespace to determine the individual elements
of your command. Although this will usually just Do What You Mean, it may
break if you have files or commands with whitespace in them.

If you do not wish this to happen, you should provide an array
reference, where all parts of your command are already separated out.
Note however, if there are extra or spurious whitespaces in these parts,
the parser or underlying code may not interpret it correctly, and
cause an error.

Example:
The following code

    gzip -cdf foo.tar.gz | tar -xf -

should either be passed as

    "gzip -cdf foo.tar.gz | tar -xf -"

or as

    ['gzip', '-cdf', 'foo.tar.gz', '|', 'tar', '-xf', '-']

But take care not to pass it as, for example

    ['gzip -cdf foo.tar.gz', '|', 'tar -xf -']

Since this will lead to issues as described above.


=item IO Redirect

Currently it is too complicated to parse your command for IO
redirections. For capturing STDOUT or STDERR there is a work around
however, since you can just inspect your buffers for the contents.

=item Interleaving STDOUT/STDERR

Neither IPC::Run nor IPC::Open3 can interleave STDOUT and STDERR. For short
bursts of output from a program, e.g. this sample,

    for ( 1..4 ) {
        $_ % 2 ? print STDOUT $_ : print STDERR $_;
    }

IPC::[Run|Open3] will first read all of STDOUT, then all of STDERR, meaning
the output looks like '13' on STDOUT and '24' on STDERR, instead of

    1
    2
    3
    4

This has been recorded in L<rt.cpan.org> as bug #37532: Unable to interleave
STDOUT and STDERR.

=back

=head1 See Also

L<IPC::Run>, L<IPC::Open3>

=head1 ACKNOWLEDGEMENTS

Thanks to James Mastros and Martijn van der Streek for their
help in getting L<IPC::Open3> to behave nicely.

Thanks to Petya Kohts for the C<run_forked> code.

=head1 BUG REPORTS

Please report bugs or other issues to E<lt>bug-ipc-cmd@rt.cpan.orgE<gt>.

=head1 AUTHOR

Original author: Jos Boumans E<lt>kane@cpan.orgE<gt>.
Current maintainer: Chris Williams E<lt>bingos@cpan.orgE<gt>.

=head1 COPYRIGHT

This library is free software; you may redistribute and/or modify it
under the same terms as Perl itself.

=cut
