package IO::Uncompress::Adapter::Identity;

use warnings;
use strict;
use bytes;

use IO::Compress::Base::Common  2.204 qw(:Status);
use IO::Compress::Zip::Constants ;

our ($VERSION);

$VERSION = '2.204';

use Compress::Raw::Zlib  2.204 ();

sub mkUncompObject
{
    my $streaming = shift;
    my $zip64 = shift;

    my $crc32 = 1; #shift ;
    my $adler32 = shift;

    bless { 'CompSize'   => U64->new(), # 0,
            'UnCompSize' => 0,
            'wantCRC32'  => $crc32,
            'CRC32'      => Compress::Raw::Zlib::crc32(''),
            'wantADLER32'=> $adler32,
            'ADLER32'    => Compress::Raw::Zlib::adler32(''),
            'ConsumesInput' => 1,
            'Streaming'  => $streaming,
            'Zip64'      => $zip64,
            'DataHdrSize'  => $zip64 ? 24 :  16,
            'Pending'   => '',

          } ;
}


sub uncompr
{
    my $self = shift;
    my $in = $_[0];
    my $eof = $_[2];

    my $len = length $$in;
    my $remainder = '';

    if (defined $$in && $len) {

        if ($self->{Streaming}) {

            if (length $self->{Pending}) {
                $$in = $self->{Pending} . $$in ;
                $len = length $$in;
                $self->{Pending} = '';
            }

            my $ind = index($$in, "\x50\x4b\x07\x08");

            if ($ind < 0) {
                $len = length $$in;
                if ($len >= 3 && substr($$in, -3) eq "\x50\x4b\x07") {
                    $ind = $len - 3 ;
                }
                elsif ($len >= 2 && substr($$in, -2) eq "\x50\x4b") {
                    $ind = $len - 2 ;
                }
                elsif ($len >= 1 && substr($$in, -1) eq "\x50") {
                    $ind = $len - 1 ;
                }
            }

            if ($ind >= 0) {
                $remainder = substr($$in, $ind) ;
                substr($$in, $ind) = '' ;
            }
        }

        if (length $remainder && length $remainder < $self->{DataHdrSize}) {
            $self->{Pending} = $remainder ;
            $remainder = '';
        }
        elsif (length $remainder >= $self->{DataHdrSize}) {
            my $crc = unpack "V", substr($remainder, 4);
            if ($crc == Compress::Raw::Zlib::crc32($$in,  $self->{CRC32})) {
                my ($l1, $l2) ;

                if ($self->{Zip64}) {
                    $l1 = U64::newUnpack_V64(substr($remainder, 8));
                    $l2 = U64::newUnpack_V64(substr($remainder, 16));
                }
                else {
                    $l1 = U64::newUnpack_V32(substr($remainder, 8));
                    $l2 = U64::newUnpack_V32(substr($remainder, 12));
                }

                my $newLen = $self->{CompSize}->clone();
                $newLen->add(length $$in);
                if ($l1->equal($l2) && $l1->equal($newLen) ) {
                    $eof = 1;
                }
                else {
                    $$in .= substr($remainder, 0, 4) ;
                    $remainder       = substr($remainder, 4);
                    #$self->{Pending} = substr($remainder, 4);
                    #$remainder = '';
                    $eof = 0;
                }
            }
            else {
                $$in .= substr($remainder, 0, 4) ;
                $remainder       = substr($remainder, 4);
                #$self->{Pending} = substr($remainder, 4);
                #$remainder = '';
                $eof = 0;
            }
        }

        if (length $$in) {
            $self->{CompSize}->add(length $$in) ;

            $self->{CRC32} = Compress::Raw::Zlib::crc32($$in,  $self->{CRC32})
                if $self->{wantCRC32};

            $self->{ADLER32} = Compress::Zlib::adler32($$in,  $self->{ADLER32})
                if $self->{wantADLER32};
        }

        ${ $_[1] } .= $$in;
        $$in  = $remainder;
    }

    return STATUS_ENDSTREAM if $eof;
    return STATUS_OK ;
}

sub reset
{
    my $self = shift;

    $self->{CompSize}->reset();
    $self->{UnCompSize} = 0;
    $self->{CRC32}      = Compress::Raw::Zlib::crc32('');
    $self->{ADLER32}    = Compress::Raw::Zlib::adler32('');

    return STATUS_OK ;
}

#sub count
#{
#    my $self = shift ;
#    return $self->{UnCompSize} ;
#}

sub compressedBytes
{
    my $self = shift ;
    return $self->{CompSize} ;
}

sub uncompressedBytes
{
    my $self = shift ;
    return $self->{CompSize} ;
}

sub sync
{
    return STATUS_OK ;
}

sub crc32
{
    my $self = shift ;
    return $self->{CRC32};
}

sub adler32
{
    my $self = shift ;
    return $self->{ADLER32};
}


1;

__END__
