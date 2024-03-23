seatd(1)

# NAME

seatd - A seat management daemon

# SYNOPSIS

*seatd* [options]

# OPTIONS

*-h*
	Show help message and quit.

*-n <fd>*
	FD to notify readiness on. A single newline will be written and the fd
	closed when seatd is ready to serve requests. This is compatible with
	s6's notification protocol.

*-u <user>*
	User to own the seatd socket.

*-g <group>*
	Group to own the seatd socket.

*-l <loglevel>*
	Log-level to use. Must be one of debug, info, error or silent. Defaults
	to error.

*-v*
	Show the version number and quit.

# DESCRIPTION

seatd provides central seat management, mediating access to shared resources
such as displays and input devices in a multi-session, multi-seat environment.

seatd operates over a UNIX domain socket, with *libseat* providing the
client-side of the protocol.

The location of the socket for seatd is set at compile-time.

# ENVIRONMENT

*SEATD_VTBOUND*
	If set to "0", the seat will not be bound to a VT.

# SEE ALSO

The libseat library, *<libseat.h>*, *seatd-launch*(1)

# AUTHORS

Maintained by Kenny Levinsen <contact@kl.wtf>, who is assisted by other
open-source contributors. For more information about seatd development, see
https://sr.ht/~kennylevinsen/seatd.
