package Archive::Tar::Constant;

use strict;
use warnings;

use vars qw[$VERSION @ISA @EXPORT];

BEGIN {
    require Exporter;

    $VERSION    = '2.40';
    @ISA        = qw[Exporter];

    require Time::Local if $^O eq "MacOS";
}

@EXPORT = Archive::Tar::Constant->_list_consts( __PACKAGE__ );

use constant FILE           => 0;
use constant HARDLINK       => 1;
use constant SYMLINK        => 2;
use constant CHARDEV        => 3;
use constant BLOCKDEV       => 4;
use constant DIR            => 5;
use constant FIFO           => 6;
use constant SOCKET         => 8;
use constant UNKNOWN        => 9;
use constant LONGLINK       => 'L';
use constant LABEL          => 'V';

use constant BUFFER         => 4096;
use constant HEAD           => 512;
use constant BLOCK          => 512;

use constant COMPRESS_GZIP  => 9;
use constant COMPRESS_BZIP  => 'bzip2';
use constant COMPRESS_XZ    => 'xz';

use constant BLOCK_SIZE     => sub { my $n = int($_[0]/BLOCK); $n++ if $_[0] % BLOCK; $n * BLOCK };
use constant TAR_PAD        => sub { my $x = shift || return; return "\0" x (BLOCK - ($x % BLOCK) ) };
use constant TAR_END        => "\0" x BLOCK;

use constant READ_ONLY      => sub { shift() ? 'rb' : 'r' };
use constant WRITE_ONLY     => sub { $_[0] ? 'wb' . shift : 'w' };
use constant MODE_READ      => sub { $_[0] =~ /^r/ ? 1 : 0 };

# Pointless assignment to make -w shut up
my $getpwuid; $getpwuid = 'unknown' unless eval { my $f = getpwuid (0); };
my $getgrgid; $getgrgid = 'unknown' unless eval { my $f = getgrgid (0); };
use constant UNAME          => sub { $getpwuid || scalar getpwuid( shift() ) || '' };
use constant GNAME          => sub { $getgrgid || scalar getgrgid( shift() ) || '' };
use constant UID            => $>;
use constant GID            => (split ' ', $) )[0];

use constant MODE           => do { 0666 & (0777 & ~umask) };
use constant STRIP_MODE     => sub { shift() & 0777 };
use constant CHECK_SUM      => "      ";

use constant UNPACK         => 'a100 a8 a8 a8 a12 a12 a8 a1 a100 A6 a2 a32 a32 a8 a8 a155 x12';	# cdrake - size must be a12 - not A12 - or else screws up huge file sizes (>8gb)
use constant PACK           => 'a100 a8 a8 a8 a12 a12 A8 a1 a100 a6 a2 a32 a32 a8 a8 a155 x12';
use constant NAME_LENGTH    => 100;
use constant PREFIX_LENGTH  => 155;

use constant TIME_OFFSET    => ($^O eq "MacOS") ? Time::Local::timelocal(0,0,0,1,0,1970) : 0;
use constant MAGIC          => "ustar";
use constant TAR_VERSION    => "00";
use constant LONGLINK_NAME  => '././@LongLink';
use constant PAX_HEADER     => 'pax_global_header';

                            ### allow ZLIB to be turned off using ENV: DEBUG only
use constant ZLIB           => do { !$ENV{'PERL5_AT_NO_ZLIB'} and
                                        eval { require IO::Zlib };
                                    $ENV{'PERL5_AT_NO_ZLIB'} || $@ ? 0 : 1
                                };

                            ### allow BZIP to be turned off using ENV: DEBUG only
use constant BZIP           => do { !$ENV{'PERL5_AT_NO_BZIP'} and
                                        eval { require IO::Uncompress::Bunzip2;
                                               require IO::Compress::Bzip2; };
                                    $ENV{'PERL5_AT_NO_BZIP'} || $@ ? 0 : 1
                                };

                            ### allow XZ to be turned off using ENV: DEBUG only
use constant XZ             => do { !$ENV{'PERL5_AT_NO_XZ'} and
                                        eval { require IO::Compress::Xz;
                                               require IO::Uncompress::UnXz; };
                                    $ENV{'PERL5_AT_NO_XZ'} || $@ ? 0 : 1
                                };

use constant GZIP_MAGIC_NUM => qr/^(?:\037\213|\037\235)/;

                           # ASCII:  B   Z   h    0    9
use constant BZIP_MAGIC_NUM => qr/^\x42\x5A\x68[\x30-\x39]/;

use constant XZ_MAGIC_NUM   => qr/^\xFD\x37\x7A\x58\x5A\x00/;

use constant CAN_CHOWN      => sub { ($> == 0 and $^O ne "MacOS" and $^O ne "MSWin32") };
use constant CAN_READLINK   => ($^O ne 'MSWin32' and $^O !~ /RISC(?:[ _])?OS/i and $^O ne 'VMS');
use constant ON_UNIX        => ($^O ne 'MSWin32' and $^O ne 'MacOS' and $^O ne 'VMS');
use constant ON_VMS         => $^O eq 'VMS';

sub _list_consts {
    my $class = shift;
    my $pkg   = shift;
    return unless defined $pkg; # some joker might use '0' as a pkg...

    my @rv;
    {   no strict 'refs';
        my $stash = $pkg . '::';

        for my $name (sort keys %$stash ) {

            ### is it a subentry?
            my $sub = $pkg->can( $name );
            next unless defined $sub;

            next unless defined prototype($sub) and
                     not length prototype($sub);

            push @rv, $name;
        }
    }

    return sort @rv;
}

1;
