package File::stat;
use 5.006;

use strict;
use warnings;
use warnings::register;
use Carp;
use constant _IS_CYGWIN => $^O eq "cygwin";

BEGIN { *warnif = \&warnings::warnif }

our(@EXPORT, @EXPORT_OK, %EXPORT_TAGS);

our $VERSION = '1.13';

our @fields;
our ( $st_dev, $st_ino, $st_mode,
    $st_nlink, $st_uid, $st_gid,
    $st_rdev, $st_size,
    $st_atime, $st_mtime, $st_ctime,
    $st_blksize, $st_blocks
);

BEGIN { 
    use Exporter   ();
    @EXPORT      = qw(stat lstat);
    @fields      = qw( $st_dev	   $st_ino    $st_mode 
		       $st_nlink   $st_uid    $st_gid 
		       $st_rdev    $st_size 
		       $st_atime   $st_mtime  $st_ctime 
		       $st_blksize $st_blocks
		    );
    @EXPORT_OK   = ( @fields, "stat_cando" );
    %EXPORT_TAGS = ( FIELDS => [ @fields, @EXPORT ] );
}

use Fcntl qw(S_IRUSR S_IWUSR S_IXUSR);

BEGIN {
    # These constants will croak on use if the platform doesn't define
    # them. It's important to avoid inflicting that on the user.
    no strict 'refs';
    for (qw(suid sgid svtx)) {
        my $val = eval { &{"Fcntl::S_I\U$_"} };
        *{"_$_"} = defined $val ? sub { $_[0] & $val ? 1 : "" } : sub { "" };
    }
    for (qw(SOCK CHR BLK REG DIR LNK)) {
        *{"S_IS$_"} = defined eval { &{"Fcntl::S_IF$_"} }
            ? \&{"Fcntl::S_IS$_"} : sub { "" };
    }
    # FIFO flag and macro don't quite follow the S_IF/S_IS pattern above
    # RT #111638
    *{"S_ISFIFO"} = defined &Fcntl::S_IFIFO
      ? \&Fcntl::S_ISFIFO : sub { "" };
}

# from doio.c
sub _ingroup {
    my ($gid, $eff)   = @_;

    # I am assuming that since VMS doesn't have getgroups(2), $) will
    # always only contain a single entry.
    $^O eq "VMS"    and return $_[0] == $);

    my ($egid, @supp) = split " ", $);
    my ($rgid)        = split " ", $(;

    $gid == ($eff ? $egid : $rgid)  and return 1;
    grep $gid == $_, @supp          and return 1;

    return "";
}

# VMS uses the Unix version of the routine, even though this is very
# suboptimal. VMS has a permissions structure that doesn't really fit
# into struct stat, and unlike on Win32 the normal -X operators respect
# that, but unfortunately by the time we get here we've already lost the
# information we need. It looks to me as though if we were to preserve
# the st_devnam entry of vmsish.h's fake struct stat (which actually
# holds the filename) it might be possible to do this right, but both
# getting that value out of the struct (perl's stat doesn't return it)
# and interpreting it later would require this module to have an XS
# component (at which point we might as well just call Perl_cando and
# have done with it).
    
if (grep $^O eq $_, qw/os2 MSWin32/) {

    # from doio.c
    *cando = sub { ($_[0][2] & $_[1]) ? 1 : "" };
}
else {

    # from doio.c
    *cando = sub {
        my ($s, $mode, $eff) = @_;
        my $uid = $eff ? $> : $<;
        my ($stmode, $stuid, $stgid) = @$s[2,4,5];

        # This code basically assumes that the rwx bits of the mode are
        # the 0777 bits, but so does Perl_cando.

        if (_IS_CYGWIN ? _ingroup(544, $eff) : ($uid == 0 && $^O ne "VMS")) {
            # If we're root on unix
            # not testing for executable status => all file tests are true
            return 1 if !($mode & 0111);
            # testing for executable status =>
            # for a file, any x bit will do
            # for a directory, always true
            return 1 if $stmode & 0111 || S_ISDIR($stmode);
            return "";
        }

        if ($stuid == $uid) {
            $stmode & $mode         and return 1;
        }
        elsif (_ingroup($stgid, $eff)) {
            $stmode & ($mode >> 3)  and return 1;
        }
        else {
            $stmode & ($mode >> 6)  and return 1;
        }
        return "";
    };
}

# alias for those who don't like objects
*stat_cando = \&cando;

my %op = (
    r => sub { cando($_[0], S_IRUSR, 1) },
    w => sub { cando($_[0], S_IWUSR, 1) },
    x => sub { cando($_[0], S_IXUSR, 1) },
    o => sub { $_[0][4] == $>           },

    R => sub { cando($_[0], S_IRUSR, 0) },
    W => sub { cando($_[0], S_IWUSR, 0) },
    X => sub { cando($_[0], S_IXUSR, 0) },
    O => sub { $_[0][4] == $<           },

    e => sub { 1 },
    z => sub { $_[0][7] == 0    },
    s => sub { $_[0][7]         },

    f => sub { S_ISREG ($_[0][2]) },
    d => sub { S_ISDIR ($_[0][2]) },
    l => sub { S_ISLNK ($_[0][2]) },
    p => sub { S_ISFIFO($_[0][2]) },
    S => sub { S_ISSOCK($_[0][2]) },
    b => sub { S_ISBLK ($_[0][2]) },
    c => sub { S_ISCHR ($_[0][2]) },

    u => sub { _suid($_[0][2]) },
    g => sub { _sgid($_[0][2]) },
    k => sub { _svtx($_[0][2]) },

    M => sub { ($^T - $_[0][9] ) / 86400 },
    C => sub { ($^T - $_[0][10]) / 86400 },
    A => sub { ($^T - $_[0][8] ) / 86400 },
);

use constant HINT_FILETEST_ACCESS => 0x00400000;

# we need fallback=>1 or stringifying breaks
use overload 
    fallback => 1,
    -X => sub {
        my ($s, $op) = @_;

        if (index("rwxRWX", $op) >= 0) {
            (caller 0)[8] & HINT_FILETEST_ACCESS
                and warnif("File::stat ignores use filetest 'access'");

            $^O eq "VMS" and warnif("File::stat ignores VMS ACLs");

            # It would be nice to have a warning about using -l on a
            # non-lstat, but that would require an extra member in the
            # object.
        }

        if ($op{$op}) {
            return $op{$op}->($_[0]);
        }
        else {
            croak "-$op is not implemented on a File::stat object";
        }
    };

# Class::Struct forbids use of @ISA
sub import { goto &Exporter::import }

use Class::Struct qw(struct);
struct 'File::stat' => [
     map { $_ => '$' } qw{
	 dev ino mode nlink uid gid rdev size
	 atime mtime ctime blksize blocks
     }
];

sub populate (@) {
    return unless @_;
    my $stob = new();
    @$stob = (
	$st_dev, $st_ino, $st_mode, $st_nlink, $st_uid, $st_gid, $st_rdev,
        $st_size, $st_atime, $st_mtime, $st_ctime, $st_blksize, $st_blocks ) 
	    = @_;
    return $stob;
} 

sub lstat ($)  { populate(CORE::lstat(shift)) }

sub stat ($) {
    my $arg = shift;
    my $st = populate(CORE::stat $arg);
    return $st if defined $st;
	my $fh;
    {
		local $!;
		no strict 'refs';
		require Symbol;
		$fh = \*{ Symbol::qualify( $arg, caller() )};
		return unless defined fileno $fh;
	}
    return populate(CORE::stat $fh);
}

1;
__END__

=head1 NAME

File::stat - by-name interface to Perl's built-in stat() functions

=head1 SYNOPSIS

 use File::stat;
 my $st = stat($file) or die "No $file: $!";
 if ( ($st->mode & 0111) && ($st->nlink > 1) ) {
     print "$file is executable with lotsa links\n";
 } 

 if ( -x $st ) {
     print "$file is executable\n";
 }

 use Fcntl "S_IRUSR";
 if ( $st->cando(S_IRUSR, 1) ) {
     print "My effective uid can read $file\n";
 }

 use File::stat qw(:FIELDS);
 stat($file) or die "No $file: $!";
 if ( ($st_mode & 0111) && ($st_nlink > 1) ) {
     print "$file is executable with lotsa links\n";
 } 

=head1 DESCRIPTION

This module's default exports override the core stat() 
and lstat() functions, replacing them with versions that return 
"File::stat" objects.  This object has methods that
return the similarly named structure field name from the
stat(2) function; namely,
dev,
ino,
mode,
nlink,
uid,
gid,
rdev,
size,
atime,
mtime,
ctime,
blksize,
and
blocks.  

As of version 1.02 (provided with perl 5.12) the object provides C<"-X">
overloading, so you can call filetest operators (C<-f>, C<-x>, and so
on) on it. It also provides a C<< ->cando >> method, called like

 $st->cando( ACCESS, EFFECTIVE )

where I<ACCESS> is one of C<S_IRUSR>, C<S_IWUSR> or C<S_IXUSR> from the
L<Fcntl|Fcntl> module, and I<EFFECTIVE> indicates whether to use
effective (true) or real (false) ids. The method interprets the C<mode>,
C<uid> and C<gid> fields, and returns whether or not the current process
would be allowed the specified access.

If you don't want to use the objects, you may import the C<< ->cando >>
method into your namespace as a regular function called C<stat_cando>.
This takes an arrayref containing the return values of C<stat> or
C<lstat> as its first argument, and interprets it for you.

You may also import all the structure fields directly into your namespace
as regular variables using the :FIELDS import tag.  (Note that this still
overrides your stat() and lstat() functions.)  Access these fields as
variables named with a preceding C<st_> in front their method names.
Thus, C<$stat_obj-E<gt>dev()> corresponds to $st_dev if you import
the fields.

To access this functionality without the core overrides,
pass the C<use> an empty import list, and then access
function functions with their full qualified names.
On the other hand, the built-ins are still available
via the C<CORE::> pseudo-package.

=head1 BUGS

As of Perl 5.8.0 after using this module you cannot use the implicit
C<$_> or the special filehandle C<_> with stat() or lstat(), trying
to do so leads into strange errors.  The workaround is for C<$_> to
be explicit

    my $stat_obj = stat $_;

and for C<_> to explicitly populate the object using the unexported
and undocumented populate() function with CORE::stat():

    my $stat_obj = File::stat::populate(CORE::stat(_));

=head1 ERRORS

=over 4

=item -%s is not implemented on a File::stat object

The filetest operators C<-t>, C<-T> and C<-B> are not implemented, as
they require more information than just a stat buffer.

=back

=head1 WARNINGS

These can all be disabled with

    no warnings "File::stat";

=over 4

=item File::stat ignores use filetest 'access'

You have tried to use one of the C<-rwxRWX> filetests with C<use
filetest 'access'> in effect. C<File::stat> will ignore the pragma, and
just use the information in the C<mode> member as usual.

=item File::stat ignores VMS ACLs

VMS systems have a permissions structure that cannot be completely
represented in a stat buffer, and unlike on other systems the builtin
filetest operators respect this. The C<File::stat> overloads, however,
do not, since the information required is not available.

=back

=head1 NOTE

While this class is currently implemented using the Class::Struct
module to build a struct-like class, you shouldn't rely upon this.

=head1 AUTHOR

Tom Christiansen
