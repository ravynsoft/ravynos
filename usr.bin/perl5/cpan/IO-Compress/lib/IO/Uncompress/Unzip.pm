package IO::Uncompress::Unzip;

require 5.006 ;

# for RFC1952

use strict ;
use warnings;
use bytes;

use IO::File;
use IO::Uncompress::RawInflate  2.204 ;
use IO::Compress::Base::Common  2.204 qw(:Status );
use IO::Uncompress::Adapter::Inflate  2.204 ;
use IO::Uncompress::Adapter::Identity 2.204 ;
use IO::Compress::Zlib::Extra 2.204 ;
use IO::Compress::Zip::Constants 2.204 ;

use Compress::Raw::Zlib  2.204 () ;

BEGIN
{
   # Don't trigger any __DIE__ Hooks.
   local $SIG{__DIE__};

    eval{ require IO::Uncompress::Adapter::Bunzip2 ;
          IO::Uncompress::Adapter::Bunzip2->import() } ;
    eval{ require IO::Uncompress::Adapter::UnLzma ;
          IO::Uncompress::Adapter::UnLzma->import() } ;
    eval{ require IO::Uncompress::Adapter::UnXz ;
          IO::Uncompress::Adapter::UnXz->import() } ;
    eval{ require IO::Uncompress::Adapter::UnZstd ;
          IO::Uncompress::Adapter::UnZstd->import() } ;
}


require Exporter ;

our ($VERSION, @ISA, @EXPORT_OK, %EXPORT_TAGS, $UnzipError, %headerLookup);

$VERSION = '2.204';
$UnzipError = '';

@ISA    = qw(IO::Uncompress::RawInflate Exporter);
@EXPORT_OK = qw($UnzipError unzip );
%EXPORT_TAGS = %IO::Uncompress::RawInflate::EXPORT_TAGS ;
push @{ $EXPORT_TAGS{all} }, @EXPORT_OK ;
Exporter::export_ok_tags('all');

%headerLookup = (
        ZIP_CENTRAL_HDR_SIG,            \&skipCentralDirectory,
        ZIP_END_CENTRAL_HDR_SIG,        \&skipEndCentralDirectory,
        ZIP64_END_CENTRAL_REC_HDR_SIG,  \&skipCentralDirectory64Rec,
        ZIP64_END_CENTRAL_LOC_HDR_SIG,  \&skipCentralDirectory64Loc,
        ZIP64_ARCHIVE_EXTRA_SIG,        \&skipArchiveExtra,
        ZIP64_DIGITAL_SIGNATURE_SIG,    \&skipDigitalSignature,
        );

my %MethodNames = (
        ZIP_CM_DEFLATE()    => 'Deflated',
        ZIP_CM_BZIP2()      => 'Bzip2',
        ZIP_CM_LZMA()       => 'Lzma',
        ZIP_CM_STORE()      => 'Stored',
        ZIP_CM_XZ()         => 'Xz',
        ZIP_CM_ZSTD()       => 'Zstd',
    );

sub new
{
    my $class = shift ;
    my $obj = IO::Compress::Base::Common::createSelfTiedObject($class, \$UnzipError);
    $obj->_create(undef, 0, @_);
}

sub unzip
{
    my $obj = IO::Compress::Base::Common::createSelfTiedObject(undef, \$UnzipError);
    return $obj->_inf(@_) ;
}

sub getExtraParams
{

    return (
#            # Zip header fields
            'name'    => [IO::Compress::Base::Common::Parse_any,       undef],

            'stream'  => [IO::Compress::Base::Common::Parse_boolean,   0],
            'efs'     => [IO::Compress::Base::Common::Parse_boolean,   0],

            # TODO - This means reading the central directory to get
            # 1. the local header offsets
            # 2. The compressed data length
        );
}

sub ckParams
{
    my $self = shift ;
    my $got = shift ;

    # unzip always needs crc32
    $got->setValue('crc32' => 1);

    *$self->{UnzipData}{Name} = $got->getValue('name');
    *$self->{UnzipData}{efs} = $got->getValue('efs');

    return 1;
}

sub mkUncomp
{
    my $self = shift ;
    my $got = shift ;

     my $magic = $self->ckMagic()
        or return 0;

    *$self->{Info} = $self->readHeader($magic)
        or return undef ;

    return 1;

}

sub ckMagic
{
    my $self = shift;

    my $magic ;
    $self->smartReadExact(\$magic, 4);

    *$self->{HeaderPending} = $magic ;

    return $self->HeaderError("Minimum header size is " .
                              4 . " bytes")
        if length $magic != 4 ;

    return $self->HeaderError("Bad Magic")
        if ! _isZipMagic($magic) ;

    *$self->{Type} = 'zip';

    return $magic ;
}


sub fastForward
{
    my $self = shift;
    my $offset = shift;

    # TODO - if Stream isn't enabled & reading from file, use seek

    my $buffer = '';
    my $c = 1024 * 16;

    while ($offset > 0)
    {
        $c = length $offset
            if length $offset < $c ;

        $offset -= $c;

        $self->smartReadExact(\$buffer, $c)
            or return 0;
    }

    return 1;
}


sub readHeader
{
    my $self = shift;
    my $magic = shift ;

    my $name =  *$self->{UnzipData}{Name} ;
    my $hdr = $self->_readZipHeader($magic) ;

    while (defined $hdr)
    {
        if (! defined $name || $hdr->{Name} eq $name)
        {
            return $hdr ;
        }

        # skip the data
        # TODO - when Stream is off, use seek
        my $buffer;
        if (*$self->{ZipData}{Streaming}) {
            while (1) {

                my $b;
                my $status = $self->smartRead(\$b, 1024 * 16);

                return $self->saveErrorString(undef, "Truncated file")
                    if $status <= 0 ;

                my $temp_buf ;
                my $out;

                $status = *$self->{Uncomp}->uncompr(\$b, \$temp_buf, 0, $out);

                return $self->saveErrorString(undef, *$self->{Uncomp}{Error},
                                                     *$self->{Uncomp}{ErrorNo})
                    if $self->saveStatus($status) == STATUS_ERROR;

                $self->pushBack($b)  ;

                if ($status == STATUS_ENDSTREAM) {
                    *$self->{Uncomp}->reset();
                    last;
                }
            }

            # skip the trailer
            $self->smartReadExact(\$buffer, $hdr->{TrailerLength})
                or return $self->saveErrorString(undef, "Truncated file");
        }
        else {
            my $c = $hdr->{CompressedLength}->get64bit();
            $self->fastForward($c)
                or return $self->saveErrorString(undef, "Truncated file");
            $buffer = '';
        }

        $self->chkTrailer($buffer) == STATUS_OK
            or return $self->saveErrorString(undef, "Truncated file");

        $hdr = $self->_readFullZipHeader();

        return $self->saveErrorString(undef, "Cannot find '$name'")
            if $self->smartEof();
    }

    return undef;
}

sub chkTrailer
{
    my $self = shift;
    my $trailer = shift;

    my ($sig, $CRC32, $cSize, $uSize) ;
    my ($cSizeHi, $uSizeHi) = (0, 0);
    if (*$self->{ZipData}{Streaming}) {
        $sig   = unpack ("V", substr($trailer, 0, 4));
        $CRC32 = unpack ("V", substr($trailer, 4, 4));

        if (*$self->{ZipData}{Zip64} ) {
            $cSize = U64::newUnpack_V64 substr($trailer,  8, 8);
            $uSize = U64::newUnpack_V64 substr($trailer, 16, 8);
        }
        else {
            $cSize = U64::newUnpack_V32 substr($trailer,  8, 4);
            $uSize = U64::newUnpack_V32 substr($trailer, 12, 4);
        }

        return $self->TrailerError("Data Descriptor signature, got $sig")
            if $sig != ZIP_DATA_HDR_SIG;
    }
    else {
        ($CRC32, $cSize, $uSize) =
            (*$self->{ZipData}{Crc32},
             *$self->{ZipData}{CompressedLen},
             *$self->{ZipData}{UnCompressedLen});
    }

    *$self->{Info}{CRC32} = *$self->{ZipData}{CRC32} ;
    *$self->{Info}{CompressedLength} = $cSize->get64bit();
    *$self->{Info}{UncompressedLength} = $uSize->get64bit();

    if (*$self->{Strict}) {
        return $self->TrailerError("CRC mismatch")
            if $CRC32  != *$self->{ZipData}{CRC32} ;

        return $self->TrailerError("CSIZE mismatch.")
            if ! $cSize->equal(*$self->{CompSize});

        return $self->TrailerError("USIZE mismatch.")
            if ! $uSize->equal(*$self->{UnCompSize});
    }

    my $reachedEnd = STATUS_ERROR ;
    # check for central directory or end of central directory
    while (1)
    {
        my $magic ;
        my $got = $self->smartRead(\$magic, 4);

        return $self->saveErrorString(STATUS_ERROR, "Truncated file")
            if $got != 4 && *$self->{Strict};

        if ($got == 0) {
            return STATUS_EOF ;
        }
        elsif ($got < 0) {
            return STATUS_ERROR ;
        }
        elsif ($got < 4) {
            $self->pushBack($magic)  ;
            return STATUS_OK ;
        }

        my $sig = unpack("V", $magic) ;

        my $hdr;
        if ($hdr = $headerLookup{$sig})
        {
            if (&$hdr($self, $magic) != STATUS_OK ) {
                if (*$self->{Strict}) {
                    return STATUS_ERROR ;
                }
                else {
                    $self->clearError();
                    return STATUS_OK ;
                }
            }

            if ($sig == ZIP_END_CENTRAL_HDR_SIG)
            {
                return STATUS_OK ;
                last;
            }
        }
        elsif ($sig == ZIP_LOCAL_HDR_SIG)
        {
            $self->pushBack($magic)  ;
            return STATUS_OK ;
        }
        else
        {
            # put the data back
            $self->pushBack($magic)  ;
            last;
        }
    }

    return $reachedEnd ;
}

sub skipCentralDirectory
{
    my $self = shift;
    my $magic = shift ;

    my $buffer;
    $self->smartReadExact(\$buffer, 46 - 4)
        or return $self->TrailerError("Minimum header size is " .
                                     46 . " bytes") ;

    my $keep = $magic . $buffer ;
    *$self->{HeaderPending} = $keep ;

   #my $versionMadeBy      = unpack ("v", substr($buffer, 4-4,  2));
   #my $extractVersion     = unpack ("v", substr($buffer, 6-4,  2));
   #my $gpFlag             = unpack ("v", substr($buffer, 8-4,  2));
   #my $compressedMethod   = unpack ("v", substr($buffer, 10-4, 2));
   #my $lastModTime        = unpack ("V", substr($buffer, 12-4, 4));
   #my $crc32              = unpack ("V", substr($buffer, 16-4, 4));
    my $compressedLength   = unpack ("V", substr($buffer, 20-4, 4));
    my $uncompressedLength = unpack ("V", substr($buffer, 24-4, 4));
    my $filename_length    = unpack ("v", substr($buffer, 28-4, 2));
    my $extra_length       = unpack ("v", substr($buffer, 30-4, 2));
    my $comment_length     = unpack ("v", substr($buffer, 32-4, 2));
   #my $disk_start         = unpack ("v", substr($buffer, 34-4, 2));
   #my $int_file_attrib    = unpack ("v", substr($buffer, 36-4, 2));
   #my $ext_file_attrib    = unpack ("V", substr($buffer, 38-4, 2));
   #my $lcl_hdr_offset     = unpack ("V", substr($buffer, 42-4, 2));


    my $filename;
    my $extraField;
    my $comment ;
    if ($filename_length)
    {
        $self->smartReadExact(\$filename, $filename_length)
            or return $self->TruncatedTrailer("filename");
        $keep .= $filename ;
    }

    if ($extra_length)
    {
        $self->smartReadExact(\$extraField, $extra_length)
            or return $self->TruncatedTrailer("extra");
        $keep .= $extraField ;
    }

    if ($comment_length)
    {
        $self->smartReadExact(\$comment, $comment_length)
            or return $self->TruncatedTrailer("comment");
        $keep .= $comment ;
    }

    return STATUS_OK ;
}

sub skipArchiveExtra
{
    my $self = shift;
    my $magic = shift ;

    my $buffer;
    $self->smartReadExact(\$buffer, 4)
        or return $self->TrailerError("Minimum header size is " .
                                     4 . " bytes") ;

    my $keep = $magic . $buffer ;

    my $size = unpack ("V", $buffer);

    $self->smartReadExact(\$buffer, $size)
        or return $self->TrailerError("Minimum header size is " .
                                     $size . " bytes") ;

    $keep .= $buffer ;
    *$self->{HeaderPending} = $keep ;

    return STATUS_OK ;
}


sub skipCentralDirectory64Rec
{
    my $self = shift;
    my $magic = shift ;

    my $buffer;
    $self->smartReadExact(\$buffer, 8)
        or return $self->TrailerError("Minimum header size is " .
                                     8 . " bytes") ;

    my $keep = $magic . $buffer ;

    my ($sizeLo, $sizeHi)  = unpack ("V V", $buffer);
    my $size = $sizeHi * U64::MAX32 + $sizeLo;

    $self->fastForward($size)
        or return $self->TrailerError("Minimum header size is " .
                                     $size . " bytes") ;

   #$keep .= $buffer ;
   #*$self->{HeaderPending} = $keep ;

   #my $versionMadeBy      = unpack ("v",   substr($buffer,  0, 2));
   #my $extractVersion     = unpack ("v",   substr($buffer,  2, 2));
   #my $diskNumber         = unpack ("V",   substr($buffer,  4, 4));
   #my $cntrlDirDiskNo     = unpack ("V",   substr($buffer,  8, 4));
   #my $entriesInThisCD    = unpack ("V V", substr($buffer, 12, 8));
   #my $entriesInCD        = unpack ("V V", substr($buffer, 20, 8));
   #my $sizeOfCD           = unpack ("V V", substr($buffer, 28, 8));
   #my $offsetToCD         = unpack ("V V", substr($buffer, 36, 8));

    return STATUS_OK ;
}

sub skipCentralDirectory64Loc
{
    my $self = shift;
    my $magic = shift ;

    my $buffer;
    $self->smartReadExact(\$buffer, 20 - 4)
        or return $self->TrailerError("Minimum header size is " .
                                     20 . " bytes") ;

    my $keep = $magic . $buffer ;
    *$self->{HeaderPending} = $keep ;

   #my $startCdDisk        = unpack ("V",   substr($buffer,  4-4, 4));
   #my $offsetToCD         = unpack ("V V", substr($buffer,  8-4, 8));
   #my $diskCount          = unpack ("V",   substr($buffer, 16-4, 4));

    return STATUS_OK ;
}

sub skipEndCentralDirectory
{
    my $self = shift;
    my $magic = shift ;


    my $buffer;
    $self->smartReadExact(\$buffer, 22 - 4)
        or return $self->TrailerError("Minimum header size is " .
                                     22 . " bytes") ;

    my $keep = $magic . $buffer ;
    *$self->{HeaderPending} = $keep ;

   #my $diskNumber         = unpack ("v", substr($buffer, 4-4,  2));
   #my $cntrlDirDiskNo     = unpack ("v", substr($buffer, 6-4,  2));
   #my $entriesInThisCD    = unpack ("v", substr($buffer, 8-4,  2));
   #my $entriesInCD        = unpack ("v", substr($buffer, 10-4, 2));
   #my $sizeOfCD           = unpack ("V", substr($buffer, 12-4, 4));
   #my $offsetToCD         = unpack ("V", substr($buffer, 16-4, 4));
    my $comment_length     = unpack ("v", substr($buffer, 20-4, 2));


    my $comment ;
    if ($comment_length)
    {
        $self->smartReadExact(\$comment, $comment_length)
            or return $self->TruncatedTrailer("comment");
        $keep .= $comment ;
    }

    return STATUS_OK ;
}


sub _isZipMagic
{
    my $buffer = shift ;
    return 0 if length $buffer < 4 ;
    my $sig = unpack("V", $buffer) ;
    return $sig == ZIP_LOCAL_HDR_SIG ;
}


sub _readFullZipHeader($)
{
    my ($self) = @_ ;
    my $magic = '' ;

    $self->smartReadExact(\$magic, 4);

    *$self->{HeaderPending} = $magic ;

    return $self->HeaderError("Minimum header size is " .
                              30 . " bytes")
        if length $magic != 4 ;


    return $self->HeaderError("Bad Magic")
        if ! _isZipMagic($magic) ;

    my $status = $self->_readZipHeader($magic);
    delete *$self->{Transparent} if ! defined $status ;
    return $status ;
}

sub _readZipHeader($)
{
    my ($self, $magic) = @_ ;
    my ($HeaderCRC) ;
    my ($buffer) = '' ;

    $self->smartReadExact(\$buffer, 30 - 4)
        or return $self->HeaderError("Minimum header size is " .
                                     30 . " bytes") ;

    my $keep = $magic . $buffer ;
    *$self->{HeaderPending} = $keep ;

    my $extractVersion     = unpack ("v", substr($buffer, 4-4,  2));
    my $gpFlag             = unpack ("v", substr($buffer, 6-4,  2));
    my $compressedMethod   = unpack ("v", substr($buffer, 8-4,  2));
    my $lastModTime        = unpack ("V", substr($buffer, 10-4, 4));
    my $crc32              = unpack ("V", substr($buffer, 14-4, 4));
    my $compressedLength   = U64::newUnpack_V32 substr($buffer, 18-4, 4);
    my $uncompressedLength = U64::newUnpack_V32 substr($buffer, 22-4, 4);
    my $filename_length    = unpack ("v", substr($buffer, 26-4, 2));
    my $extra_length       = unpack ("v", substr($buffer, 28-4, 2));

    my $filename;
    my $extraField;
    my @EXTRA = ();

    # Some programs (some versions of LibreOffice) mark entries as streamed, but still fill out
    # compressedLength/uncompressedLength & crc32 in the local file header.
    # The expected data descriptor is not populated.
    # So only assume streaming if the Streaming bit is set AND the compressed length is zero
    my $streamingMode = (($gpFlag & ZIP_GP_FLAG_STREAMING_MASK)  && $crc32 == 0) ? 1 : 0 ;

    my $efs_flag = ($gpFlag & ZIP_GP_FLAG_LANGUAGE_ENCODING) ? 1 : 0;

    return $self->HeaderError("Encrypted content not supported")
        if $gpFlag & (ZIP_GP_FLAG_ENCRYPTED_MASK|ZIP_GP_FLAG_STRONG_ENCRYPTED_MASK);

    return $self->HeaderError("Patch content not supported")
        if $gpFlag & ZIP_GP_FLAG_PATCHED_MASK;

    *$self->{ZipData}{Streaming} = $streamingMode;


    if ($filename_length)
    {
        $self->smartReadExact(\$filename, $filename_length)
            or return $self->TruncatedHeader("Filename");

        if (*$self->{UnzipData}{efs} && $efs_flag && $] >= 5.008004)
        {
            require Encode;
            eval { $filename = Encode::decode_utf8($filename, 1) }
                or Carp::croak "Zip Filename not UTF-8" ;
        }

        $keep .= $filename ;
    }

    my $zip64 = 0 ;

    if ($extra_length)
    {
        $self->smartReadExact(\$extraField, $extra_length)
            or return $self->TruncatedHeader("Extra Field");

        my $bad = IO::Compress::Zlib::Extra::parseRawExtra($extraField,
                                                \@EXTRA, 1, 0);
        return $self->HeaderError($bad)
            if defined $bad;

        $keep .= $extraField ;

        my %Extra ;
        for (@EXTRA)
        {
            $Extra{$_->[0]} = \$_->[1];
        }

        if (defined $Extra{ZIP_EXTRA_ID_ZIP64()})
        {
            $zip64 = 1 ;

            my $buff = ${ $Extra{ZIP_EXTRA_ID_ZIP64()} };

            # This code assumes that all the fields in the Zip64
            # extra field aren't necessarily present. The spec says that
            # they only exist if the equivalent local headers are -1.

            if (! $streamingMode) {
                my $offset = 0 ;

                if (U64::full32 $uncompressedLength->get32bit() ) {
                    $uncompressedLength
                            = U64::newUnpack_V64 substr($buff, 0, 8);

                    $offset += 8 ;
                }

                if (U64::full32 $compressedLength->get32bit() ) {

                    $compressedLength
                        = U64::newUnpack_V64 substr($buff, $offset, 8);

                    $offset += 8 ;
                }
           }
        }
    }

    *$self->{ZipData}{Zip64} = $zip64;

    if (! $streamingMode) {
        *$self->{ZipData}{Streaming} = 0;
        *$self->{ZipData}{Crc32} = $crc32;
        *$self->{ZipData}{CompressedLen} = $compressedLength;
        *$self->{ZipData}{UnCompressedLen} = $uncompressedLength;
        *$self->{CompressedInputLengthRemaining} =
            *$self->{CompressedInputLength} = $compressedLength->get64bit();
    }

    *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(undef);
    *$self->{ZipData}{Method} = $compressedMethod;
    if ($compressedMethod == ZIP_CM_DEFLATE)
    {
        *$self->{Type} = 'zip-deflate';
        my $obj = IO::Uncompress::Adapter::Inflate::mkUncompObject(1,0,0);

        *$self->{Uncomp} = $obj;
    }
    elsif ($compressedMethod == ZIP_CM_BZIP2)
    {
        return $self->HeaderError("Unsupported Compression format $compressedMethod")
            if ! defined $IO::Uncompress::Adapter::Bunzip2::VERSION ;

        *$self->{Type} = 'zip-bzip2';

        my $obj = IO::Uncompress::Adapter::Bunzip2::mkUncompObject();

        *$self->{Uncomp} = $obj;
    }
    elsif ($compressedMethod == ZIP_CM_XZ)
    {
        return $self->HeaderError("Unsupported Compression format $compressedMethod")
            if ! defined $IO::Uncompress::Adapter::UnXz::VERSION ;

        *$self->{Type} = 'zip-xz';

        my $obj = IO::Uncompress::Adapter::UnXz::mkUncompObject();

        *$self->{Uncomp} = $obj;
    }
    elsif ($compressedMethod == ZIP_CM_ZSTD)
    {
        return $self->HeaderError("Unsupported Compression format $compressedMethod")
            if ! defined $IO::Uncompress::Adapter::UnZstd::VERSION ;

        *$self->{Type} = 'zip-zstd';

        my $obj = IO::Uncompress::Adapter::UnZstd::mkUncompObject();

        *$self->{Uncomp} = $obj;
    }
    elsif ($compressedMethod == ZIP_CM_LZMA)
    {
        return $self->HeaderError("Unsupported Compression format $compressedMethod")
            if ! defined $IO::Uncompress::Adapter::UnLzma::VERSION ;

        *$self->{Type} = 'zip-lzma';
        my $LzmaHeader;
        $self->smartReadExact(\$LzmaHeader, 4)
                or return $self->saveErrorString(undef, "Truncated file");
        my ($verHi, $verLo)   = unpack ("CC", substr($LzmaHeader, 0, 2));
        my $LzmaPropertiesSize   = unpack ("v", substr($LzmaHeader, 2, 2));


        my $LzmaPropertyData;
        $self->smartReadExact(\$LzmaPropertyData, $LzmaPropertiesSize)
                or return $self->saveErrorString(undef, "Truncated file");

        if (! $streamingMode) {
            *$self->{ZipData}{CompressedLen}->subtract(4 + $LzmaPropertiesSize) ;
            *$self->{CompressedInputLengthRemaining} =
                *$self->{CompressedInputLength} = *$self->{ZipData}{CompressedLen}->get64bit();
        }

        my $obj =
            IO::Uncompress::Adapter::UnLzma::mkUncompZipObject($LzmaPropertyData);

        *$self->{Uncomp} = $obj;
    }
    elsif ($compressedMethod == ZIP_CM_STORE)
    {
        *$self->{Type} = 'zip-stored';

        my $obj =
        IO::Uncompress::Adapter::Identity::mkUncompObject($streamingMode,
                                                          $zip64);

        *$self->{Uncomp} = $obj;
    }
    else
    {
        return $self->HeaderError("Unsupported Compression format $compressedMethod");
    }

    return {
        'Type'               => 'zip',
        'FingerprintLength'  => 4,
        #'HeaderLength'       => $compressedMethod == 8 ? length $keep : 0,
        'HeaderLength'       => length $keep,
        'Zip64'              => $zip64,
        'TrailerLength'      => ! $streamingMode ? 0 : $zip64 ? 24 : 16,
        'Header'             => $keep,
        'CompressedLength'   => $compressedLength ,
        'UncompressedLength' => $uncompressedLength ,
        'CRC32'              => $crc32 ,
        'Name'               => $filename,
        'efs'                => $efs_flag, # language encoding flag
        'Time'               => _dosToUnixTime($lastModTime),
        'Stream'             => $streamingMode,

        'MethodID'           => $compressedMethod,
        'MethodName'         => $MethodNames{$compressedMethod} || 'Unknown',

#        'TextFlag'      => $flag & GZIP_FLG_FTEXT ? 1 : 0,
#        'HeaderCRCFlag' => $flag & GZIP_FLG_FHCRC ? 1 : 0,
#        'NameFlag'      => $flag & GZIP_FLG_FNAME ? 1 : 0,
#        'CommentFlag'   => $flag & GZIP_FLG_FCOMMENT ? 1 : 0,
#        'ExtraFlag'     => $flag & GZIP_FLG_FEXTRA ? 1 : 0,
#        'Comment'       => $comment,
#        'OsID'          => $os,
#        'OsName'        => defined $GZIP_OS_Names{$os}
#                                 ? $GZIP_OS_Names{$os} : "Unknown",
#        'HeaderCRC'     => $HeaderCRC,
#        'Flags'         => $flag,
#        'ExtraFlags'    => $xfl,
        'ExtraFieldRaw' => $extraField,
        'ExtraField'    => [ @EXTRA ],


      }
}

sub filterUncompressed
{
    my $self = shift ;

    if (*$self->{ZipData}{Method} == ZIP_CM_DEFLATE) {
        *$self->{ZipData}{CRC32} = *$self->{Uncomp}->crc32() ;
    }
    else {
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(${$_[0]}, *$self->{ZipData}{CRC32}, $_[1]);
    }
}


# from Archive::Zip & info-zip
sub _dosToUnixTime
{
	my $dt = shift;

	my $year = ( ( $dt >> 25 ) & 0x7f ) + 80;
	my $mon  = ( ( $dt >> 21 ) & 0x0f ) - 1;
	my $mday = ( ( $dt >> 16 ) & 0x1f );

	my $hour = ( ( $dt >> 11 ) & 0x1f );
	my $min  = ( ( $dt >> 5 ) & 0x3f );
	my $sec  = ( ( $dt << 1 ) & 0x3e );

    use Time::Local ;
    my $time_t = Time::Local::timelocal( $sec, $min, $hour, $mday, $mon, $year);
    return 0 if ! defined $time_t;
    return $time_t;

}

#sub scanCentralDirectory
#{
#    # Use cases
#    # 1 32-bit CD
#    # 2 64-bit CD
#
#    my $self = shift ;
#
#    my @CD = ();
#    my $offset = $self->findCentralDirectoryOffset();
#
#    return 0
#        if ! defined $offset;
#
#    $self->smarkSeek($offset, 0, SEEK_SET) ;
#
#    # Now walk the Central Directory Records
#    my $buffer ;
#    while ($self->smartReadExact(\$buffer, 46) &&
#           unpack("V", $buffer) == ZIP_CENTRAL_HDR_SIG) {
#
#        my $compressedLength   = unpack ("V", substr($buffer, 20, 4));
#        my $filename_length    = unpack ("v", substr($buffer, 28, 2));
#        my $extra_length       = unpack ("v", substr($buffer, 30, 2));
#        my $comment_length     = unpack ("v", substr($buffer, 32, 2));
#
#        $self->smarkSeek($filename_length + $extra_length + $comment_length, 0, SEEK_CUR)
#            if $extra_length || $comment_length || $filename_length;
#        push @CD, $compressedLength ;
#    }
#
#}
#
#sub findCentralDirectoryOffset
#{
#    my $self = shift ;
#
#    # Most common use-case is where there is no comment, so
#    # know exactly where the end of central directory record
#    # should be.
#
#    $self->smarkSeek(-22, 0, SEEK_END) ;
#
#    my $buffer;
#    $self->smartReadExact(\$buffer, 22) ;
#
#    my $zip64 = 0;
#    my $centralDirOffset ;
#    if ( unpack("V", $buffer) == ZIP_END_CENTRAL_HDR_SIG ) {
#        $centralDirOffset = unpack ("V", substr($buffer, 16, 2));
#    }
#    else {
#        die "xxxx";
#    }
#
#    return $centralDirOffset ;
#}
#
#sub is84BitCD
#{
#    # TODO
#    my $self = shift ;
#}


sub skip
{
    my $self = shift;
    my $size = shift;

    use Fcntl qw(SEEK_CUR);
    if (ref $size eq 'U64') {
        $self->smartSeek($size->get64bit(), SEEK_CUR);
    }
    else {
        $self->smartSeek($size, SEEK_CUR);
    }

}


sub scanCentralDirectory
{
    my $self = shift;

    my $here = $self->tell();

    # Use cases
    # 1 32-bit CD
    # 2 64-bit CD

    my @CD = ();
    my $offset = $self->findCentralDirectoryOffset();

    return ()
        if ! defined $offset;

    $self->smarkSeek($offset, 0, SEEK_SET) ;

    # Now walk the Central Directory Records
    my $buffer ;
    while ($self->smartReadExact(\$buffer, 46) &&
           unpack("V", $buffer) == ZIP_CENTRAL_HDR_SIG) {

        my $compressedLength   = unpack("V", substr($buffer, 20, 4));
        my $uncompressedLength = unpack("V", substr($buffer, 24, 4));
        my $filename_length    = unpack("v", substr($buffer, 28, 2));
        my $extra_length       = unpack("v", substr($buffer, 30, 2));
        my $comment_length     = unpack("v", substr($buffer, 32, 2));

        $self->skip($filename_length ) ;

        my $v64 = U64->new( $compressedLength );

        if (U64::full32 $compressedLength ) {
            $self->smartReadExact(\$buffer, $extra_length) ;
            die "xxx $offset $comment_length $filename_length $extra_length" . length($buffer)
                if length($buffer) != $extra_length;
            my $got = $self->get64Extra($buffer, U64::full32 $uncompressedLength);

            # If not Zip64 extra field, assume size is 0xFFFFFFFF
            $v64 = $got if defined $got;
        }
        else {
            $self->skip($extra_length) ;
        }

        $self->skip($comment_length ) ;

        push @CD, $v64 ;
    }

    $self->smartSeek($here, 0, SEEK_SET) ;

    return @CD;
}

sub get64Extra
{
    my $self = shift ;

    my $buffer = shift;
    my $is_uncomp = shift ;

    my $extra = IO::Compress::Zlib::Extra::findID(0x0001, $buffer);

    if (! defined $extra)
    {
        return undef;
    }
    else
    {
        my $u64 = U64::newUnpack_V64(substr($extra,  $is_uncomp ? 8 : 0)) ;
        return $u64;
    }
}

sub offsetFromZip64
{
    my $self = shift ;
    my $here = shift;

    $self->smartSeek($here - 20, 0, SEEK_SET)
        or die "xx $!" ;

    my $buffer;
    my $got = 0;
    $self->smartReadExact(\$buffer, 20)
        or die "xxx $here $got $!" ;

    if ( unpack("V", $buffer) == ZIP64_END_CENTRAL_LOC_HDR_SIG ) {
        my $cd64 = U64::Value_VV64 substr($buffer,  8, 8);

        $self->smartSeek($cd64, 0, SEEK_SET) ;

        $self->smartReadExact(\$buffer, 4)
            or die "xxx" ;

        if ( unpack("V", $buffer) == ZIP64_END_CENTRAL_REC_HDR_SIG ) {

            $self->smartReadExact(\$buffer, 8)
                or die "xxx" ;
            my $size  = U64::Value_VV64($buffer);
            $self->smartReadExact(\$buffer, $size)
                or die "xxx" ;

            my $cd64 =  U64::Value_VV64 substr($buffer,  36, 8);

            return $cd64 ;
        }

        die "zzz";
    }

    die "zzz";
}

use constant Pack_ZIP_END_CENTRAL_HDR_SIG => pack("V", ZIP_END_CENTRAL_HDR_SIG);

sub findCentralDirectoryOffset
{
    my $self = shift ;

    # Most common use-case is where there is no comment, so
    # know exactly where the end of central directory record
    # should be.

    $self->smartSeek(-22, 0, SEEK_END) ;
    my $here = $self->tell();

    my $buffer;
    $self->smartReadExact(\$buffer, 22)
        or die "xxx" ;

    my $zip64 = 0;
    my $centralDirOffset ;
    if ( unpack("V", $buffer) == ZIP_END_CENTRAL_HDR_SIG ) {
        $centralDirOffset = unpack("V", substr($buffer, 16,  4));
    }
    else {
        $self->smartSeek(0, 0, SEEK_END) ;

        my $fileLen = $self->tell();
        my $want = 0 ;

        while(1) {
            $want += 1024;
            my $seekTo = $fileLen - $want;
            if ($seekTo < 0 ) {
                $seekTo = 0;
                $want = $fileLen ;
            }
            $self->smartSeek( $seekTo, 0, SEEK_SET)
                or die "xxx $!" ;
            my $got;
            $self->smartReadExact($buffer, $want)
                or die "xxx " ;
            my $pos = rindex( $buffer, Pack_ZIP_END_CENTRAL_HDR_SIG);

            if ($pos >= 0) {
                #$here = $self->tell();
                $here = $seekTo + $pos ;
                $centralDirOffset = unpack("V", substr($buffer, $pos + 16,  4));
                last ;
            }

            return undef
                if $want == $fileLen;
        }
    }

    $centralDirOffset = $self->offsetFromZip64($here)
        if U64::full32 $centralDirOffset ;

    return $centralDirOffset ;
}

1;

__END__


=head1 NAME

IO::Uncompress::Unzip - Read zip files/buffers

=head1 SYNOPSIS

    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;

    my $status = unzip $input => $output [,OPTS]
        or die "unzip failed: $UnzipError\n";

    my $z = IO::Uncompress::Unzip->new( $input [OPTS] )
        or die "unzip failed: $UnzipError\n";

    $status = $z->read($buffer)
    $status = $z->read($buffer, $length)
    $status = $z->read($buffer, $length, $offset)
    $line = $z->getline()
    $char = $z->getc()
    $char = $z->ungetc()
    $char = $z->opened()

    $status = $z->inflateSync()

    $data = $z->trailingData()
    $status = $z->nextStream()
    $data = $z->getHeaderInfo()
    $z->tell()
    $z->seek($position, $whence)
    $z->binmode()
    $z->fileno()
    $z->eof()
    $z->close()

    $UnzipError ;

    # IO::File mode

    <$z>
    read($z, $buffer);
    read($z, $buffer, $length);
    read($z, $buffer, $length, $offset);
    tell($z)
    seek($z, $position, $whence)
    binmode($z)
    fileno($z)
    eof($z)
    close($z)

=head1 DESCRIPTION

This module provides a Perl interface that allows the reading of
zlib files/buffers.

For writing zip files/buffers, see the companion module IO::Compress::Zip.

The primary purpose of this module is to provide I<streaming> read access to
zip files and buffers.

At present the following compression methods are supported by IO::Uncompress::Unzip

=over 5

=item Store (0)

=item Deflate (8)

=item Bzip2 (12)

To read Bzip2 content, the module C<IO::Uncompress::Bunzip2> must
be installed.

=item Lzma (14)

To read LZMA content, the module C<IO::Uncompress::UnLzma> must
be installed.

=item Xz (95)

To read Xz content, the module C<IO::Uncompress::UnXz> must
be installed.

=item Zstandard (93)

To read Zstandard content, the module C<IO::Uncompress::UnZstd> must
be installed.

=back

=head1 Functional Interface

A top-level function, C<unzip>, is provided to carry out
"one-shot" uncompression between buffers and/or files. For finer
control over the uncompression process, see the L</"OO Interface">
section.

    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;

    unzip $input_filename_or_reference => $output_filename_or_reference [,OPTS]
        or die "unzip failed: $UnzipError\n";

The functional interface needs Perl5.005 or better.

=head2 unzip $input_filename_or_reference => $output_filename_or_reference [, OPTS]

C<unzip> expects at least two parameters,
C<$input_filename_or_reference> and C<$output_filename_or_reference>
and zero or more optional parameters (see L</Optional Parameters>)

=head3 The C<$input_filename_or_reference> parameter

The parameter, C<$input_filename_or_reference>, is used to define the
source of the compressed data.

It can take one of the following forms:

=over 5

=item A filename

If the C<$input_filename_or_reference> parameter is a simple scalar, it is
assumed to be a filename. This file will be opened for reading and the
input data will be read from it.

=item A filehandle

If the C<$input_filename_or_reference> parameter is a filehandle, the input
data will be read from it.  The string '-' can be used as an alias for
standard input.

=item A scalar reference

If C<$input_filename_or_reference> is a scalar reference, the input data
will be read from C<$$input_filename_or_reference>.

=item An array reference

If C<$input_filename_or_reference> is an array reference, each element in
the array must be a filename.

The input data will be read from each file in turn.

The complete array will be walked to ensure that it only
contains valid filenames before any data is uncompressed.

=item An Input FileGlob string

If C<$input_filename_or_reference> is a string that is delimited by the
characters "<" and ">" C<unzip> will assume that it is an
I<input fileglob string>. The input is the list of files that match the
fileglob.

See L<File::GlobMapper|File::GlobMapper> for more details.

=back

If the C<$input_filename_or_reference> parameter is any other type,
C<undef> will be returned.

=head3 The C<$output_filename_or_reference> parameter

The parameter C<$output_filename_or_reference> is used to control the
destination of the uncompressed data. This parameter can take one of
these forms.

=over 5

=item A filename

If the C<$output_filename_or_reference> parameter is a simple scalar, it is
assumed to be a filename.  This file will be opened for writing and the
uncompressed data will be written to it.

=item A filehandle

If the C<$output_filename_or_reference> parameter is a filehandle, the
uncompressed data will be written to it.  The string '-' can be used as
an alias for standard output.

=item A scalar reference

If C<$output_filename_or_reference> is a scalar reference, the
uncompressed data will be stored in C<$$output_filename_or_reference>.

=item An Array Reference

If C<$output_filename_or_reference> is an array reference,
the uncompressed data will be pushed onto the array.

=item An Output FileGlob

If C<$output_filename_or_reference> is a string that is delimited by the
characters "<" and ">" C<unzip> will assume that it is an
I<output fileglob string>. The output is the list of files that match the
fileglob.

When C<$output_filename_or_reference> is an fileglob string,
C<$input_filename_or_reference> must also be a fileglob string. Anything
else is an error.

See L<File::GlobMapper|File::GlobMapper> for more details.

=back

If the C<$output_filename_or_reference> parameter is any other type,
C<undef> will be returned.

=head2 Notes

When C<$input_filename_or_reference> maps to multiple compressed
files/buffers and C<$output_filename_or_reference> is
a single file/buffer, after uncompression C<$output_filename_or_reference> will contain a
concatenation of all the uncompressed data from each of the input
files/buffers.

=head2 Optional Parameters

The optional parameters for the one-shot function C<unzip>
are (for the most part) identical to those used with the OO interface defined in the
L</"Constructor Options"> section. The exceptions are listed below

=over 5

=item C<< AutoClose => 0|1 >>

This option applies to any input or output data streams to
C<unzip> that are filehandles.

If C<AutoClose> is specified, and the value is true, it will result in all
input and/or output filehandles being closed once C<unzip> has
completed.

This parameter defaults to 0.

=item C<< BinModeOut => 0|1 >>

This option is now a no-op. All files will be written  in binmode.

=item C<< Append => 0|1 >>

The behaviour of this option is dependent on the type of output data
stream.

=over 5

=item * A Buffer

If C<Append> is enabled, all uncompressed data will be append to the end of
the output buffer. Otherwise the output buffer will be cleared before any
uncompressed data is written to it.

=item * A Filename

If C<Append> is enabled, the file will be opened in append mode. Otherwise
the contents of the file, if any, will be truncated before any uncompressed
data is written to it.

=item * A Filehandle

If C<Append> is enabled, the filehandle will be positioned to the end of
the file via a call to C<seek> before any uncompressed data is
written to it.  Otherwise the file pointer will not be moved.

=back

When C<Append> is specified, and set to true, it will I<append> all uncompressed
data to the output data stream.

So when the output is a filehandle it will carry out a seek to the eof
before writing any uncompressed data. If the output is a filename, it will be opened for
appending. If the output is a buffer, all uncompressed data will be
appended to the existing buffer.

Conversely when C<Append> is not specified, or it is present and is set to
false, it will operate as follows.

When the output is a filename, it will truncate the contents of the file
before writing any uncompressed data. If the output is a filehandle
its position will not be changed. If the output is a buffer, it will be
wiped before any uncompressed data is output.

Defaults to 0.

=item C<< MultiStream => 0|1 >>

If the input file/buffer contains multiple compressed data streams, this
option will uncompress the whole lot as a single data stream.

Defaults to 0.

=item C<< TrailingData => $scalar >>

Returns the data, if any, that is present immediately after the compressed
data stream once uncompression is complete.

This option can be used when there is useful information immediately
following the compressed data stream, and you don't know the length of the
compressed data stream.

If the input is a buffer, C<trailingData> will return everything from the
end of the compressed data stream to the end of the buffer.

If the input is a filehandle, C<trailingData> will return the data that is
left in the filehandle input buffer once the end of the compressed data
stream has been reached. You can then use the filehandle to read the rest
of the input file.

Don't bother using C<trailingData> if the input is a filename.

If you know the length of the compressed data stream before you start
uncompressing, you can avoid having to use C<trailingData> by setting the
C<InputLength> option.

=back

=head2 Examples

Say you have a zip file, C<file1.zip>, that only contains a
single member, you can read it and write the uncompressed data to the
file C<file1.txt> like this.

    use strict ;
    use warnings ;
    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;

    my $input = "file1.zip";
    my $output = "file1.txt";
    unzip $input => $output
        or die "unzip failed: $UnzipError\n";

If you have a zip file that contains multiple members and want to read a
specific member from the file, say C<"data1">, use the C<Name> option

    use strict ;
    use warnings ;
    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;

    my $input = "file1.zip";
    my $output = "file1.txt";
    unzip $input => $output, Name => "data1"
        or die "unzip failed: $UnzipError\n";

Alternatively, if you want to read the  C<"data1"> member into memory, use
a scalar reference for the C<output> parameter.

    use strict ;
    use warnings ;
    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;

    my $input = "file1.zip";
    my $output ;
    unzip $input => \$output, Name => "data1"
        or die "unzip failed: $UnzipError\n";
    # $output now contains the uncompressed data

To read from an existing Perl filehandle, C<$input>, and write the
uncompressed data to a buffer, C<$buffer>.

    use strict ;
    use warnings ;
    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;
    use IO::File ;

    my $input = IO::File->new( "<file1.zip" )
        or die "Cannot open 'file1.zip': $!\n" ;
    my $buffer ;
    unzip $input => \$buffer
        or die "unzip failed: $UnzipError\n";

=head1 OO Interface

=head2 Constructor

The format of the constructor for IO::Uncompress::Unzip is shown below

    my $z = IO::Uncompress::Unzip->new( $input [OPTS] )
        or die "IO::Uncompress::Unzip failed: $UnzipError\n";

Returns an C<IO::Uncompress::Unzip> object on success and undef on failure.
The variable C<$UnzipError> will contain an error message on failure.

If you are running Perl 5.005 or better the object, C<$z>, returned from
IO::Uncompress::Unzip can be used exactly like an L<IO::File|IO::File> filehandle.
This means that all normal input file operations can be carried out with
C<$z>.  For example, to read a line from a compressed file/buffer you can
use either of these forms

    $line = $z->getline();
    $line = <$z>;

The mandatory parameter C<$input> is used to determine the source of the
compressed data. This parameter can take one of three forms.

=over 5

=item A filename

If the C<$input> parameter is a scalar, it is assumed to be a filename. This
file will be opened for reading and the compressed data will be read from it.

=item A filehandle

If the C<$input> parameter is a filehandle, the compressed data will be
read from it.
The string '-' can be used as an alias for standard input.

=item A scalar reference

If C<$input> is a scalar reference, the compressed data will be read from
C<$$input>.

=back

=head2 Constructor Options

The option names defined below are case insensitive and can be optionally
prefixed by a '-'.  So all of the following are valid

    -AutoClose
    -autoclose
    AUTOCLOSE
    autoclose

OPTS is a combination of the following options:

=over 5

=item C<< Name => "membername" >>

Open "membername" from the zip file for reading.

=item C<< Efs => 0| 1 >>

When this option is set to true AND the zip archive being read has
the "Language Encoding Flag" (EFS) set, the member name is assumed to be encoded in UTF-8.

If the member name in the zip archive is not valid UTF-8 when this optionn is true,
the script will die with an error message.

Note that this option only works with Perl 5.8.4 or better.

This option defaults to B<false>.

=item C<< AutoClose => 0|1 >>

This option is only valid when the C<$input> parameter is a filehandle. If
specified, and the value is true, it will result in the file being closed once
either the C<close> method is called or the IO::Uncompress::Unzip object is
destroyed.

This parameter defaults to 0.

=item C<< MultiStream => 0|1 >>

Treats the complete zip file/buffer as a single compressed data
stream. When reading in multi-stream mode each member of the zip
file/buffer will be uncompressed in turn until the end of the file/buffer
is encountered.

This parameter defaults to 0.

=item C<< Prime => $string >>

This option will uncompress the contents of C<$string> before processing the
input file/buffer.

This option can be useful when the compressed data is embedded in another
file/data structure and it is not possible to work out where the compressed
data begins without having to read the first few bytes. If this is the
case, the uncompression can be I<primed> with these bytes using this
option.

=item C<< Transparent => 0|1 >>

If this option is set and the input file/buffer is not compressed data,
the module will allow reading of it anyway.

In addition, if the input file/buffer does contain compressed data and
there is non-compressed data immediately following it, setting this option
will make this module treat the whole file/buffer as a single data stream.

This option defaults to 1.

=item C<< BlockSize => $num >>

When reading the compressed input data, IO::Uncompress::Unzip will read it in
blocks of C<$num> bytes.

This option defaults to 4096.

=item C<< InputLength => $size >>

When present this option will limit the number of compressed bytes read
from the input file/buffer to C<$size>. This option can be used in the
situation where there is useful data directly after the compressed data
stream and you know beforehand the exact length of the compressed data
stream.

This option is mostly used when reading from a filehandle, in which case
the file pointer will be left pointing to the first byte directly after the
compressed data stream.

This option defaults to off.

=item C<< Append => 0|1 >>

This option controls what the C<read> method does with uncompressed data.

If set to 1, all uncompressed data will be appended to the output parameter
of the C<read> method.

If set to 0, the contents of the output parameter of the C<read> method
will be overwritten by the uncompressed data.

Defaults to 0.

=item C<< Strict => 0|1 >>

This option controls whether the extra checks defined below are used when
carrying out the decompression. When Strict is on, the extra tests are
carried out, when Strict is off they are not.

The default for this option is off.

=back

=head2 Examples

TODO

=head1 Methods

=head2 read

Usage is

    $status = $z->read($buffer)

Reads a block of compressed data (the size of the compressed block is
determined by the C<Buffer> option in the constructor), uncompresses it and
writes any uncompressed data into C<$buffer>. If the C<Append> parameter is
set in the constructor, the uncompressed data will be appended to the
C<$buffer> parameter. Otherwise C<$buffer> will be overwritten.

Returns the number of uncompressed bytes written to C<$buffer>, zero if eof
or a negative number on error.

=head2 read

Usage is

    $status = $z->read($buffer, $length)
    $status = $z->read($buffer, $length, $offset)

    $status = read($z, $buffer, $length)
    $status = read($z, $buffer, $length, $offset)

Attempt to read C<$length> bytes of uncompressed data into C<$buffer>.

The main difference between this form of the C<read> method and the
previous one, is that this one will attempt to return I<exactly> C<$length>
bytes. The only circumstances that this function will not is if end-of-file
or an IO error is encountered.

Returns the number of uncompressed bytes written to C<$buffer>, zero if eof
or a negative number on error.

=head2 getline

Usage is

    $line = $z->getline()
    $line = <$z>

Reads a single line.

This method fully supports the use of the variable C<$/> (or
C<$INPUT_RECORD_SEPARATOR> or C<$RS> when C<English> is in use) to
determine what constitutes an end of line. Paragraph mode, record mode and
file slurp mode are all supported.

=head2 getc

Usage is

    $char = $z->getc()

Read a single character.

=head2 ungetc

Usage is

    $char = $z->ungetc($string)

=head2 inflateSync

Usage is

    $status = $z->inflateSync()

TODO

=head2 getHeaderInfo

Usage is

    $hdr  = $z->getHeaderInfo();
    @hdrs = $z->getHeaderInfo();

This method returns either a hash reference (in scalar context) or a list
or hash references (in array context) that contains information about each
of the header fields in the compressed data stream(s).

=head2 tell

Usage is

    $z->tell()
    tell $z

Returns the uncompressed file offset.

=head2 eof

Usage is

    $z->eof();
    eof($z);

Returns true if the end of the compressed input stream has been reached.

=head2 seek

    $z->seek($position, $whence);
    seek($z, $position, $whence);

Provides a sub-set of the C<seek> functionality, with the restriction
that it is only legal to seek forward in the input file/buffer.
It is a fatal error to attempt to seek backward.

Note that the implementation of C<seek> in this module does not provide
true random access to a compressed file/buffer. It  works by uncompressing
data from the current offset in the file/buffer until it reaches the
uncompressed offset specified in the parameters to C<seek>. For very small
files this may be acceptable behaviour. For large files it may cause an
unacceptable delay.

The C<$whence> parameter takes one the usual values, namely SEEK_SET,
SEEK_CUR or SEEK_END.

Returns 1 on success, 0 on failure.

=head2 binmode

Usage is

    $z->binmode
    binmode $z ;

This is a noop provided for completeness.

=head2 opened

    $z->opened()

Returns true if the object currently refers to a opened file/buffer.

=head2 autoflush

    my $prev = $z->autoflush()
    my $prev = $z->autoflush(EXPR)

If the C<$z> object is associated with a file or a filehandle, this method
returns the current autoflush setting for the underlying filehandle. If
C<EXPR> is present, and is non-zero, it will enable flushing after every
write/print operation.

If C<$z> is associated with a buffer, this method has no effect and always
returns C<undef>.

B<Note> that the special variable C<$|> B<cannot> be used to set or
retrieve the autoflush setting.

=head2 input_line_number

    $z->input_line_number()
    $z->input_line_number(EXPR)

Returns the current uncompressed line number. If C<EXPR> is present it has
the effect of setting the line number. Note that setting the line number
does not change the current position within the file/buffer being read.

The contents of C<$/> are used to determine what constitutes a line
terminator.

=head2 fileno

    $z->fileno()
    fileno($z)

If the C<$z> object is associated with a file or a filehandle, C<fileno>
will return the underlying file descriptor. Once the C<close> method is
called C<fileno> will return C<undef>.

If the C<$z> object is associated with a buffer, this method will return
C<undef>.

=head2 close

    $z->close() ;
    close $z ;

Closes the output file/buffer.

For most versions of Perl this method will be automatically invoked if
the IO::Uncompress::Unzip object is destroyed (either explicitly or by the
variable with the reference to the object going out of scope). The
exceptions are Perl versions 5.005 through 5.00504 and 5.8.0. In
these cases, the C<close> method will be called automatically, but
not until global destruction of all live objects when the program is
terminating.

Therefore, if you want your scripts to be able to run on all versions
of Perl, you should call C<close> explicitly and not rely on automatic
closing.

Returns true on success, otherwise 0.

If the C<AutoClose> option has been enabled when the IO::Uncompress::Unzip
object was created, and the object is associated with a file, the
underlying file will also be closed.

=head2 nextStream

Usage is

    my $status = $z->nextStream();

Skips to the next compressed data stream in the input file/buffer. If a new
compressed data stream is found, the eof marker will be cleared and C<$.>
will be reset to 0.

If trailing data is present immediately after the zip archive and the
C<Transparent> option is enabled, this method will consider that trailing
data to be another member of the zip archive.

Returns 1 if a new stream was found, 0 if none was found, and -1 if an
error was encountered.

=head2 trailingData

Usage is

    my $data = $z->trailingData();

Returns the data, if any, that is present immediately after the compressed
data stream once uncompression is complete. It only makes sense to call
this method once the end of the compressed data stream has been
encountered.

This option can be used when there is useful information immediately
following the compressed data stream, and you don't know the length of the
compressed data stream.

If the input is a buffer, C<trailingData> will return everything from the
end of the compressed data stream to the end of the buffer.

If the input is a filehandle, C<trailingData> will return the data that is
left in the filehandle input buffer once the end of the compressed data
stream has been reached. You can then use the filehandle to read the rest
of the input file.

Don't bother using C<trailingData> if the input is a filename.

If you know the length of the compressed data stream before you start
uncompressing, you can avoid having to use C<trailingData> by setting the
C<InputLength> option in the constructor.

=head1 Importing

No symbolic constants are required by IO::Uncompress::Unzip at present.

=over 5

=item :all

Imports C<unzip> and C<$UnzipError>.
Same as doing this

    use IO::Uncompress::Unzip qw(unzip $UnzipError) ;

=back

=head1 EXAMPLES

=head2 Working with Net::FTP

See L<IO::Compress::FAQ|IO::Compress::FAQ/"Compressed files and Net::FTP">

=head2 Walking through a zip file

The code below can be used to traverse a zip file, one compressed data
stream at a time.

    use IO::Uncompress::Unzip qw($UnzipError);

    my $zipfile = "somefile.zip";
    my $u = IO::Uncompress::Unzip->new( $zipfile )
        or die "Cannot open $zipfile: $UnzipError";

    my $status;
    for ($status = 1; $status > 0; $status = $u->nextStream())
    {

        my $name = $u->getHeaderInfo()->{Name};
        warn "Processing member $name\n" ;

        my $buff;
        while (($status = $u->read($buff)) > 0) {
            # Do something here
        }

        last if $status < 0;
    }

    die "Error processing $zipfile: $!\n"
        if $status < 0 ;

Each individual compressed data stream is read until the logical
end-of-file is reached. Then C<nextStream> is called. This will skip to the
start of the next compressed data stream and clear the end-of-file flag.

It is also worth noting that C<nextStream> can be called at any time -- you
don't have to wait until you have exhausted a compressed data stream before
skipping to the next one.

=head2 Unzipping a complete zip file to disk

Daniel S. Sterling has written a script that uses C<IO::Uncompress::UnZip>
to read a zip file and unzip its contents to disk.

The script is available from L<https://gist.github.com/eqhmcow/5389877>

=head1 SUPPORT

General feedback/questions/bug reports should be sent to
L<https://github.com/pmqs/IO-Compress/issues> (preferred) or
L<https://rt.cpan.org/Public/Dist/Display.html?Name=IO-Compress>.

=head1 SEE ALSO

L<Compress::Zlib>, L<IO::Compress::Gzip>, L<IO::Uncompress::Gunzip>, L<IO::Compress::Deflate>, L<IO::Uncompress::Inflate>, L<IO::Compress::RawDeflate>, L<IO::Uncompress::RawInflate>, L<IO::Compress::Bzip2>, L<IO::Uncompress::Bunzip2>, L<IO::Compress::Lzma>, L<IO::Uncompress::UnLzma>, L<IO::Compress::Xz>, L<IO::Uncompress::UnXz>, L<IO::Compress::Lzip>, L<IO::Uncompress::UnLzip>, L<IO::Compress::Lzop>, L<IO::Uncompress::UnLzop>, L<IO::Compress::Lzf>, L<IO::Uncompress::UnLzf>, L<IO::Compress::Zstd>, L<IO::Uncompress::UnZstd>, L<IO::Uncompress::AnyInflate>, L<IO::Uncompress::AnyUncompress>

L<IO::Compress::FAQ|IO::Compress::FAQ>

L<File::GlobMapper|File::GlobMapper>, L<Archive::Zip|Archive::Zip>,
L<Archive::Tar|Archive::Tar>,
L<IO::Zlib|IO::Zlib>

For RFC 1950, 1951 and 1952 see
L<https://datatracker.ietf.org/doc/html/rfc1950>,
L<https://datatracker.ietf.org/doc/html/rfc1951> and
L<https://datatracker.ietf.org/doc/html/rfc1952>

The I<zlib> compression library was written by Jean-loup Gailly
C<gzip@prep.ai.mit.edu> and Mark Adler C<madler@alumni.caltech.edu>.

The primary site for the I<zlib> compression library is
L<http://www.zlib.org>.

The primary site for the I<zlib-ng> compression library is
L<https://github.com/zlib-ng/zlib-ng>.

The primary site for gzip is L<http://www.gzip.org>.

=head1 AUTHOR

This module was written by Paul Marquess, C<pmqs@cpan.org>.

=head1 MODIFICATION HISTORY

See the Changes file.

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2005-2023 Paul Marquess. All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.
