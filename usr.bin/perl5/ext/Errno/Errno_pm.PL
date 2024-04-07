use ExtUtils::MakeMaker;
use Config;
use strict;

our $VERSION = "1.37";

my %err = ();

my $IsMSWin32 = $^O eq 'MSWin32';

unlink "Errno.pm" if -f "Errno.pm";
unlink "Errno.tmp" if -f "Errno.tmp";
open OUT, '>', 'Errno.tmp' or die "Cannot open Errno.tmp: $!";
select OUT;
my $file;
my @files = get_files();
if ($Config{gccversion} ne '' && $^O eq 'MSWin32') {
    # MinGW complains "warning: #pragma system_header ignored outside include
    # file" if the header files are processed individually, so include them
    # all in .c file and process that instead.
    open INCS, '>', 'includes.c' or
	die "Cannot open includes.c";
    foreach $file (@files) {
	next if $file eq 'errno.c';
	next unless -f $file;
	print INCS qq[#include "$file"\n];
    }
    close INCS;
    process_file('includes.c');
    unlink 'includes.c';
}
else {
    foreach $file (@files) {
	process_file($file);
    }
}
write_errno_pm();
unlink "errno.c" if -f "errno.c";
close OUT or die "Error closing Errno.tmp: $!";
select STDOUT;
rename "Errno.tmp", "Errno.pm" or die "Cannot rename Errno.tmp to Errno.pm: $!";

sub process_file {
    my($file) = @_;

    # for win32 perl under cygwin, we need to get a windows pathname
    if ($^O eq 'MSWin32' && $Config{cc} =~ /\B-mno-cygwin\b/ &&
        defined($file) && !-f $file) {
        chomp($file = `cygpath -w "$file"`);
    }

    return unless defined $file and -f $file;
#    warn "Processing $file\n";

    local *FH;
    if (($^O eq 'VMS') && ($Config{vms_cc_type} ne 'gnuc')) {
	unless(open(FH," LIBRARY/EXTRACT=ERRNO/OUTPUT=SYS\$OUTPUT $file |")) {
            warn "Cannot open '$file'";
            return;
	}     
    } elsif ($Config{gccversion} ne '' && $^O ne 'darwin' ) {
	# With the -dM option, gcc outputs every #define it finds
	unless(open(FH,"$Config{cc} -E -dM $Config{cppflags} $file |")) {
            warn "Cannot open '$file'";
            return;
	}     
    } else {
	unless(open(FH, '<', $file)) {
	    # This file could be a temporary file created by cppstdin
	    # so only warn under -w, and return
            warn "Cannot open '$file'" if $^W;
            return;
	}
    }
    
    my $pat;
    if ($IsMSWin32) {
	$pat = '^\s*#\s*define\s+((?:WSA)?E\w+)\s+';
    }
    else {
	$pat = '^\s*#\s*define\s+(E\w+)\s+';
    }
    while(<FH>) {
	$err{$1} = 1
	    if /$pat/;
    }

    close(FH);
}

my $cppstdin;

sub default_cpp {
    unless (defined $cppstdin) {
	use File::Spec;
	$cppstdin = $Config{cppstdin};
	my $upup_cppstdin = File::Spec->catfile(File::Spec->updir,
						File::Spec->updir,
						"cppstdin");
	my $cppstdin_is_wrapper =
	    ($cppstdin eq 'cppstdin'
		and -f $upup_cppstdin
		    and -x $upup_cppstdin);
	$cppstdin = $upup_cppstdin if $cppstdin_is_wrapper;
    }
    return "$cppstdin $Config{cppflags} $Config{cppminus}";
}

sub get_files {
    my @file;
    # When cross-compiling we may store a path for gcc's "sysroot" option:
    my $sysroot = $Config{sysroot} || '';
    my $linux_errno_h;
    if ($^O eq 'linux') {
	# Some Linuxes have weird errno.hs which generate
	# no #file or #line directives
	($linux_errno_h) = grep { -e $_ } map { "$_/errno.h" }
	    "$sysroot/usr/include", "$sysroot/usr/local/include",
	    split / / => $Config{locincpth};
    }

    # VMS keeps its include files in system libraries
    if ($^O eq 'VMS') {
	push(@file, 'Sys$Library:DECC$RTLDEF.TLB');
    } elsif ($^O eq 'os390') {
	# OS/390 C compiler doesn't generate #file or #line directives
        # and it does not tag the header as 1047 (EBCDIC), so make a local
        # copy and tag it
        my $cp = `cp /usr/include/errno.h ./errno.h`;
        my $chtag = `chtag -t -cIBM-1047 ./errno.h`;
	push(@file, './errno.h');
    } elsif ($Config{archname} eq 'arm-riscos') {
	# Watch out for cross compiling for RISC OS
	my $dep = `echo "#include <errno.h>" | gcc -E -M -`;
	if ($dep =~ /(\S+errno\.h)/) {
	     push(@file, $1);
	}
    } elsif ($^O eq 'linux' &&
	      $Config{gccversion} ne '' && 
	      $Config{gccversion} !~ /intel/i &&
	      # might be using, say, Intel's icc
	      $linux_errno_h
	     ) {
	push(@file, $linux_errno_h);
    } elsif ($^O eq 'haiku') {
	# hidden in a special place
	push(@file, '/boot/system/develop/headers/posix/errno.h');

    } elsif ($^O eq 'vos') {
	# avoid problem where cpp returns non-POSIX pathnames
	push(@file, '/system/include_library/errno.h');
    } else {
	open(CPPI, '>', 'errno.c') or
	    die "Cannot open errno.c";

        print CPPI "#include <errno.h>\n";
        if ($IsMSWin32) {
            print CPPI qq[#include "../../win32/include/sys/errno2.h"\n];
        }

	close(CPPI);

	# invoke CPP and read the output
	if ($IsMSWin32) {
	    open(CPPO,"$Config{cpprun} $Config{cppflags} errno.c |") or
		die "Cannot run '$Config{cpprun} $Config{cppflags} errno.c'";
	} else {
	    my $cpp = default_cpp();
	    open(CPPO,"$cpp < errno.c |") or
		die "Cannot exec $cpp";
	}

	my $pat = '^#\s*(?:line)?\s*\d+\s+"([^"]+)"';
	while(<CPPO>) {
	    if ($^O eq 'os2' or $IsMSWin32) {
		if (/$pat/o) {
		   my $f = $1;
		   $f =~ s,\\\\,/,g;
		   push(@file, $f);
		}
	    }
	    else {
		push(@file, $1) if /$pat/o;
	    }
	}
	close(CPPO);
    }
    return uniq(@file);
}

# 
#
sub uniq
{
	# At this point List::Util::uniq appears not to be usable so
	# roll our own.
	#
	# Returns a list with unique values, while keeping the order
	#
	return do { my %seen; grep { !$seen{$_}++ } @_ };
}

sub write_errno_pm {
    my $err;

    # quick sanity check

    die "No error definitions found" unless keys %err;

    # create the CPP input

    open(CPPI, '>', 'errno.c') or
	die "Cannot open errno.c";

    print CPPI "#include <errno.h>\n";

    if ($IsMSWin32) {
	print CPPI qq[#include "../../win32/include/sys/errno2.h"\n];
    }
 
    foreach $err (keys %err) {
	print CPPI '"',$err,'" [[',$err,']]',"\n";
    }

    close(CPPI);

    {	# BeOS (support now removed) did not enter this block
    # invoke CPP and read the output

	my $inhibit_linemarkers = '';
	if ($Config{gccversion} =~ /\A(\d+)\./ and $1 >= 5) {
	    # GCC 5.0 interleaves expanded macros with line numbers breaking
	    # each line into multiple lines. RT#123784
	    $inhibit_linemarkers = ' -P';
	}

	if ($^O eq 'VMS') {
	    my $cpp = "$Config{cppstdin} $Config{cppflags}" .
		$inhibit_linemarkers . " $Config{cppminus}";
	    $cpp =~ s/sys\$input//i;
	    open(CPPO,"$cpp  errno.c |") or
		die "Cannot exec $Config{cppstdin}";
	} elsif ($IsMSWin32) {
	    my $cpp = "$Config{cpprun} $Config{cppflags}" .
		$inhibit_linemarkers;
	    open(CPPO,"$cpp errno.c |") or
		die "Cannot run '$cpp errno.c'";
	} else {
	    my $cpp = default_cpp() . $inhibit_linemarkers;
	    open(CPPO,"$cpp < errno.c |")
		or die "Cannot exec $cpp";
	}

	%err = ();

	while(<CPPO>) {
	    my($name,$expr);
	    next unless ($name, $expr) = /"(.*?)"\s*\[\s*\[\s*(.*?)\s*\]\s*\]/;
	    next if $name eq $expr;
	    $expr =~ s/\(?\(\s*[a-z_]\w*\s*\)\(?([^\)]+)\)?\)?/$1/i; # ((type)0xcafebabe) et alia
	    $expr =~ s/\b((?:0x)?[0-9a-f]+)[LU]+\b/$1/gi; # 2147483647L et alia
	    next if $expr =~ m/\b[a-z_]\w*\b/i; # skip expressions containing function names etc
	    if($expr =~ m/^0[xX]/) {
		$err{$name} = hex $expr;
	    }
	    else {
		$err{$name} = eval $expr;
	    }
	    delete $err{$name} unless defined $err{$name};
	}
	close(CPPO);
    }

    # escape $Config{'archname'}
    my $archname = $Config{'archname'};
    $archname =~ s/([@%\$])/\\$1/g;

    # Write Errno.pm

    print <<"EDQ";
# -*- buffer-read-only: t -*-
#
# This file is auto-generated by ext/Errno/Errno_pm.PL.
# ***ANY*** changes here will be lost.
#

package Errno;
use Exporter 'import';
use strict;

EDQ

    # Errno only needs Config to make sure it hasn't changed platforms.
    # If someone set $ENV{PERL_BUILD_EXPAND_CONFIG_VARS} at build time,
    # they've already declared perl doesn't need to worry about this risk.
    if(!$ENV{'PERL_BUILD_EXPAND_CONFIG_VARS'}) {
        print <<"CONFIG_CHECK_END";
use Config;
"\$Config{'archname'}-\$Config{'osvers'}" eq
"$archname-$Config{'osvers'}" or
	die "Errno architecture ($archname-$Config{'osvers'}) does not match executable architecture (\$Config{'archname'}-\$Config{'osvers'})";

CONFIG_CHECK_END
}

    print <<"EDQ";
our \$VERSION = "$VERSION";
\$VERSION = eval \$VERSION;

my %err;

BEGIN {
    %err = (
EDQ

    my @err = sort { $err{$a} <=> $err{$b} || $a cmp $b }
	grep { $err{$_} =~ /-?\d+$/ } keys %err;

    foreach $err (@err) {
	print "\t$err => $err{$err},\n";
    }

print <<'ESQ';
    );
    # Generate proxy constant subroutines for all the values.
    # Well, almost all the values. Unfortunately we can't assume that at this
    # point that our symbol table is empty, as code such as if the parser has
    # seen code such as C<exists &Errno::EINVAL>, it will have created the
    # typeglob.
    # Doing this before defining @EXPORT_OK etc means that even if a platform is
    # crazy enough to define EXPORT_OK as an error constant, everything will
    # still work, because the parser will upgrade the PCS to a real typeglob.
    # We rely on the subroutine definitions below to update the internal caches.
    # Don't use %each, as we don't want a copy of the value.
    foreach my $name (keys %err) {
        if ($Errno::{$name}) {
            # We expect this to be reached fairly rarely, so take an approach
            # which uses the least compile time effort in the common case:
            eval "sub $name() { $err{$name} }; 1" or die $@;
        } else {
            $Errno::{$name} = \$err{$name};
        }
    }
}

our @EXPORT_OK = keys %err;

our %EXPORT_TAGS = (
    POSIX => [qw(
ESQ

    my $k = join(" ", grep { exists $err{$_} } 
	qw(E2BIG EACCES EADDRINUSE EADDRNOTAVAIL EAFNOSUPPORT
	EAGAIN EALREADY EBADF EBUSY ECHILD ECONNABORTED
	ECONNREFUSED ECONNRESET EDEADLK EDESTADDRREQ EDOM EDQUOT
	EEXIST EFAULT EFBIG EHOSTDOWN EHOSTUNREACH EINPROGRESS
	EINTR EINVAL EIO EISCONN EISDIR ELOOP EMFILE EMLINK
	EMSGSIZE ENAMETOOLONG ENETDOWN ENETRESET ENETUNREACH
	ENFILE ENOBUFS ENODEV ENOENT ENOEXEC ENOLCK ENOMEM
	ENOPROTOOPT ENOSPC ENOSYS ENOTBLK ENOTCONN ENOTDIR
	ENOTEMPTY ENOTSOCK ENOTTY ENXIO EOPNOTSUPP EPERM
	EPFNOSUPPORT EPIPE EPROCLIM EPROTONOSUPPORT EPROTOTYPE
	ERANGE EREMOTE ERESTART EROFS ESHUTDOWN ESOCKTNOSUPPORT
	ESPIPE ESRCH ESTALE ETIMEDOUT ETOOMANYREFS ETXTBSY
	EUSERS EWOULDBLOCK EXDEV));

    $k =~ s/(.{50,70})\s/$1\n\t/g;
    print "\t",$k,"\n    )],\n";

    if ($IsMSWin32) {
	print "    WINSOCK => [qw(\n";
	$k = join(" ", grep { /^WSAE/ } sort keys %err);
	$k =~ s/(.{50,70})\s/$1\n\t/g;
	print "\t",$k,"\n    )],\n";
    }

    print ");\n\n";

    print <<'ESQ';
sub TIEHASH { bless \%err }

sub FETCH {
    my (undef, $errname) = @_;
    return "" unless exists $err{$errname};
    my $errno = $err{$errname};
    return $errno == $! ? $errno : 0;
}

sub STORE {
    require Carp;
    Carp::confess("ERRNO hash is read only!");
}

# This is the true return value
*CLEAR = *DELETE = \*STORE; # Typeglob aliasing uses less space

sub NEXTKEY {
    each %err;
}

sub FIRSTKEY {
    my $s = scalar keys %err;	# initialize iterator
    each %err;
}

sub EXISTS {
    my (undef, $errname) = @_;
    exists $err{$errname};
}

sub _tie_it {
    tie %{$_[0]}, __PACKAGE__;
}

__END__

=head1 NAME

Errno - System errno constants

=head1 SYNOPSIS

    use Errno qw(EINTR EIO :POSIX);

=head1 DESCRIPTION

C<Errno> defines and conditionally exports all the error constants
defined in your system F<errno.h> include file. It has a single export
tag, C<:POSIX>, which will export all POSIX defined error numbers.

On Windows, C<Errno> also defines and conditionally exports all the
Winsock error constants defined in your system F<WinError.h> include
file. These are included in a second export tag, C<:WINSOCK>.

C<Errno> also makes C<%!> magic such that each element of C<%!> has a
non-zero value only if C<$!> is set to that value. For example:

    my $fh;
    unless (open($fh, "<", "/fangorn/spouse")) {
        if ($!{ENOENT}) {
            warn "Get a wife!\n";
        } else {
            warn "This path is barred: $!";
        } 
    } 

If a specified constant C<EFOO> does not exist on the system, C<$!{EFOO}>
returns C<"">.  You may use C<exists $!{EFOO}> to check whether the
constant is available on the system.

Perl automatically loads C<Errno> the first time you use C<%!>, so you don't
need an explicit C<use>.

=head1 CAVEATS

Importing a particular constant may not be very portable, because the
import will fail on platforms that do not have that constant.  A more
portable way to set C<$!> to a valid value is to use:

    if (exists &Errno::EFOO) {
        $! = &Errno::EFOO;
    }

=head1 AUTHOR

Graham Barr <gbarr@pobox.com>

=head1 COPYRIGHT

Copyright (c) 1997-8 Graham Barr. All rights reserved.
This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=cut

# ex: set ro:
ESQ

}
