package Test2::Util;
use strict;
use warnings;

our $VERSION = '1.302194';

use POSIX();
use Config qw/%Config/;
use Carp qw/croak/;

BEGIN {
    local ($@, $!, $SIG{__DIE__});
    *HAVE_PERLIO = eval { require PerlIO; PerlIO->VERSION(1.02); } ? sub() { 1 } : sub() { 0 };
}

our @EXPORT_OK = qw{
    try

    pkg_to_file

    get_tid USE_THREADS
    CAN_THREAD
    CAN_REALLY_FORK
    CAN_FORK

    CAN_SIGSYS

    IS_WIN32

    ipc_separator

    gen_uid

    do_rename do_unlink

    try_sig_mask

    clone_io
};
BEGIN { require Exporter; our @ISA = qw(Exporter) }

BEGIN {
    *IS_WIN32 = ($^O eq 'MSWin32') ? sub() { 1 } : sub() { 0 };
}

sub _can_thread {
    return 0 unless $] >= 5.008001;
    return 0 unless $Config{'useithreads'};

    # Threads are broken on perl 5.10.0 built with gcc 4.8+
    if ($] == 5.010000 && $Config{'ccname'} eq 'gcc' && $Config{'gccversion'}) {
        return 0 unless $Config{'gccversion'} =~ m/^(\d+)\.(\d+)/;
        my @parts = split /[\.\s]+/, $Config{'gccversion'};
        return 0 if $parts[0] > 4 || ($parts[0] == 4 && $parts[1] >= 8);
    }

    # Change to a version check if this ever changes
    return 0 if $INC{'Devel/Cover.pm'};
    return 1;
}

sub _can_fork {
    return 1 if $Config{d_fork};
    return 0 unless IS_WIN32 || $^O eq 'NetWare';
    return 0 unless $Config{useithreads};
    return 0 unless $Config{ccflags} =~ /-DPERL_IMPLICIT_SYS/;

    return _can_thread();
}

BEGIN {
    no warnings 'once';
    *CAN_THREAD      = _can_thread()   ? sub() { 1 } : sub() { 0 };
}
my $can_fork;
sub CAN_FORK () {
    return $can_fork
        if defined $can_fork;
    $can_fork = !!_can_fork();
    no warnings 'redefine';
    *CAN_FORK = $can_fork ? sub() { 1 } : sub() { 0 };
    $can_fork;
}
my $can_really_fork;
sub CAN_REALLY_FORK () {
    return $can_really_fork
        if defined $can_really_fork;
    $can_really_fork = !!$Config{d_fork};
    no warnings 'redefine';
    *CAN_REALLY_FORK = $can_really_fork ? sub() { 1 } : sub() { 0 };
    $can_really_fork;
}

sub _manual_try(&;@) {
    my $code = shift;
    my $args = \@_;
    my $err;

    my $die = delete $SIG{__DIE__};

    eval { $code->(@$args); 1 } or $err = $@ || "Error was squashed!\n";

    $die ? $SIG{__DIE__} = $die : delete $SIG{__DIE__};

    return (!defined($err), $err);
}

sub _local_try(&;@) {
    my $code = shift;
    my $args = \@_;
    my $err;

    no warnings;
    local $SIG{__DIE__};
    eval { $code->(@$args); 1 } or $err = $@ || "Error was squashed!\n";

    return (!defined($err), $err);
}

# Older versions of perl have a nasty bug on win32 when localizing a variable
# before forking or starting a new thread. So for those systems we use the
# non-local form. When possible though we use the faster 'local' form.
BEGIN {
    if (IS_WIN32 && $] < 5.020002) {
        *try = \&_manual_try;
    }
    else {
        *try = \&_local_try;
    }
}

BEGIN {
    if (CAN_THREAD) {
        if ($INC{'threads.pm'}) {
            # Threads are already loaded, so we do not need to check if they
            # are loaded each time
            *USE_THREADS = sub() { 1 };
            *get_tid     = sub() { threads->tid() };
        }
        else {
            # :-( Need to check each time to see if they have been loaded.
            *USE_THREADS = sub() { $INC{'threads.pm'} ? 1 : 0 };
            *get_tid     = sub() { $INC{'threads.pm'} ? threads->tid() : 0 };
        }
    }
    else {
        # No threads, not now, not ever!
        *USE_THREADS = sub() { 0 };
        *get_tid     = sub() { 0 };
    }
}

sub pkg_to_file {
    my $pkg = shift;
    my $file = $pkg;
    $file =~ s{(::|')}{/}g;
    $file .= '.pm';
    return $file;
}

sub ipc_separator() { "~" }

my $UID = 1;
sub gen_uid() { join ipc_separator() => ($$, get_tid(), time, $UID++) }

sub _check_for_sig_sys {
    my $sig_list = shift;
    return $sig_list =~ m/\bSYS\b/;
}

BEGIN {
    if (_check_for_sig_sys($Config{sig_name})) {
        *CAN_SIGSYS = sub() { 1 };
    }
    else {
        *CAN_SIGSYS = sub() { 0 };
    }
}

my %PERLIO_SKIP = (
    unix => 1,
    via  => 1,
);

sub clone_io {
    my ($fh) = @_;
    my $fileno = eval { fileno($fh) };

    return $fh if !defined($fileno) || !length($fileno) || $fileno < 0;

    open(my $out, '>&' . $fileno) or die "Can't dup fileno $fileno: $!";

    my %seen;
    my @layers = HAVE_PERLIO ? grep { !$PERLIO_SKIP{$_} and !$seen{$_}++ } PerlIO::get_layers($fh) : ();
    binmode($out, join(":", "", "raw", @layers));

    my $old = select $fh;
    my $af  = $|;
    select $out;
    $| = $af;
    select $old;

    return $out;
}

BEGIN {
    if (IS_WIN32) {
        my $max_tries = 5;

        *do_rename = sub {
            my ($from, $to) = @_;

            my $err;
            for (1 .. $max_tries) {
                return (1) if rename($from, $to);
                $err = "$!";
                last if $_ == $max_tries;
                sleep 1;
            }

            return (0, $err);
        };
        *do_unlink = sub {
            my ($file) = @_;

            my $err;
            for (1 .. $max_tries) {
                return (1) if unlink($file);
                $err = "$!";
                last if $_ == $max_tries;
                sleep 1;
            }

            return (0, "$!");
        };
    }
    else {
        *do_rename = sub {
            my ($from, $to) = @_;
            return (1) if rename($from, $to);
            return (0, "$!");
        };
        *do_unlink = sub {
            my ($file) = @_;
            return (1) if unlink($file);
            return (0, "$!");
        };
    }
}

sub try_sig_mask(&) {
    my $code = shift;

    my ($old, $blocked);
    unless(IS_WIN32) {
        my $to_block = POSIX::SigSet->new(
            POSIX::SIGINT(),
            POSIX::SIGALRM(),
            POSIX::SIGHUP(),
            POSIX::SIGTERM(),
            POSIX::SIGUSR1(),
            POSIX::SIGUSR2(),
        );
        $old = POSIX::SigSet->new;
        $blocked = POSIX::sigprocmask(POSIX::SIG_BLOCK(), $to_block, $old);
        # Silently go on if we failed to log signals, not much we can do.
    }

    my ($ok, $err) = &try($code);

    # If our block was successful we want to restore the old mask.
    POSIX::sigprocmask(POSIX::SIG_SETMASK(), $old, POSIX::SigSet->new()) if defined $blocked;

    return ($ok, $err);
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Util - Tools used by Test2 and friends.

=head1 DESCRIPTION

Collection of tools used by L<Test2> and friends.

=head1 EXPORTS

All exports are optional. You must specify subs to import.

=over 4

=item ($success, $error) = try { ... }

Eval the codeblock, return success or failure, and the error message. This code
protects $@ and $!, they will be restored by the end of the run. This code also
temporarily blocks $SIG{DIE} handlers.

=item protect { ... }

Similar to try, except that it does not catch exceptions. The idea here is to
protect $@ and $! from changes. $@ and $! will be restored to whatever they
were before the run so long as it is successful. If the run fails $! will still
be restored, but $@ will contain the exception being thrown.

=item CAN_FORK

True if this system is capable of true or pseudo-fork.

=item CAN_REALLY_FORK

True if the system can really fork. This will be false for systems where fork
is emulated.

=item CAN_THREAD

True if this system is capable of using threads.

=item USE_THREADS

Returns true if threads are enabled, false if they are not.

=item get_tid

This will return the id of the current thread when threads are enabled,
otherwise it returns 0.

=item my $file = pkg_to_file($package)

Convert a package name to a filename.

=item $string = ipc_separator()

Get the IPC separator. Currently this is always the string C<'~'>.

=item $string = gen_uid()

Generate a unique id (NOT A UUID). This will typically be the process id, the
thread id, the time, and an incrementing integer all joined with the
C<ipc_separator()>.

These ID's are unique enough for most purposes. For identical ids to be
generated you must have 2 processes with the same PID generate IDs at the same
time with the same current state of the incrementing integer. This is a
perfectly reasonable thing to expect to happen across multiple machines, but is
quite unlikely to happen on one machine.

This can fail to be unique if a process generates an id, calls exec, and does
it again after the exec and it all happens in less than a second. It can also
happen if the systems process id's cycle in less than a second allowing 2
different programs that use this generator to run with the same PID in less
than a second. Both these cases are sufficiently unlikely. If you need
universally unique ids, or ids that are unique in these conditions, look at
L<Data::UUID>.

=item ($ok, $err) = do_rename($old_name, $new_name)

Rename a file, this wraps C<rename()> in a way that makes it more reliable
cross-platform when trying to rename files you recently altered.

=item ($ok, $err) = do_unlink($filename)

Unlink a file, this wraps C<unlink()> in a way that makes it more reliable
cross-platform when trying to unlink files you recently altered.

=item ($ok, $err) = try_sig_mask { ... }

Complete an action with several signals masked, they will be unmasked at the
end allowing any signals that were intercepted to get handled.

This is primarily used when you need to make several actions atomic (against
some signals anyway).

Signals that are intercepted:

=over 4

=item SIGINT

=item SIGALRM

=item SIGHUP

=item SIGTERM

=item SIGUSR1

=item SIGUSR2

=back

=back

=head1 NOTES && CAVEATS

=over 4

=item 5.10.0

Perl 5.10.0 has a bug when compiled with newer gcc versions. This bug causes a
segfault whenever a new thread is launched. Test2 will attempt to detect
this, and note that the system is not capable of forking when it is detected.

=item Devel::Cover

Devel::Cover does not support threads. CAN_THREAD will return false if
Devel::Cover is loaded before the check is first run.

=back

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=item Kent Fredric E<lt>kentnl@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
