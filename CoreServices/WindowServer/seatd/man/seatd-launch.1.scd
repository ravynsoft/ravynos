seatd-launch(1)

# NAME

seatd-launch - Start a process with its own seatd instance

# SYNOPSIS

*seatd-launch* [options] [--] command

# OPTIONS

*-l <loglevel>*
	Log-level to pass to seatd. See *seatd*(1) for information about
	available log-levels.

*-h*
	Show help message and quit.

*-v*
	Show the version number and quit.

# DESCRIPTION

seatd-launch starts a seatd instance with a dedicated socket path, waits for it
to be ready, and starts the specified command with SEATD_SOCK set
appropriately.  Once the specified command terminates, the seatd instance is
also terminated.

seatd requires root privileges to perform its tasks. This can be achieved
through SUID of seatd-launch or by running seatd-launch as root. seatd-launch
will drop privileges from the effective user to the real user before running
the specified command. If the real user is root, this is simply a noop. You
should only run seatd-launch as root if you intend for the specified command to
run as root as well.

seatd-launch serves a similar purpose to the libseat "builtin" backend, but is
superior to it for two reasons:
. The specified command never runs as root
. The standard seatd executable and libseat backend is used

# EXIT STATUS

seatd-launch exits with the status of its child. When the child terminates on
a signal _N_, seatd-launch exits with the status 128 + _N_.

If seatd-launch fails because of another error, it exits with a non-zero
status.

# SEE ALSO

The libseat library, *<libseat.h>*, *seatd*(1)

# AUTHORS

Maintained by Kenny Levinsen <contact@kl.wtf>, who is assisted by other
open-source contributors. For more information about seatd development, see
https://sr.ht/~kennylevinsen/seatd.
