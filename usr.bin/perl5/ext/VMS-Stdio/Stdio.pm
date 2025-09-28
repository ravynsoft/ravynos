#   VMS::Stdio - VMS extensions to Perl's stdio calls
#
#   Author:  Charles Bailey  bailey@genetics.upenn.edu
#   Version: 2.2
#   Revised: 19-Jul-1998
#   Docs revised: 13-Oct-1998 Dan Sugalski <sugalskd@ous.edu>

package VMS::Stdio;

require 5.006;
use Carp '&croak';
use DynaLoader ();
use Exporter 'import';

our $VERSION = '2.46';
our @ISA = qw( DynaLoader IO::File );
our @EXPORT = qw( &O_APPEND &O_CREAT &O_EXCL  &O_NDELAY &O_NOWAIT
              &O_RDONLY &O_RDWR  &O_TRUNC &O_WRONLY );
our @EXPORT_OK = qw( &binmode &flush &getname &remove &rewind &sync &setdef &tmpnam
                 &vmsopen &vmssysopen &waitfh &writeof );
our %EXPORT_TAGS = ( CONSTANTS => [ qw( &O_APPEND &O_CREAT &O_EXCL  &O_NDELAY
                                    &O_NOWAIT &O_RDONLY &O_RDWR &O_TRUNC
                                    &O_WRONLY ) ],
                 FUNCTIONS => [ qw( &binmode &flush &getname &remove &rewind
                                    &setdef &sync &tmpnam &vmsopen &vmssysopen
                                    &waitfh &writeof ) ] );

bootstrap VMS::Stdio $VERSION;

sub AUTOLOAD {
    my($constname) = $AUTOLOAD;
    $constname =~ s/.*:://;
    if ($constname =~ /^O_/) {
      my($val) = constant($constname);
      defined $val or croak("Unknown VMS::Stdio constant $constname");
      *$AUTOLOAD = sub { $val; }
    }
    else { # We don't know about it; hand off to IO::File
      require IO::File;

      *$AUTOLOAD = eval "sub { shift->IO::File::$constname(\@_) }";
      croak "Error autoloading IO::File::$constname: $@" if $@;
    }
    goto &$AUTOLOAD;
}

sub DESTROY { close($_[0]); }


1;

__END__

=head1 NAME

VMS::Stdio - standard I/O functions via VMS extensions

=head1 SYNOPSIS

  use VMS::Stdio qw( &flush &getname &remove &rewind &setdef &sync
                     &tmpnam &vmsopen &vmssysopen &waitfh &writeof );
  setdef("new:[default.dir]");
  $uniquename = tmpnam;
  $fh = vmsopen("my.file","rfm=var","alq=100",...) or die $!;
  $name = getname($fh);
  print $fh "Hello, world!\n";
  flush($fh);
  sync($fh);
  rewind($fh);
  $line = <$fh>;
  undef $fh;  # closes file
  $fh = vmssysopen("another.file", O_RDONLY | O_NDELAY, 0, "ctx=bin");
  sysread($fh,$data,128);
  waitfh($fh);
  close($fh);
  remove("another.file");
  writeof($pipefh);
  binmode($fh);

=head1 DESCRIPTION

This package gives Perl scripts access via VMS extensions to several
C stdio operations not available through Perl's CORE I/O functions.
The specific routines are described below.  These functions are
prototyped as unary operators, with the exception of C<vmsopen>
and C<vmssysopen>, which can take any number of arguments, and
C<tmpnam>, which takes none.

All of the routines are available for export, though none are
exported by default.  All of the constants used by C<vmssysopen>
to specify access modes are exported by default.  The routines
are associated with the Exporter tag FUNCTIONS, and the constants
are associated with the Exporter tag CONSTANTS, so you can more
easily choose what you'd like to import:

    # import constants, but not functions
    use VMS::Stdio;  # same as use VMS::Stdio qw( :DEFAULT );
    # import functions, but not constants
    use VMS::Stdio qw( !:CONSTANTS :FUNCTIONS ); 
    # import both
    use VMS::Stdio qw( :CONSTANTS :FUNCTIONS ); 
    # import neither
    use VMS::Stdio ();

Of course, you can also choose to import specific functions by
name, as usual.

This package C<ISA> IO::File, so that you can call IO::File
methods on the handles returned by C<vmsopen> and C<vmssysopen>.
The IO::File package is not initialized, however, until you
actually call a method that VMS::Stdio doesn't provide.  This
is done to save startup time for users who don't wish to use
the IO::File methods.

B<Note:>  In order to conform to naming conventions for Perl
extensions and functions, the name of this package was
changed to from VMS::stdio to VMS::Stdio as of Perl 5.002, and the names of some
routines were changed.  For many releases, calls to the old VMS::stdio routines
would generate a warning, and then route to the equivalent
VMS::Stdio function.  This compatibility interface has now been removed.

=over 4

=item binmode

This function causes the file handle to be reopened with the CRTL's
carriage control processing disabled; its effect is the same as that
of the C<b> access mode in C<vmsopen>.  After the file is reopened,
the file pointer is positioned as close to its position before the
call as possible (I<i.e.> as close as fsetpos() can get it -- for
some record-structured files, it's not possible to return to the
exact byte offset in the file).  Because the file must be reopened,
this function cannot be used on temporary-delete files. C<binmode>
returns true if successful, and C<undef> if not.

Note that the effect of C<binmode> differs from that of the binmode()
function on operating systems such as Windows and MSDOS, and is not
needed to process most types of file.

=item flush

This function causes the contents of stdio buffers for the specified
file handle to be flushed.  If C<undef> is used as the argument to
C<flush>, all currently open file handles are flushed.  Like the CRTL
fflush() routine, it does not flush any underlying RMS buffers for the
file, so the data may not be flushed all the way to the disk.  C<flush>
returns a true value if successful, and C<undef> if not.

=item getname

The C<getname> function returns the file specification associated
with a Perl I/O handle.  If an error occurs, it returns C<undef>.

=item remove

This function deletes the file named in its argument, returning
a true value if successful and C<undef> if not.  It differs from
the CORE Perl function C<unlink> in that it does not try to
reset file protection if the original protection does not give
you delete access to the file (cf. L<perlvms>).  In other words,
C<remove> is equivalent to

  unlink($file) if VMS::Filespec::candelete($file);

=item rewind

C<rewind> resets the current position of the specified file handle
to the beginning of the file.  It's really just a convenience
method equivalent in effect to C<seek($fh,0,0)>.  It returns a
true value if successful, and C<undef> if it fails.

=item setdef

This function sets the default device and directory for the process.
It is identical to the built-in chdir() operator, except that the change
persists after Perl exits.  It returns a true value on success, and
C<undef> if it encounters an error.

=item sync

This function flushes buffered data for the specified file handle
from stdio and RMS buffers all the way to disk.  If successful, it
returns a true value; otherwise, it returns C<undef>.

=item tmpnam

The C<tmpnam> function returns a unique string which can be used
as a filename when creating temporary files.  If, for some
reason, it is unable to generate a name, it returns C<undef>.

=item vmsopen

The C<vmsopen> function enables you to specify optional RMS arguments
to the VMS CRTL when opening a file.  Its operation is similar to the built-in
Perl C<open> function (see L<perlfunc> for a complete description),
but it will only open normal files; it cannot open pipes or duplicate
existing I/O handles.  Up to 8 optional arguments may follow the
file name.  These arguments should be strings which specify
optional file characteristics as allowed by the CRTL. (See the
CRTL reference manual description of creat() and fopen() for details.)
If successful, C<vmsopen> returns a VMS::Stdio file handle; if an
error occurs, it returns C<undef>.

You can use the file handle returned by C<vmsopen> just as you
would any other Perl file handle.  The class VMS::Stdio ISA
IO::File, so you can call IO::File methods using the handle
returned by C<vmsopen>.  However, C<use>ing VMS::Stdio does not
automatically C<use> IO::File; you must do so explicitly in
your program if you want to call IO::File methods.  This is
done to avoid the overhead of initializing the IO::File package
in programs which intend to use the handle returned by C<vmsopen>
as a normal Perl file handle only.  When the scalar containing
a VMS::Stdio file handle is overwritten, C<undef>d, or goes
out of scope, the associated file is closed automatically.

File characteristic options:

=over 2

=item alq=INTEGER

Sets the allocation quantity for this file

=item bls=INTEGER

File blocksize

=item ctx=STRING

Sets the context for the file. Takes one of these arguments:

=over 4

=item bin

Disables LF to CRLF translation

=item cvt

Negates previous setting of C<ctx=noctx>

=item nocvt

Disables conversion of FORTRAN carriage control

=item rec

Force record-mode access

=item stm

Force stream mode

=item xplct

Causes records to be flushed I<only> when the file is closed, or when an
explicit flush is done

=back

=item deq=INTEGER

Sets the default extension quantity

=item dna=FILESPEC

Sets the default filename string. Used to fill in any missing pieces of the
filename passed.

=item fop=STRING

File processing option. Takes one or more of the following (in a
comma-separated list if there's more than one)

=over 4

=item ctg

Contiguous.

=item cbt

Contiguous-best-try.

=item dfw

Deferred write; only applicable to files opened for shared access.

=item dlt

Delete file on close.

=item tef

Truncate at end-of-file.

=item cif

Create if nonexistent.

=item sup

Supersede.

=item scf

Submit as command file on close.

=item spl

Spool to system printer on close.

=item tmd

Temporary delete.

=item tmp

Temporary (no file directory).

=item nef

Not end-of-file.

=item rck

Read check compare operation.

=item wck

Write check compare operation.

=item mxv

Maximize version number.

=item rwo

Rewind file on open.

=item pos

Current position.

=item rwc

Rewind file on close.

=item sqo

File can only be processed in a sequential manner.

=back

=item fsz=INTEGER

Fixed header size

=item gbc=INTEGER

Global buffers requested for the file

=item mbc=INTEGER

Multiblock count

=item mbf=INTEGER

Bultibuffer count

=item mrs=INTEGER

Maximum record size

=item rat=STRING

File record attributes. Takes one of the following:

=over 4

=item cr

Carriage-return control.

=item blk

Disallow records to span block boundaries.

=item ftn

FORTRAN print control.

=item none

Explicitly forces no carriage control.

=item prn

Print file format.

=back

=item rfm=STRING

File record format. Takes one of the following:

=over 4

=item fix

Fixed-length record format.

=item stm

RMS stream record format.

=item stmlf

Stream format with line-feed terminator.

=item stmcr

Stream format with carriage-return terminator.

=item var

Variable-length record format.

=item vfc

Variable-length record with fixed control.

=item udf

Undefined format

=back

=item rop=STRING

Record processing operations. Takes one or more of the following in a
comma-separated list:

=over 4

=item asy

Asynchronous I/O.

=item cco

Cancel Ctrl/O (used with Terminal I/O).

=item cvt

Capitalizes characters on a read from the terminal.

=item eof

Positions the record stream to the end-of-file for the connect operation
only.

=item nlk

Do not lock record.

=item pmt

Enables use of the prompt specified by pmt=usr-prmpt on input from the
terminal.

=item pta

Eliminates any information in the type-ahead buffer on a read from the
terminal.

=item rea

Locks record for a read operation for this process, while allowing other
accessors to read the record.

=item rlk

Locks record for write.

=item rne

Suppresses echoing of input data on the screen as it is entered on the
keyboard.

=item rnf

Indicates that Ctrl/U, Ctrl/R, and DELETE are not to be considered control
commands on terminal input, but are to be passed to the application
program.

=item rrl

Reads regardless of lock.

=item syncsts

Returns success status of RMS$_SYNCH if the requested service completes its
task immediately.

=item tmo

Timeout I/O.

=item tpt

Allows put/write services using sequential record access mode to occur at
any point in the file, truncating the file at that point.

=item ulk

Prohibits RMS from automatically unlocking records.

=item wat

Wait until record is available, if currently locked by another stream.

=item rah

Read ahead.

=item wbh

Write behind.

=back

=item rtv=INTEGER

The number of retrieval pointers that RMS has to maintain (0 to 127255)

=item shr=STRING

File sharing options. Choose one of the following:

=over 4

=item del

Allows users to delete.

=item get

Allows users to read.

=item mse

Allows mainstream access.

=item nil

Prohibits file sharing.

=item put

Allows users to write.

=item upd

Allows users to update.

=item upi

Allows one or more writers.

=back

=item tmo=INTEGER

I/O timeout value

=back

=item vmssysopen

This function bears the same relationship to the CORE function
C<sysopen> as C<vmsopen> does to C<open>.  Its first three arguments
are the name, access flags, and permissions for the file.  Like
C<vmsopen>, it takes up to 8 additional string arguments which
specify file characteristics.  Its return value is identical to
that of C<vmsopen>.

The symbolic constants for the mode argument are exported by
VMS::Stdio by default, and are also exported by the Fcntl package.

=item waitfh

This function causes Perl to wait for the completion of an I/O
operation on the file handle specified as its argument.  It is
used with handles opened for asynchronous I/O, and performs its
task by calling the CRTL routine fwait().

=item writeof

This function writes an EOF to a file handle, if the device driver
supports this operation.  Its primary use is to send an EOF to a
subprocess through a pipe opened for writing without closing the
pipe.  It returns a true value if successful, and C<undef> if
it encounters an error.

=back

=head1 REVISION

This document was last revised on 13-Oct-1998, for Perl 5.004, 5.005, and
5.6.0.

=cut
