# File/Copy.pm. Written in 1994 by Aaron Sherman <ajs@ajs.com>. This
# source code has been placed in the public domain by the author.
# Please be kind and preserve the documentation.
#
# Additions copyright 1996 by Charles Bailey.  Permission is granted
# to distribute the revised code under the same terms as Perl itself.

package File::Copy;

use 5.035007;
use strict;
use warnings; no warnings 'newline';
no warnings 'experimental::builtin';
use builtin 'blessed';
use overload;
use File::Spec;
use Config;
# We want HiRes stat and utime if available
BEGIN { eval q{ use Time::HiRes qw( stat utime ) } };
our(@ISA, @EXPORT, @EXPORT_OK, $VERSION, $Too_Big, $Syscopy_is_copy);
sub copy;
sub syscopy;
sub cp;
sub mv;

$VERSION = '2.41';

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(copy move);
@EXPORT_OK = qw(cp mv);

$Too_Big = 1024 * 1024 * 2;

sub croak {
    require Carp;
    goto &Carp::croak;
}

sub carp {
    require Carp;
    goto &Carp::carp;
}

sub _catname {
    my($from, $to) = @_;
    if (not defined &basename) {
        require File::Basename;
        File::Basename->import( 'basename' );
    }

    return File::Spec->catfile($to, basename($from));
}

# _eq($from, $to) tells whether $from and $to are identical
sub _eq {
    my ($from, $to) = map {
        blessed($_) && overload::Method($_, q{""})
            ? "$_"
            : $_
    } (@_);
    return '' if ( (ref $from) xor (ref $to) );
    return $from == $to if ref $from;
    return $from eq $to;
}

sub copy {
    croak("Usage: copy(FROM, TO [, BUFFERSIZE]) ")
      unless(@_ == 2 || @_ == 3);

    my $from = shift;
    my $to = shift;

    my $size;
    if (@_) {
	$size = shift(@_) + 0;
	croak("Bad buffer size for copy: $size\n") unless ($size > 0);
    }

    my $from_a_handle = (ref($from)
			 ? (ref($from) eq 'GLOB'
			    || UNIVERSAL::isa($from, 'GLOB')
                            || UNIVERSAL::isa($from, 'IO::Handle'))
			 : (ref(\$from) eq 'GLOB'));
    my $to_a_handle =   (ref($to)
			 ? (ref($to) eq 'GLOB'
			    || UNIVERSAL::isa($to, 'GLOB')
                            || UNIVERSAL::isa($to, 'IO::Handle'))
			 : (ref(\$to) eq 'GLOB'));

    if (_eq($from, $to)) { # works for references, too
	carp("'$from' and '$to' are identical (not copied)");
        return 0;
    }

    if (!$from_a_handle && !$to_a_handle && -d $to && ! -d $from) {
	$to = _catname($from, $to);
    }

    if ((($Config{d_symlink} && $Config{d_readlink}) || $Config{d_link}) &&
	!($^O eq 'os2')) {
	my @fs = stat($from);
	if (@fs) {
	    my @ts = stat($to);
	    if (@ts && $fs[0] == $ts[0] && $fs[1] eq $ts[1] && !-p $from) {
		carp("'$from' and '$to' are identical (not copied)");
                return 0;
	    }
	}
    }
    elsif (_eq($from, $to)) {
	carp("'$from' and '$to' are identical (not copied)");
	return 0;
    }

    if (defined &syscopy && !$Syscopy_is_copy
	&& !$to_a_handle
	&& !($from_a_handle && $^O eq 'os2' )	# OS/2 cannot handle handles
	&& !($from_a_handle && $^O eq 'MSWin32')
       )
    {
        if ($^O eq 'VMS' && -e $from
            && ! -d $to && ! -d $from) {

            # VMS natively inherits path components from the source of a
            # copy, but we want the Unixy behavior of inheriting from
            # the current working directory.  Also, default in a trailing
            # dot for null file types.

            $to = VMS::Filespec::rmsexpand(VMS::Filespec::vmsify($to), '.');

            # Get rid of the old versions to be like UNIX
            1 while unlink $to;
        }

        return syscopy($from, $to) || 0;
    }

    my $closefrom = 0;
    my $closeto = 0;
    my ($status, $r, $buf);
    local($\) = '';

    my $from_h;
    if ($from_a_handle) {
       $from_h = $from;
    } else {
       open $from_h, "<", $from or goto fail_open1;
       binmode $from_h or die "($!,$^E)";
       $closefrom = 1;
    }

    # Seems most logical to do this here, in case future changes would want to
    # make this croak for some reason.
    unless (defined $size) {
	$size = tied(*$from_h) ? 0 : -s $from_h || 0;
	$size = 1024 if ($size < 512);
	$size = $Too_Big if ($size > $Too_Big);
    }

    my $to_h;
    if ($to_a_handle) {
       $to_h = $to;
    } else {
	$to_h = \do { local *FH }; # XXX is this line obsolete?
	open $to_h, ">", $to or goto fail_open2;
	binmode $to_h or die "($!,$^E)";
	$closeto = 1;
    }

    $! = 0;
    for (;;) {
	my ($r, $w, $t);
       defined($r = sysread($from_h, $buf, $size))
	    or goto fail_inner;
	last unless $r;
	for ($w = 0; $w < $r; $w += $t) {
           $t = syswrite($to_h, $buf, $r - $w, $w)
		or goto fail_inner;
	}
    }

    close($to_h) || goto fail_open2 if $closeto;
    close($from_h) || goto fail_open1 if $closefrom;

    # Use this idiom to avoid uninitialized value warning.
    return 1;

    # All of these contortions try to preserve error messages...
  fail_inner:
    if ($closeto) {
	$status = $!;
	$! = 0;
       close $to_h;
	$! = $status unless $!;
    }
  fail_open2:
    if ($closefrom) {
	$status = $!;
	$! = 0;
       close $from_h;
	$! = $status unless $!;
    }
  fail_open1:
    return 0;
}

sub cp {
    my($from,$to) = @_;
    my(@fromstat) = stat $from;
    my(@tostat) = stat $to;
    my $perm;

    return 0 unless copy(@_) and @fromstat;

    if (@tostat) {
        $perm = $tostat[2];
    } else {
        $perm = $fromstat[2] & ~(umask || 0);
	@tostat = stat $to;
    }
    # Might be more robust to look for S_I* in Fcntl, but we're
    # trying to avoid dependence on any XS-containing modules,
    # since File::Copy is used during the Perl build.
    $perm &= 07777;
    if ($perm & 06000) {
	croak("Unable to check setuid/setgid permissions for $to: $!")
	    unless @tostat;

	if ($perm & 04000 and                     # setuid
	    $fromstat[4] != $tostat[4]) {         # owner must match
	    $perm &= ~06000;
	}

	if ($perm & 02000 && $> != 0) {           # if not root, setgid
	    my $ok = $fromstat[5] == $tostat[5];  # group must match
	    if ($ok) {                            # and we must be in group
                $ok = grep { $_ == $fromstat[5] } split /\s+/, $)
	    }
	    $perm &= ~06000 unless $ok;
	}
    }
    return 0 unless @tostat;
    return 1 if $perm == ($tostat[2] & 07777);
    return eval { chmod $perm, $to; } ? 1 : 0;
}

sub _move {
    croak("Usage: move(FROM, TO) ") unless @_ == 3;

    my($from,$to,$fallback) = @_;

    my($fromsz,$tosz1,$tomt1,$tosz2,$tomt2,$sts,$ossts);

    if (-d $to && ! -d $from) {
	$to = _catname($from, $to);
    }

    ($tosz1,$tomt1) = (stat($to))[7,9];
    $fromsz = -s $from;
    if ($^O eq 'os2' and defined $tosz1 and defined $fromsz) {
      # will not rename with overwrite
      unlink $to;
    }

    if ($^O eq 'VMS' && -e $from
        && ! -d $to && ! -d $from) {

            # VMS natively inherits path components from the source of a
            # copy, but we want the Unixy behavior of inheriting from
            # the current working directory.  Also, default in a trailing
            # dot for null file types.

            $to = VMS::Filespec::rmsexpand(VMS::Filespec::vmsify($to), '.');

            # Get rid of the old versions to be like UNIX
            1 while unlink $to;
    }

    return 1 if rename $from, $to;

    # Did rename return an error even though it succeeded, because $to
    # is on a remote NFS file system, and NFS lost the server's ack?
    return 1 if defined($fromsz) && !-e $from &&           # $from disappeared
                (($tosz2,$tomt2) = (stat($to))[7,9]) &&    # $to's there
                  ((!defined $tosz1) ||			   #  not before or
		   ($tosz1 != $tosz2 or $tomt1 != $tomt2)) &&  #   was changed
                $tosz2 == $fromsz;                         # it's all there

    ($tosz1,$tomt1) = (stat($to))[7,9];  # just in case rename did something

    {
        local $@;
        eval {
            local $SIG{__DIE__};
            $fallback->($from,$to) or die;
            my($atime, $mtime) = (stat($from))[8,9];
            utime($atime, $mtime, $to);
            unlink($from)   or die;
        };
        return 1 unless $@;
    }
    ($sts,$ossts) = ($! + 0, $^E + 0);

    ($tosz2,$tomt2) = ((stat($to))[7,9],0,0) if defined $tomt1;
    unlink($to) if !defined($tomt1) or $tomt1 != $tomt2 or $tosz1 != $tosz2;
    ($!,$^E) = ($sts,$ossts);
    return 0;
}

sub move { _move(@_,\&copy); }
sub mv   { _move(@_,\&cp);   }

# &syscopy is an XSUB under OS/2
unless (defined &syscopy) {
    if ($^O eq 'VMS') {
	*syscopy = \&rmscopy;
    } elsif ($^O eq 'MSWin32' && defined &DynaLoader::boot_DynaLoader) {
	# Win32::CopyFile() fill only work if we can load Win32.xs
	*syscopy = sub {
	    return 0 unless @_ == 2;
	    return Win32::CopyFile(@_, 1);
	};
    } else {
	$Syscopy_is_copy = 1;
	*syscopy = \&copy;
    }
}

1;

__END__

=head1 NAME

File::Copy - Copy files or filehandles

=head1 SYNOPSIS

	use File::Copy;

	copy("sourcefile", "destinationfile") or die "Copy failed: $!";
	copy("Copy.pm", \*STDOUT);
	move("/dev1/sourcefile", "/dev2/destinationfile");

	use File::Copy "cp";

	my $n = FileHandle->new("/a/file", "r");
	cp($n, "x");

=head1 DESCRIPTION

The File::Copy module provides two basic functions, C<copy> and
C<move>, which are useful for getting the contents of a file from
one place to another.

=over 4

=item copy
X<copy> X<cp>

The C<copy> function takes two
parameters: a file to copy from and a file to copy to. Either
argument may be a string, a FileHandle reference or a FileHandle
glob. Obviously, if the first argument is a filehandle of some
sort, it will be read from, and if it is a file I<name> it will
be opened for reading. Likewise, the second argument will be
written to. If the second argument does not exist but the parent
directory does exist, then it will be created. Trying to copy
a file into a non-existent directory is an error.
Trying to copy a file on top of itself is also an error.
C<copy> will not overwrite read-only files.

If the destination (second argument) already exists and is a directory,
and the source (first argument) is not a filehandle, then the source
file will be copied into the directory specified by the destination,
using the same base name as the source file.  It's a failure to have a
filehandle as the source when the destination is a directory.

B<Note that passing in
files as handles instead of names may lead to loss of information
on some operating systems; it is recommended that you use file
names whenever possible.>  Files are opened in binary mode where
applicable.  To get a consistent behaviour when copying from a
filehandle to a file, use C<binmode> on the filehandle.

An optional third parameter can be used to specify the buffer
size used for copying. This is the number of bytes from the
first file, that will be held in memory at any given time, before
being written to the second file. The default buffer size depends
upon the file, but will generally be the whole file (up to 2MB), or
1k for filehandles that do not reference files (eg. sockets).

You may use the syntax C<use File::Copy "cp"> to get at the C<cp>
alias for this function. The syntax is I<exactly> the same.  The
behavior is nearly the same as well: as of version 2.15, C<cp> will
preserve the source file's permission bits like the shell utility
C<cp(1)> would do with default options, while C<copy> uses the default
permissions for the target file (which may depend on the process'
C<umask>, file ownership, inherited ACLs, etc.).  That is, if the
destination file already exists, C<cp> will leave its permissions
unchanged; otherwise the permissions are taken from the source file
and modified by the C<umask>.  If an error occurs in setting
permissions, C<cp> will return 0, regardless of whether the file was
successfully copied.

=item move
X<move> X<mv> X<rename>

The C<move> function also takes two parameters: the current name
and the intended name of the file to be moved.  If the destination
already exists and is a directory, and the source is not a
directory, then the source file will be renamed into the directory
specified by the destination.

If possible, move() will simply rename the file.  Otherwise, it copies
the file to the new location and deletes the original.  If an error occurs
during this copy-and-delete process, you may be left with a (possibly partial)
copy of the file under the destination name.

You may use the C<mv> alias for this function in the same way that
you may use the C<cp> alias for C<copy>.

=item syscopy
X<syscopy>

File::Copy also provides the C<syscopy> routine, which copies the
file specified in the first parameter to the file specified in the
second parameter, preserving OS-specific attributes and file
structure.  For Unix systems, this is equivalent to the simple
C<copy> routine, which doesn't preserve OS-specific attributes.  For
VMS systems, this calls the C<rmscopy> routine (see below).  For OS/2
systems, this calls the C<syscopy> XSUB directly. For Win32 systems,
this calls C<Win32::CopyFile>.

B<Special behaviour if C<syscopy> is defined (OS/2, VMS and Win32)>:

If both arguments to C<copy> are not file handles,
then C<copy> will perform a "system copy" of
the input file to a new output file, in order to preserve file
attributes, indexed file structure, I<etc.>  The buffer size
parameter is ignored.  If either argument to C<copy> is a
handle to an opened file, then data is copied using Perl
operators, and no effort is made to preserve file attributes
or record structure.

The system copy routine may also be called directly under VMS and OS/2
as C<File::Copy::syscopy> (or under VMS as C<File::Copy::rmscopy>, which
is the routine that does the actual work for syscopy).

=item rmscopy($from,$to[,$date_flag])
X<rmscopy>

The first and second arguments may be strings, typeglobs, typeglob
references, or objects inheriting from IO::Handle;
they are used in all cases to obtain the
I<filespec> of the input and output files, respectively.  The
name and type of the input file are used as defaults for the
output file, if necessary.

A new version of the output file is always created, which
inherits the structure and RMS attributes of the input file,
except for owner and protections (and possibly timestamps;
see below).  All data from the input file is copied to the
output file; if either of the first two parameters to C<rmscopy>
is a file handle, its position is unchanged.  (Note that this
means a file handle pointing to the output file will be
associated with an old version of that file after C<rmscopy>
returns, not the newly created version.)

The third parameter is an integer flag, which tells C<rmscopy>
how to handle timestamps.  If it is E<lt> 0, none of the input file's
timestamps are propagated to the output file.  If it is E<gt> 0, then
it is interpreted as a bitmask: if bit 0 (the LSB) is set, then
timestamps other than the revision date are propagated; if bit 1
is set, the revision date is propagated.  If the third parameter
to C<rmscopy> is 0, then it behaves much like the DCL COPY command:
if the name or type of the output file was explicitly specified,
then no timestamps are propagated, but if they were taken implicitly
from the input filespec, then all timestamps other than the
revision date are propagated.  If this parameter is not supplied,
it defaults to 0.

C<rmscopy> is VMS specific and cannot be exported; it must be
referenced by its full name, e.g.:

  File::Copy::rmscopy($from, $to) or die $!;

Like C<copy>, C<rmscopy> returns 1 on success.  If an error occurs,
it sets C<$!>, deletes the output file, and returns 0.

=back

=head1 RETURN

All functions return 1 on success, 0 on failure.
$! will be set if an error was encountered.

=head1 NOTES

Before calling copy() or move() on a filehandle, the caller should
close or flush() the file to avoid writes being lost. Note that this
is the case even for move(), because it may actually copy the file,
depending on the OS-specific implementation, and the underlying
filesystem(s).

=head1 AUTHOR

File::Copy was written by Aaron Sherman I<E<lt>ajs@ajs.comE<gt>> in 1995,
and updated by Charles Bailey I<E<lt>bailey@newman.upenn.eduE<gt>> in 1996.

=cut

