# GDBM_File.pm -- Perl 5 interface to GNU gdbm library.

=head1 NAME

GDBM_File - Perl5 access to the gdbm library.

=head1 SYNOPSIS

    use GDBM_File;
    [$db =] tie %hash, 'GDBM_File', $filename, GDBM_WRCREAT, 0640
                or die "$GDBM_File::gdbm_errno";
    # Use the %hash...

    $e = $db->errno;
    $e = $db->syserrno;
    $str = $db->strerror;
    $bool = $db->needs_recovery;

    $db->clear_error;

    $db->reorganize;
    $db->sync;

    $n = $db->count;

    $n = $db->flags;

    $str = $db->dbname;

    $db->cache_size;
    $db->cache_size($newsize);

    $n = $db->block_size;

    $bool = $db->sync_mode;
    $db->sync_mode($bool);

    $bool = $db->centfree;
    $db->centfree($bool);

    $bool = $db->coalesce;
    $db->coalesce($bool);

    $bool = $db->mmap;

    $size = $db->mmapsize;
    $db->mmapsize($newsize);

    $db->recover(%args);

    untie %hash ;

=head1 DESCRIPTION

B<GDBM_File> is a module which allows Perl programs to make use of the
facilities provided by the GNU gdbm library.  If you intend to use this
module you should really have a copy of the B<GDBM manual> at hand.
The manual is avaialble online at
L<https://www.gnu.org.ua/software/gdbm/manual>.

Most of the B<gdbm> functions are available through the B<GDBM_File>
interface.

Unlike Perl's built-in hashes, it is not safe to C<delete> the current
item from a GDBM_File tied hash while iterating over it with C<each>.
This is a limitation of the gdbm library.

=head2 Tie

Use the Perl built-in B<tie> to associate a B<GDBM> database with a Perl
hash:

   tie %hash, 'GDBM_File', $filename, $flags, $mode;

Here, I<$filename> is the name of the database file to open or create.
I<$flags> is a bitwise OR of I<access mode> and optional I<modifiers>.
Access mode is one of:

=over 4

=item B<GDBM_READER>

Open existing database file in read-only mode.

=item B<GDBM_WRITER>

Open existing database file in read-write mode.

=item B<GDBM_WRCREAT>

If the database file exists, open it in read-write mode.  If it doesn't,
create it first and open read-write.

=item B<GDBM_NEWDB>

Create new database and open it read-write.  If the database already exists,
truncate it first.

=back

A number of modifiers can be OR'd to the access mode.  Most of them are
rarely needed (see L<https://www.gnu.org.ua/software/gdbm/manual/Open.html>
for a complete list), but one is worth mentioning.  The B<GDBM_NUMSYNC>
modifier, when used with B<GDBM_NEWDB>, instructs B<GDBM> to create the
database in I<extended> (so called I<numsync>) format.  This format is
best suited for crash-tolerant implementations.  See B<CRASH TOLERANCE>
below for more information.

The I<$mode> parameter is the file mode for creating new database
file.  Use an octal constant or a combination of C<S_I*> constants
from the B<Fcntl> module.  This parameter is used if I<$flags> is
B<GDBM_NEWDB> or B<GDBM_WRCREAT>.

On success, B<tie> returns an object of class B<GDBM_File>.  On failure,
it returns B<undef>.  It is recommended to always check the return value,
to make sure your hash is successfully associated with the database file.
See B<ERROR HANDLING> below for examples.

=head1 STATIC METHODS

=head2 GDBM_version

    $str = GDBM_File->GDBM_version;
    @ar = GDBM_File->GDBM_version;

Returns the version number of the underlying B<libgdbm> library. In scalar
context, returns the library version formatted as string:

    MINOR.MAJOR[.PATCH][ (GUESS)]

where I<MINOR>, I<MAJOR>, and I<PATCH> are version numbers, and I<GUESS> is
a guess level (see below).

In list context, returns a list:

    ( MINOR, MAJOR, PATCH [, GUESS] )

The I<GUESS> component is present only if B<libgdbm> version is 1.8.3 or
earlier. This is because earlier releases of B<libgdbm> did not include
information about their version and the B<GDBM_File> module has to implement
certain guesswork in order to determine it. I<GUESS> is a textual description
in string context, and a positive number indicating how rough the guess is
in list context. Possible values are:

=over 4

=item 1  - exact guess

The major and minor version numbers are guaranteed to be correct. The actual
patchlevel is most probably guessed right, but can be 1-2 less than indicated.

=item 2  - approximate

The major and minor number are guaranteed to be correct. The patchlevel is
set to the upper bound.

=item 3  - rough guess

The version is guaranteed to be not newer than B<I<MAJOR>.I<MINOR>>.

=back

=head1 ERROR HANDLING

=head2 $GDBM_File::gdbm_errno

When referenced in numeric context, retrieves the current value of the
B<gdbm_errno> variable, i.e. a numeric code describing the state of the
most recent operation on any B<gdbm> database.  Each numeric code has a
symbolic name associated with it.   For a comprehensive list  of these, see
L<https://www.gnu.org.ua/software/gdbm/manual/Error-codes.html>.  Notice,
that this list includes all error codes defined for the most recent
version of B<gdbm>.  Depending on the actual version of the library
B<GDBM_File> is built with, some of these may be missing.

In string context, B<$gdbm_errno> returns a human-readable description of
the error.  If necessary, this description includes the value of B<$!>.
This makes it possible to use it in diagnostic messages.  For example,
the usual tying sequence is

    tie %hash, 'GDBM_File', $filename, GDBM_WRCREAT, 0640
         or die "$GDBM_File::gdbm_errno";

The following, more complex, example illustrates how you can fall back
to read-only mode if the database file permissions forbid read-write
access:

    use Errno qw(EACCES);
    unless (tie(%hash, 'GDBM_File', $filename, GDBM_WRCREAT, 0640)) {
        if ($GDBM_File::gdbm_errno == GDBM_FILE_OPEN_ERROR
            && $!{EACCES}) {
            if (tie(%hash, 'GDBM_File', $filename, GDBM_READER, 0640)) {
                die "$GDBM_File::gdbm_errno";
            }
        } else {
            die "$GDBM_File::gdbm_errno";
        }
    }

=head2 gdbm_check_syserr

    if (gdbm_check_syserr(gdbm_errno)) ...

Returns true if the system error number (B<$!>) gives more information on
the cause of the error.

=head1 DATABASE METHODS

=head2 close

    $db->close;

Closes the database.  Normally you would just do B<untie>.  However, you
will need to use this function if you have explicitly assigned the result
of B<tie> to a variable, and wish to release the database to another
users.  Consider the following code:

    $db = tie %hash, 'GDBM_File', $filename, GDBM_WRCREAT, 0640;
    # Do something with %hash or $db...
    untie %hash;
    $db->close;

In this example, doing B<untie> alone is not enough, since the database
would remain referenced by B<$db>, and, as a consequence, the database file
would remain locked.  Calling B<$db-E<gt>close> ensures the database file is
closed and unlocked.

=head2 errno

    $db->errno

Returns the last error status associated with this database.  In string
context, returns a human-readable description of the error.  See also
B<$GDBM_File::gdbm_errno> variable above.

=head2 syserrno

    $db->syserrno

Returns the last system error status (C C<errno> variable), associated with
this database,

=head2 strerror

    $db->strerror

Returns textual description of the last error that occurred in this database.

=head2 clear_error

    $db->clear_error

Clear error status.

=head2 needs_recovery

    $db->needs_recovery

Returns true if the database needs recovery.

=head2 reorganize

    $db->reorganize;

Reorganizes the database.

=head2 sync

    $db->sync;

Synchronizes recent changes to the database with its disk copy.

=head2 count

    $n = $db->count;

Returns number of keys in the database.

=head2 flags

    $db->flags;

Returns flags passed as 4th argument to B<tie>.

=head2 dbname

    $db->dbname;

Returns the database name (i.e. 3rd argument to B<tie>.

=head2 cache_size

    $db->cache_size;
    $db->cache_size($newsize);

Returns the size of the internal B<GDBM> cache for that database.

Called with argument, sets the size to I<$newsize>.

=head2 block_size

    $db->block_size;

Returns the block size of the database.

=head2 sync_mode

    $db->sync_mode;
    $db->sync_mode($bool);

Returns the status of the automatic synchronization mode. Called with argument,
enables or disables the sync mode, depending on whether $bool is B<true> or
B<false>.

When synchronization mode is on (B<true>), any changes to the database are
immediately written to the disk. This ensures database consistency in case
of any unforeseen errors (e.g. power failures), at the expense of considerable
slowdown of operation.

Synchronization mode is off by default.

=head2 centfree

    $db->centfree;
    $db->centfree($bool);

Returns status of the central free block pool (B<0> - disabled,
B<1> - enabled).

With argument, changes its status.

By default, central free block pool is disabled.

=head2 coalesce

    $db->coalesce;
    $db->coalesce($bool);

=head2 mmap

    $db->mmap;

Returns true if memory mapping is enabled.

This method will B<croak> if the B<libgdbm> library is complied without
memory mapping support.

=head2 mmapsize

    $db->mmapsize;
    $db->mmapsize($newsize);

If memory mapping is enabled, returns the size of memory mapping. With
argument, sets the size to B<$newsize>.

This method will B<croak> if the B<libgdbm> library is complied without
memory mapping support.

=head2 recover

    $db->recover(%args);

Recovers data from a failed database. B<%args> is optional and can contain
following keys:

=over 4

=item err => sub { ... }

Reference to code for detailed error reporting. Upon encountering an error,
B<recover> will call this sub with a single argument - a description of the
error.

=item backup => \$str

Creates a backup copy of the database before recovery and returns its
filename in B<$str>.

=item max_failed_keys => $n

Maximum allowed number of failed keys. If the actual number becomes equal
to I<$n>, B<recover> aborts and returns error.

=item max_failed_buckets => $n

Maximum allowed number of failed buckets. If the actual number becomes equal
to I<$n>, B<recover> aborts and returns error.

=item max_failures => $n

Maximum allowed number of failures during recovery.

=item stat => \%hash

Return recovery statistics in I<%hash>. Upon return, the following keys will
be present:

=over 8

=item recovered_keys

Number of successfully recovered keys.

=item recovered_buckets

Number of successfully recovered buckets.

=item failed_keys

Number of keys that failed to be retrieved.

=item failed_buckets

Number of buckets that failed to be retrieved.

=back

=back

=head2 convert

    $db->convert($format);

Changes the format of the database file referred to by B<$db>.

Starting from version 1.20, B<gdbm> supports two database file formats:
I<standard> and I<extended>.  The former is the traditional database
format, used by previous B<gdbm> versions.  The I<extended> format contains
additional data and is recommended for use in crash tolerant applications.

L<https://www.gnu.org.ua/software/gdbm/manual/Numsync.html>, for the
discussion of both formats.

The B<$format> argument sets the new desired database format.  It is
B<GDBM_NUMSYNC> to convert the database from standard to extended format, and
B<0> to convert it from extended to standard format.

If the database is already in the requested format, the function returns
success without doing anything.

=head2 dump

    $db->dump($filename, %options)

Creates a dump of the database file in I<$filename>.  Such file can be used
as a backup copy or sent over a wire to recreate the database on another
machine.  To create a database from the dump file, use the B<load> method.

B<GDBM> supports two dump formats: old I<binary> and new I<ascii>.  The
binary format is not portable across architectures and is deprecated.  It
is supported for backward compatibility.  The ascii format is portable and
stores additional meta-data about the file.  It was introduced with the
B<gdbm> version 1.11 and is the preferred dump format.  The B<dump> method
creates ascii dumps by default.

If the named file already exists, the function will refuse to overwrite and
will croak an error.  If it doesn't exist, it will be created with the
mode B<0666> modified by the current B<umask>.

These defaults can be altered using the following I<%options>:

=over 4

=item B<binary> => 1

Create dump in I<binary> format.

=item B<mode> => I<MODE>

Set file mode to I<MODE>.

=item B<overwrite> => 1

Silently overwrite existing files.

=back

=head2 load

    $db->load($filename, %options)

Load the data from the dump file I<$filename> into the database I<$db>.
The file must have been previously created using the B<dump> method.  File
format is recognized automatically.  By default, the function will croak
if the dump contains a key that already exists in the database.  It will
silently ignore the failure to restore database mode and/or ownership.
These defaults can be altered using the following I<%options>:

=over 4

=item B<replace> => 1

Replace existing keys.

=item B<restore_mode> => 0 | 1

If I<0>, don't try to restore the mode of the database file to that stored
in the dump.

=item B<restore_owner> => 0 | 1

If I<0>, don't try to restore the owner of the database file to that stored
in the dump.

=item B<strict_errors> => 1

Croak if failed to restore ownership and/or mode.

=back

The usual sequence to recreate a database from the dump file is:

    my %hash;
    my $db = tie %hash, 'GDBM_File', 'a.db', GDBM_NEWDB, 0640;
    $db->load('a.dump');

=head1 CRASH TOLERANCE

Crash tolerance is a new feature that, given appropriate support from the OS
and the filesystem, guarantees that a logically consistent recent state of the
database can be recovered following a crash, such as power outage, OS kernel
panic, or the like.

Crash tolerance support appeared in B<gdbm> version 1.21.  The theory behind
it is explained in "Crashproofing the Original NoSQL Key-Value Store",
by Terence Kelly (L<https://queue.acm.org/detail.cfm?id=3487353>).  A
detailed discussion of the B<gdbm> implementation is available in the
B<GDBM Manual> (L<https://www.gnu.org.ua/software/gdbm/manual/Crash-Tolerance.html>).  The information below describes the Perl interface.

For maximum robustness, we recommend to use I<extended database format>
for crash tolerant databases.  To create a database in extended format,
use the B<GDBM_NEWDB|GDBM_NUMSYNC> when opening the database, e.g.:

    $db = tie %hash, 'GDBM_File', $filename,
              GDBM_NEWDB|GDBM_NUMSYNC, 0640;

To convert existing database to the extended format, use the B<convert>
method, described above, e.g.:

    $db->convert(GDBM_NUMSYNC);

=head2 crash_tolerance_status

    GDBM_File->crash_tolerance_status;

This static method returns the status of crash tolerance support.  A
non-zero value means crash tolerance is compiled in and supported by
the operating system.

=head2 failure_atomic

    $db->failure_atomic($even, $odd)

Enables crash tolerance for the database B<$db>,  Arguments are
the pathnames of two files that will be created and filled with
I<snapshots> of the database file.  The two files must not exist
when this method is called and must reside on the same filesystem
as the database file.  This filesystem must be support the I<reflink>
operation (https://www.gnu.org.ua/software/gdbm/manual/Filesystems-supporting-crash-tolerance.html>.

After a successful call to B<failure_atomic>, every call to B<$db->sync>
method will make an efficient reflink snapshot of the database file in
one of these files; consecutive calls to B<sync> alternate between the
two, hence the names.

The most recent of these files can be used to recover the database after
a crash.  To select the right snapshot, use the B<latest_snapshot>
static method.

=head2 latest_snapshot

    $file = GDBM_File->latest_snapshot($even, $odd);

    ($file, $error) = GDBM_File->latest_snapshot($even, $odd);

Given the two snapshot names (the ones used previously in a call to
B<failure_atomic>), this method selects the one suitable for database
recovery, i.e. the file which contains the most recent database snapshot.

In scalar context, it returns the selected file name or B<undef> in case
of failure.

In array context, the returns a list of two elements: the file name
and status code.  On success, the file name is defined and the code
is B<GDBM_SNAPSHOT_OK>.  On error, the file name is B<undef>, and
the status is one of the following:

=over 4

=item GDBM_SNAPSHOT_BAD

Neither snapshot file is applicable. This means that the crash has occurred
before a call to B<failure_atomic> completed.  In this case, it is best to
fall back on a safe backup copy of the data file.

=item GDBM_SNAPSHOT_ERR

A system error occurred.  Examine B<$!> for details.  See
<https://www.gnu.org.ua/software/gdbm/manual/Crash-recovery.html> for
a comprehensive list of error codes and their meaning.

=item GDBM_SNAPSHOT_SAME

The file modes and modification dates of both snapshot files are exactly the
same.  This can happen only for databases in standard format.

=item GDBM_SNAPSHOT_SUSPICIOUS

The I<numsync> counters of the two snapshots differ by more than one.  The
most probable reason is programmer's error: the two parameters refer to
snapshots belonging to different database files.

=back

=head1 AVAILABILITY

gdbm is available from any GNU archive.  The master site is
C<ftp.gnu.org>, but you are strongly urged to use one of the many
mirrors.  You can obtain a list of mirror sites from
L<http://www.gnu.org/order/ftp.html>.

=head1 SECURITY AND PORTABILITY

GDBM files are not portable across platforms.  If you wish to transfer
a GDBM file over the wire, dump it to a portable format first.

B<Do not accept GDBM files from untrusted sources.>

Robustness of GDBM against corrupted databases depends highly on its
version.  Versions prior to 1.15 did not implement any validity
checking, so that a corrupted or maliciously crafted database file
could cause perl to crash or even expose a security vulnerability.
Versions between 1.15 and 1.20 were progressively strengthened against
invalid inputs.  Finally, version 1.21 had undergone extensive fuzzy
checking which proved its ability to withstand any kinds of inputs
without crashing.

=head1 SEE ALSO

L<perl(1)>, L<DB_File(3)>, L<perldbmfilter>,
L<gdbm(3)>,
L<https://www.gnu.org.ua/software/gdbm/manual.html>.

=cut

package GDBM_File;

use strict;
use warnings;
our($VERSION, @ISA, @EXPORT);

require Carp;
require Tie::Hash;
use Exporter 'import';
require XSLoader;
@ISA = qw(Tie::Hash);
@EXPORT = qw(
        GDBM_CACHESIZE
        GDBM_CENTFREE
        GDBM_COALESCEBLKS
        GDBM_FAST
        GDBM_FASTMODE
        GDBM_INSERT
        GDBM_NEWDB
        GDBM_NOLOCK
        GDBM_OPENMASK
        GDBM_READER
        GDBM_REPLACE
        GDBM_SYNC
        GDBM_SYNCMODE
        GDBM_WRCREAT
        GDBM_WRITER
        GDBM_NOMMAP
        GDBM_CLOEXEC
        GDBM_BSEXACT
        GDBM_XVERIFY
        GDBM_PREREAD
        GDBM_NUMSYNC
        GDBM_SNAPSHOT_OK
        GDBM_SNAPSHOT_BAD
        GDBM_SNAPSHOT_ERR
        GDBM_SNAPSHOT_SAME
        GDBM_SNAPSHOT_SUSPICIOUS
        GDBM_NO_ERROR
        GDBM_MALLOC_ERROR
        GDBM_BLOCK_SIZE_ERROR
        GDBM_FILE_OPEN_ERROR
        GDBM_FILE_WRITE_ERROR
        GDBM_FILE_SEEK_ERROR
        GDBM_FILE_READ_ERROR
        GDBM_BAD_MAGIC_NUMBER
        GDBM_EMPTY_DATABASE
        GDBM_CANT_BE_READER
        GDBM_CANT_BE_WRITER
        GDBM_READER_CANT_DELETE
        GDBM_READER_CANT_STORE
        GDBM_READER_CANT_REORGANIZE
        GDBM_UNKNOWN_UPDATE
        GDBM_ITEM_NOT_FOUND
        GDBM_REORGANIZE_FAILED
        GDBM_CANNOT_REPLACE
        GDBM_ILLEGAL_DATA
        GDBM_OPT_ALREADY_SET
        GDBM_OPT_ILLEGAL
        GDBM_BYTE_SWAPPED
        GDBM_BAD_FILE_OFFSET
        GDBM_BAD_OPEN_FLAGS
        GDBM_FILE_STAT_ERROR
        GDBM_FILE_EOF
        GDBM_NO_DBNAME
        GDBM_ERR_FILE_OWNER
        GDBM_ERR_FILE_MODE
        GDBM_UNKNOWN_ERROR
        GDBM_NEED_RECOVERY
        GDBM_BACKUP_FAILED
        GDBM_DIR_OVERFLOW
        GDBM_BAD_BUCKET
        GDBM_BAD_HEADER
        GDBM_BAD_AVAIL
        GDBM_BAD_HASH_TABLE
        GDBM_BAD_DIR_ENTRY
        GDBM_FILE_CLOSE_ERROR
        GDBM_FILE_SYNC_ERROR
        GDBM_FILE_TRUNCATE_ERROR
        GDBM_BUCKET_CACHE_CORRUPTED
        GDBM_BAD_HASH_ENTRY
        GDBM_MALFORMED_DATA
        GDBM_OPT_BADVAL
        GDBM_ERR_SNAPSHOT_CLONE
        GDBM_ERR_REALPATH
        GDBM_ERR_USAGE
        gdbm_check_syserr
);

# This module isn't dual life, so no need for dev version numbers.
$VERSION = '1.24';

our $gdbm_errno;

XSLoader::load();

1;
