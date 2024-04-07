#!perl -w

use strict;

use File::Basename;
use File::Spec;

use Test::More;

BEGIN {
    use_ok( 'Pod::Functions' );
}

# How do you test exported vars?
my( $pkg_ref, $exp_ref ) = ( \%Pod::Functions::Kinds, \%Kinds );
is( $pkg_ref, $exp_ref, '%Pod::Functions::Kinds exported' );

( $pkg_ref, $exp_ref ) = ( \%Pod::Functions::Type, \%Type );
is( $pkg_ref, $exp_ref, '%Pod::Functions::Type exported' );

( $pkg_ref, $exp_ref ) = ( \%Pod::Functions::Flavor, \%Flavor );
is( $pkg_ref, $exp_ref, '%Pod::Functions::Flavor exported' );

( $pkg_ref, $exp_ref ) = ( \%Pod::Functions::Type_Description, 
                           \%Type_Description );
is( $pkg_ref, $exp_ref, '%Pod::Functions::Type_Description exported' );

( $pkg_ref, $exp_ref ) = ( \@Pod::Functions::Type_Order, \@Type_Order );
is( $pkg_ref, $exp_ref, '@Pod::Functions::Type_Order exported' );

# Check @Type_Order
my @categories = qw(
    String  Regexp  Math   ARRAY  LIST      HASH    I/O
    Binary  File    Flow   Namespace Misc    Process
    Modules Objects Socket SysV   User      Network Time
);

is_deeply( \@Type_Order, \@categories,
    '@Type_Order' );

my @cat_keys = grep exists $Type_Description{ $_ } => @Type_Order;

is_deeply( \@cat_keys, \@categories,
    'keys() %Type_Description' );

SKIP: {
	my $test_out = do { local $/; <DATA> }; 
	
	skip( "Can't fork '$^X': $!", 1) 
	    unless open my $fh, qq[$^X "-I../../lib" Functions.pm |];
	my $fake_out = do { local $/; <$fh> };
	skip( "Pipe error: $!", 1)
	    unless close $fh;

	is( $fake_out, $test_out, 'run as plain program' );
}

foreach my $func (sort keys %Flavor) {
    my $desc = $Flavor{$func};
    like($desc, qr/^(?:[a-z]|SysV)/,
	 "Description for $desc starts with a lowercase letter or SysV");
}

done_testing();

=head1 NAME

Functions.t - Test Pod::Functions

=head1 AUTHOR

20011229 Abe Timmerman <abe@ztreet.demon.nl>

=cut

__DATA__

Functions for SCALARs or strings:
     chomp, chop, chr, crypt, fc, hex, index, lc, lcfirst,
     length, oct, ord, pack, q/STRING/, qq/STRING/, reverse,
     rindex, sprintf, substr, tr///, uc, ucfirst, y///

Regular expressions and pattern matching:
     m//, pos, qr/STRING/, quotemeta, s///, split, study

Numeric functions:
     abs, atan2, cos, exp, hex, int, log, oct, rand, sin, sqrt,
     srand

Functions for real @ARRAYs:
     each, keys, pop, push, shift, splice, unshift, values

Functions for list data:
     grep, join, map, qw/STRING/, reverse, sort, unpack

Functions for real %HASHes:
     delete, each, exists, keys, values

Input and output functions:
     binmode, close, closedir, dbmclose, dbmopen, die, eof,
     fileno, flock, format, getc, print, printf, read, readdir,
     readline, rewinddir, say, seek, seekdir, select, syscall,
     sysread, sysseek, syswrite, tell, telldir, truncate, warn,
     write

Functions for fixed-length data or records:
     pack, read, syscall, sysread, sysseek, syswrite, unpack,
     vec

Functions for filehandles, files, or directories:
     -X, chdir, chmod, chown, chroot, fcntl, glob, ioctl, link,
     lstat, mkdir, open, opendir, readlink, rename, rmdir,
     select, stat, symlink, sysopen, umask, unlink, utime

Keywords related to the control flow of your Perl program:
     __FILE__, __LINE__, __PACKAGE__, __SUB__, break, caller,
     continue, die, do, dump, eval, evalbytes, exit, goto,
     last, method, next, redo, return, sub, wantarray

Keywords related to scoping:
     caller, class, field, import, local, my, our, package,
     state, use

Miscellaneous functions:
     defined, formline, lock, prototype, reset, scalar, undef

Functions for processes and process groups:
     alarm, exec, fork, getpgrp, getppid, getpriority, kill,
     pipe, qx/STRING/, readpipe, setpgrp, setpriority, sleep,
     system, times, wait, waitpid

Keywords related to Perl modules:
     do, import, no, package, require, use

Keywords related to classes and object-orientation:
     bless, class, dbmclose, dbmopen, field, method, package,
     ref, tie, tied, untie, use

Low-level socket functions:
     accept, bind, connect, getpeername, getsockname,
     getsockopt, listen, recv, send, setsockopt, shutdown,
     socket, socketpair

System V interprocess communication functions:
     msgctl, msgget, msgrcv, msgsnd, semctl, semget, semop,
     shmctl, shmget, shmread, shmwrite

Fetching user and group info:
     endgrent, endhostent, endnetent, endpwent, getgrent,
     getgrgid, getgrnam, getlogin, getpwent, getpwnam,
     getpwuid, setgrent, setpwent

Fetching network info:
     endprotoent, endservent, gethostbyaddr, gethostbyname,
     gethostent, getnetbyaddr, getnetbyname, getnetent,
     getprotobyname, getprotobynumber, getprotoent,
     getservbyname, getservbyport, getservent, sethostent,
     setnetent, setprotoent, setservent

Time-related functions:
     gmtime, localtime, time, times
