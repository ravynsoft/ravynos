package IO::Uncompress::Adapter::Bunzip2;

use strict;
use warnings;
use bytes;

use IO::Compress::Base::Common 2.204 qw(:Status);

use Compress::Raw::Bzip2 2.204 ;

our ($VERSION, @ISA);
$VERSION = '2.204';

sub mkUncompObject
{
    my $small     = shift || 0;
    my $verbosity = shift || 0;

    my ($inflate, $status) = Compress::Raw::Bunzip2->new(1, 1, $small, $verbosity, 1);

    return (undef, "Could not create Inflation object: $status", $status)
        if $status != BZ_OK ;

    return bless {'Inf'           => $inflate,
                  'CompSize'      => 0,
                  'UnCompSize'    => 0,
                  'Error'         => '',
                  'ConsumesInput' => 1,
                 }  ;

}

sub uncompr
{
    my $self = shift ;
    my $from = shift ;
    my $to   = shift ;
    my $eof  = shift ;

    my $inf   = $self->{Inf};

    my $status = $inf->bzinflate($from, $to);
    $self->{ErrorNo} = $status;

    if ($status != BZ_OK && $status != BZ_STREAM_END )
    {
        $self->{Error} = "Inflation Error: $status";
        return STATUS_ERROR;
    }


    return STATUS_OK        if $status == BZ_OK ;
    return STATUS_ENDSTREAM if $status == BZ_STREAM_END ;
    return STATUS_ERROR ;
}


sub reset
{
    my $self = shift ;

    my ($inf, $status) = Compress::Raw::Bunzip2->new();
    $self->{ErrorNo} = ($status == BZ_OK) ? 0 : $status ;

    if ($status != BZ_OK)
    {
        $self->{Error} = "Cannot create Inflate object: $status";
        return STATUS_ERROR;
    }

    $self->{Inf} = $inf;

    return STATUS_OK ;
}

sub compressedBytes
{
    my $self = shift ;
    $self->{Inf}->compressedBytes();
}

sub uncompressedBytes
{
    my $self = shift ;
    $self->{Inf}->uncompressedBytes();
}

sub crc32
{
    my $self = shift ;
    #$self->{Inf}->crc32();
}

sub adler32
{
    my $self = shift ;
    #$self->{Inf}->adler32();
}

sub sync
{
    my $self = shift ;
    #( $self->{Inf}->inflateSync(@_) == BZ_OK)
    #        ? STATUS_OK
    #        : STATUS_ERROR ;
}


1;

__END__
