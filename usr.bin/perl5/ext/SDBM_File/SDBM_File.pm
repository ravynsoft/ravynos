package SDBM_File;

use strict;
use warnings;

require Tie::Hash;
require XSLoader;

our @ISA = qw(Tie::Hash);
our $VERSION = "1.17";

our @EXPORT_OK = qw(PAGFEXT DIRFEXT PAIRMAX);
use Exporter "import";

XSLoader::load();

1;

__END__

=head1 NAME

SDBM_File - Tied access to sdbm files

=head1 SYNOPSIS

 use Fcntl;   # For O_RDWR, O_CREAT, etc.
 use SDBM_File;

 tie(%h, 'SDBM_File', 'filename', O_RDWR|O_CREAT, 0666)
   or die "Couldn't tie SDBM file 'filename': $!; aborting";

 # Now read and change the hash
 $h{newkey} = newvalue;
 print $h{oldkey}; 
 ...

 untie %h;

=head1 DESCRIPTION

C<SDBM_File> establishes a connection between a Perl hash variable and
a file in SDBM_File format.  You can manipulate the data in the file
just as if it were in a Perl hash, but when your program exits, the
data will remain in the file, to be used the next time your program
runs.

=head2 Tie

Use C<SDBM_File> with the Perl built-in C<tie> function to establish
the connection between the variable and the file.

    tie %hash, 'SDBM_File', $basename, $modeflags, $perms;

    tie %hash, 'SDBM_File', $dirfile,  $modeflags, $perms, $pagfilename;

C<$basename> is the base filename for the database.  The database is two
files with ".dir" and ".pag" extensions appended to C<$basename>,

    $basename.dir     (or .sdbm_dir on VMS, per DIRFEXT constant)
    $basename.pag

The two filenames can also be given separately in full as C<$dirfile>
and C<$pagfilename>.  This suits for two files without ".dir" and ".pag"
extensions, perhaps for example two files from L<File::Temp>.

C<$modeflags> can be the following constants from the C<Fcntl> module (in
the style of the L<open(2)> system call),

    O_RDONLY          read-only access
    O_WRONLY          write-only access
    O_RDWR            read and write access

If you want to create the file if it does not already exist then bitwise-OR
(C<|>) C<O_CREAT> too.  If you omit C<O_CREAT> and the database does not
already exist then the C<tie> call will fail.

    O_CREAT           create database if doesn't already exist

C<$perms> is the file permissions bits to use if new database files are
created.  This parameter is mandatory even when not creating a new database.
The permissions will be reduced by the user's umask so the usual value here
would be 0666, or if some very private data then 0600.  (See
L<perlfunc/umask>.)

=head1 EXPORTS

SDBM_File optionally exports the following constants:

=over

=item *

C<PAGFEXT> - the extension used for the page file, usually C<.pag>.

=item *

C<DIRFEXT> - the extension used for the directory file, C<.dir>
everywhere but VMS, where it is C<.sdbm_dir>.

=item *

C<PAIRMAX> - the maximum size of a stored hash entry, including the
length of both the key and value.

=back

These constants can also be used with fully qualified names,
eg. C<SDBM_File::PAGFEXT>.

=head1 DIAGNOSTICS

On failure, the C<tie> call returns an undefined value and probably
sets C<$!> to contain the reason the file could not be tied.

=head2 C<sdbm store returned -1, errno 22, key "..." at ...>

This warning is emitted when you try to store a key or a value that
is too long.  It means that the change was not recorded in the
database.  See BUGS AND WARNINGS below.

=head1 SECURITY WARNING

B<Do not accept SDBM files from untrusted sources!>

The sdbm file format was designed for speed and convenience, not for
portability or security.  A maliciously crafted file might cause perl to
crash or even expose a security vulnerability.

=head1 BUGS AND WARNINGS

There are a number of limits on the size of the data that you can
store in the SDBM file.  The most important is that the length of a
key, plus the length of its associated value, may not exceed 1008
bytes.

See L<perlfunc/tie>, L<perldbmfilter>, L<Fcntl>

=cut
