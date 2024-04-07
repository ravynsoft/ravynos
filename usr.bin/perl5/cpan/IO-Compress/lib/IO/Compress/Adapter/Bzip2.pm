package IO::Compress::Adapter::Bzip2 ;

use strict;
use warnings;
use bytes;

use IO::Compress::Base::Common  2.204 qw(:Status);

use Compress::Raw::Bzip2  2.204 ;

our ($VERSION);
$VERSION = '2.204';

sub mkCompObject
{
    my $BlockSize100K = shift ;
    my $WorkFactor = shift ;
    my $Verbosity  = shift ;

    $BlockSize100K = 1 if ! defined $BlockSize100K ;
    $WorkFactor    = 0 if ! defined $WorkFactor ;
    $Verbosity     = 0 if ! defined $Verbosity ;

    my ($def, $status) = Compress::Raw::Bzip2->new(1, $BlockSize100K,
                                                 $WorkFactor, $Verbosity);

    return (undef, "Could not create Deflate object: $status", $status)
        if $status != BZ_OK ;

    return bless {'Def'        => $def,
                  'Error'      => '',
                  'ErrorNo'    => 0,
                 }  ;
}

sub compr
{
    my $self = shift ;

    my $def   = $self->{Def};

    my $status = $def->bzdeflate($_[0], $_[1]) ;
    $self->{ErrorNo} = $status;

    if ($status != BZ_RUN_OK)
    {
        $self->{Error} = "Deflate Error: $status";
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

sub flush
{
    my $self = shift ;

    my $def   = $self->{Def};

    my $status = $def->bzflush($_[0]);
    $self->{ErrorNo} = $status;

    if ($status != BZ_RUN_OK)
    {
        $self->{Error} = "Deflate Error: $status";
        return STATUS_ERROR;
    }

    return STATUS_OK;

}

sub close
{
    my $self = shift ;

    my $def   = $self->{Def};

    my $status = $def->bzclose($_[0]);
    $self->{ErrorNo} = $status;

    if ($status != BZ_STREAM_END)
    {
        $self->{Error} = "Deflate Error: $status";
        return STATUS_ERROR;
    }

    return STATUS_OK;

}


sub reset
{
    my $self = shift ;

    my $outer = $self->{Outer};

    my ($def, $status) = Compress::Raw::Bzip2->new();
    $self->{ErrorNo} = ($status == BZ_OK) ? 0 : $status ;

    if ($status != BZ_OK)
    {
        $self->{Error} = "Cannot create Deflate object: $status";
        return STATUS_ERROR;
    }

    $self->{Def} = $def;

    return STATUS_OK;
}

sub compressedBytes
{
    my $self = shift ;
    $self->{Def}->compressedBytes();
}

sub uncompressedBytes
{
    my $self = shift ;
    $self->{Def}->uncompressedBytes();
}

#sub total_out
#{
#    my $self = shift ;
#    0;
#}
#

#sub total_in
#{
#    my $self = shift ;
#    $self->{Def}->total_in();
#}
#
#sub crc32
#{
#    my $self = shift ;
#    $self->{Def}->crc32();
#}
#
#sub adler32
#{
#    my $self = shift ;
#    $self->{Def}->adler32();
#}


1;

__END__
