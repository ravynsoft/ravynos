package IO::Compress::Zip ;

use strict ;
use warnings;
use bytes;

use IO::Compress::Base::Common  2.204 qw(:Status );
use IO::Compress::RawDeflate 2.204 ();
use IO::Compress::Adapter::Deflate 2.204 ;
use IO::Compress::Adapter::Identity 2.204 ;
use IO::Compress::Zlib::Extra 2.204 ;
use IO::Compress::Zip::Constants 2.204 ;

use File::Spec();
use Config;

use Compress::Raw::Zlib  2.204 ();

BEGIN
{
    eval { require IO::Compress::Adapter::Bzip2 ;
           IO::Compress::Adapter::Bzip2->import( 2.201 );
           require IO::Compress::Bzip2 ;
           IO::Compress::Bzip2->import( 2.201 );
         } ;

    eval { require IO::Compress::Adapter::Lzma ;
           IO::Compress::Adapter::Lzma->import( 2.201 );
           require IO::Compress::Lzma ;
           IO::Compress::Lzma->import( 2.201 );
         } ;

    eval { require IO::Compress::Adapter::Xz ;
           IO::Compress::Adapter::Xz->import( 2.201 );
           require IO::Compress::Xz ;
           IO::Compress::Xz->import( 2.201 );
         } ;
    eval { require IO::Compress::Adapter::Zstd ;
           IO::Compress::Adapter::Zstd->import( 2.201 );
           require IO::Compress::Zstd ;
           IO::Compress::Zstd->import( 2.201 );
         } ;
}


require Exporter ;

our ($VERSION, @ISA, @EXPORT_OK, %EXPORT_TAGS, %DEFLATE_CONSTANTS, $ZipError);

$VERSION = '2.204';
$ZipError = '';

@ISA = qw(IO::Compress::RawDeflate Exporter);
@EXPORT_OK = qw( $ZipError zip ) ;
%EXPORT_TAGS = %IO::Compress::RawDeflate::DEFLATE_CONSTANTS ;

push @{ $EXPORT_TAGS{all} }, @EXPORT_OK ;

$EXPORT_TAGS{zip_method} = [qw( ZIP_CM_STORE ZIP_CM_DEFLATE ZIP_CM_BZIP2 ZIP_CM_LZMA ZIP_CM_XZ ZIP_CM_ZSTD)];
push @{ $EXPORT_TAGS{all} }, @{ $EXPORT_TAGS{zip_method} };

Exporter::export_ok_tags('all');

sub new
{
    my $class = shift ;

    my $obj = IO::Compress::Base::Common::createSelfTiedObject($class, \$ZipError);
    $obj->_create(undef, @_);

}

sub zip
{
    my $obj = IO::Compress::Base::Common::createSelfTiedObject(undef, \$ZipError);
    return $obj->_def(@_);
}

sub isMethodAvailable
{
    my $method = shift;

    # Store & Deflate are always available
    return 1
        if $method == ZIP_CM_STORE || $method == ZIP_CM_DEFLATE ;

    return 1
        if $method == ZIP_CM_BZIP2 &&
           defined $IO::Compress::Adapter::Bzip2::VERSION &&
           defined &{ "IO::Compress::Adapter::Bzip2::mkRawZipCompObject" };

    return 1
        if $method == ZIP_CM_LZMA &&
           defined $IO::Compress::Adapter::Lzma::VERSION &&
           defined &{ "IO::Compress::Adapter::Lzma::mkRawZipCompObject" };

    return 1
        if $method == ZIP_CM_XZ &&
           defined $IO::Compress::Adapter::Xz::VERSION &&
           defined &{ "IO::Compress::Adapter::Xz::mkRawZipCompObject" };

    return 1
        if $method == ZIP_CM_ZSTD &&
           defined $IO::Compress::Adapter::ZSTD::VERSION &&
           defined &{ "IO::Compress::Adapter::ZSTD::mkRawZipCompObject" };

    return 0;
}

sub beforePayload
{
    my $self = shift ;

    if (*$self->{ZipData}{Sparse} ) {
        my $inc = 1024 * 100 ;
        my $NULLS = ("\x00" x $inc) ;
        my $sparse = *$self->{ZipData}{Sparse} ;
        *$self->{CompSize}->add( $sparse );
        *$self->{UnCompSize}->add( $sparse );

        *$self->{FH}->seek($sparse, IO::Handle::SEEK_CUR);

        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32($NULLS, *$self->{ZipData}{CRC32})
            for 1 .. int $sparse / $inc;
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(substr($NULLS, 0,  $sparse % $inc),
                                         *$self->{ZipData}{CRC32})
            if $sparse % $inc;
    }
}

sub mkComp
{
    my $self = shift ;
    my $got = shift ;

    my ($obj, $errstr, $errno) ;

    if (*$self->{ZipData}{Method} == ZIP_CM_STORE) {
        ($obj, $errstr, $errno) = IO::Compress::Adapter::Identity::mkCompObject(
                                                 $got->getValue('level'),
                                                 $got->getValue('strategy')
                                                 );
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(undef);
    }
    elsif (*$self->{ZipData}{Method} == ZIP_CM_DEFLATE) {
        ($obj, $errstr, $errno) = IO::Compress::Adapter::Deflate::mkCompObject(
                                                 $got->getValue('crc32'),
                                                 $got->getValue('adler32'),
                                                 $got->getValue('level'),
                                                 $got->getValue('strategy')
                                                 );
    }
    elsif (*$self->{ZipData}{Method} == ZIP_CM_BZIP2) {
        ($obj, $errstr, $errno) = IO::Compress::Adapter::Bzip2::mkCompObject(
                                                $got->getValue('blocksize100k'),
                                                $got->getValue('workfactor'),
                                                $got->getValue('verbosity')
                                               );
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(undef);
    }
    elsif (*$self->{ZipData}{Method} == ZIP_CM_LZMA) {
        ($obj, $errstr, $errno) = IO::Compress::Adapter::Lzma::mkRawZipCompObject($got->getValue('preset'),
                                                                                 $got->getValue('extreme'),
                                                                                 );
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(undef);
    }
    elsif (*$self->{ZipData}{Method} == ZIP_CM_XZ) {
        ($obj, $errstr, $errno) = IO::Compress::Adapter::Xz::mkCompObject($got->getValue('preset'),
                                                                                 $got->getValue('extreme'),
                                                                                 0
                                                                                 );
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(undef);
    }
    elsif (*$self->{ZipData}{Method} == ZIP_CM_ZSTD) {
        ($obj, $errstr, $errno) = IO::Compress::Adapter::Zstd::mkCompObject(defined $got->getValue('level') ? $got->getValue('level') : 3,
                                                                           );
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(undef);
    }

    return $self->saveErrorString(undef, $errstr, $errno)
       if ! defined $obj;

    if (! defined *$self->{ZipData}{SizesOffset}) {
        *$self->{ZipData}{SizesOffset} = 0;
        *$self->{ZipData}{Offset} = U64->new();
    }

    *$self->{ZipData}{AnyZip64} = 0
        if ! defined  *$self->{ZipData}{AnyZip64} ;

    return $obj;
}

sub reset
{
    my $self = shift ;

    *$self->{Compress}->reset();
    *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32('');

    return STATUS_OK;
}

sub filterUncompressed
{
    my $self = shift ;

    if (*$self->{ZipData}{Method} == ZIP_CM_DEFLATE) {
        *$self->{ZipData}{CRC32} = *$self->{Compress}->crc32();
    }
    else {
        *$self->{ZipData}{CRC32} = Compress::Raw::Zlib::crc32(${$_[0]}, *$self->{ZipData}{CRC32});

    }
}

sub canonicalName
{
    # This sub is derived from Archive::Zip::_asZipDirName

    # Return the normalized name as used in a zip file (path
    # separators become slashes, etc.).
    # Will translate internal slashes in path components (i.e. on Macs) to
    # underscores.  Discards volume names.
    # When $forceDir is set, returns paths with trailing slashes
    #
    # input         output
    # .             '.'
    # ./a           a
    # ./a/b         a/b
    # ./a/b/        a/b
    # a/b/          a/b
    # /a/b/         a/b
    # c:\a\b\c.doc  a/b/c.doc      # on Windows
    # "i/o maps:whatever"   i_o maps/whatever   # on Macs

    my $name      = shift;
    my $forceDir  = shift ;

    my ( $volume, $directories, $file ) =
      File::Spec->splitpath( File::Spec->canonpath($name), $forceDir );

    my @dirs = map { $_ =~ s{/}{_}g; $_ }
               File::Spec->splitdir($directories);

    if ( @dirs > 0 ) { pop (@dirs) if $dirs[-1] eq '' }   # remove empty component
    push @dirs, defined($file) ? $file : '' ;

    my $normalised_path = join '/', @dirs;

    # Leading directory separators should not be stored in zip archives.
    # Example:
    #   C:\a\b\c\      a/b/c
    #   C:\a\b\c.txt   a/b/c.txt
    #   /a/b/c/        a/b/c
    #   /a/b/c.txt     a/b/c.txt
    $normalised_path =~ s{^/}{};  # remove leading separator

    return $normalised_path;
}


sub mkHeader
{
    my $self  = shift;
    my $param = shift ;

    *$self->{ZipData}{LocalHdrOffset} = U64::clone(*$self->{ZipData}{Offset});

    my $comment = '';
    $comment = $param->valueOrDefault('comment') ;

    my $filename = '';
    $filename = $param->valueOrDefault('name') ;

    $filename = canonicalName($filename)
        if length $filename && $param->getValue('canonicalname') ;

    if (defined *$self->{ZipData}{FilterName} ) {
        local *_ = \$filename ;
        &{ *$self->{ZipData}{FilterName} }() ;
    }

   if ( $param->getValue('efs') && $] >= 5.008004) {
        if (length $filename) {
            utf8::downgrade($filename, 1)
                or Carp::croak "Wide character in zip filename";
        }

        if (length $comment) {
            utf8::downgrade($comment, 1)
                or Carp::croak "Wide character in zip comment";
        }
   }

    my $hdr = '';

    my $time = _unixToDosTime($param->getValue('time'));

    my $extra = '';
    my $ctlExtra = '';
    my $empty = 0;
    my $osCode = $param->getValue('os_code') ;
    my $extFileAttr = 0 ;

    # This code assumes Unix.
    # TODO - revisit this
    $extFileAttr = 0100644 << 16
        if $osCode == ZIP_OS_CODE_UNIX ;

    if (*$self->{ZipData}{Zip64}) {
        $empty = IO::Compress::Base::Common::MAX32;

        my $x = '';
        $x .= pack "V V", 0, 0 ; # uncompressedLength
        $x .= pack "V V", 0, 0 ; # compressedLength

        # Zip64 needs to be first in extra field to workaround a Windows Explorer Bug
        # See http://www.info-zip.org/phpBB3/viewtopic.php?f=3&t=440 for details
        $extra .= IO::Compress::Zlib::Extra::mkSubField(ZIP_EXTRA_ID_ZIP64, $x);
    }

    if (! $param->getValue('minimal')) {
        if ($param->parsed('mtime'))
        {
            $extra .= mkExtendedTime($param->getValue('mtime'),
                                    $param->getValue('atime'),
                                    $param->getValue('ctime'));

            $ctlExtra .= mkExtendedTime($param->getValue('mtime'));
        }

        if ( $osCode == ZIP_OS_CODE_UNIX )
        {
            if ( $param->getValue('want_exunixn') )
            {
                    my $ux3 = mkUnixNExtra( @{ $param->getValue('want_exunixn') });
                    $extra    .= $ux3;
                    $ctlExtra .= $ux3;
            }

            if ( $param->getValue('exunix2') )
            {
                    $extra    .= mkUnix2Extra( @{ $param->getValue('exunix2') });
                    $ctlExtra .= mkUnix2Extra();
            }
        }

        $extFileAttr = $param->getValue('extattr')
            if defined $param->getValue('extattr') ;

        $extra .= $param->getValue('extrafieldlocal')
            if defined $param->getValue('extrafieldlocal');

        $ctlExtra .= $param->getValue('extrafieldcentral')
            if defined $param->getValue('extrafieldcentral');
    }

    my $method = *$self->{ZipData}{Method} ;
    my $gpFlag = 0 ;
    $gpFlag |= ZIP_GP_FLAG_STREAMING_MASK
        if *$self->{ZipData}{Stream} ;

    $gpFlag |= ZIP_GP_FLAG_LZMA_EOS_PRESENT
        if $method == ZIP_CM_LZMA ;

    $gpFlag |= ZIP_GP_FLAG_LANGUAGE_ENCODING
        if  $param->getValue('efs') && (length($filename) || length($comment));

    my $version = $ZIP_CM_MIN_VERSIONS{$method};
    $version = ZIP64_MIN_VERSION
        if ZIP64_MIN_VERSION > $version && *$self->{ZipData}{Zip64};

    my $madeBy = ($param->getValue('os_code') << 8) + $version;
    my $extract = $version;

    *$self->{ZipData}{Version} = $version;
    *$self->{ZipData}{MadeBy} = $madeBy;

    my $ifa = 0;
    $ifa |= ZIP_IFA_TEXT_MASK
        if $param->getValue('textflag');

    $hdr .= pack "V", ZIP_LOCAL_HDR_SIG ; # signature
    $hdr .= pack 'v', $extract   ; # extract Version & OS
    $hdr .= pack 'v', $gpFlag    ; # general purpose flag (set streaming mode)
    $hdr .= pack 'v', $method    ; # compression method (deflate)
    $hdr .= pack 'V', $time      ; # last mod date/time
    $hdr .= pack 'V', 0          ; # crc32               - 0 when streaming
    $hdr .= pack 'V', $empty     ; # compressed length   - 0 when streaming
    $hdr .= pack 'V', $empty     ; # uncompressed length - 0 when streaming
    $hdr .= pack 'v', length $filename ; # filename length
    $hdr .= pack 'v', length $extra ; # extra length

    $hdr .= $filename ;

    # Remember the offset for the compressed & uncompressed lengths in the
    # local header.
    if (*$self->{ZipData}{Zip64}) {
        *$self->{ZipData}{SizesOffset} = *$self->{ZipData}{Offset}->get64bit()
            + length($hdr) + 4 ;
    }
    else {
        *$self->{ZipData}{SizesOffset} = *$self->{ZipData}{Offset}->get64bit()
                                            + 18;
    }

    $hdr .= $extra ;


    my $ctl = '';

    $ctl .= pack "V", ZIP_CENTRAL_HDR_SIG ; # signature
    $ctl .= pack 'v', $madeBy    ; # version made by
    $ctl .= pack 'v', $extract   ; # extract Version
    $ctl .= pack 'v', $gpFlag    ; # general purpose flag (streaming mode)
    $ctl .= pack 'v', $method    ; # compression method (deflate)
    $ctl .= pack 'V', $time      ; # last mod date/time
    $ctl .= pack 'V', 0          ; # crc32
    $ctl .= pack 'V', $empty     ; # compressed length
    $ctl .= pack 'V', $empty     ; # uncompressed length
    $ctl .= pack 'v', length $filename ; # filename length

    *$self->{ZipData}{ExtraOffset} = length $ctl;
    *$self->{ZipData}{ExtraSize} = length $ctlExtra ;

    $ctl .= pack 'v', length $ctlExtra ; # extra length
    $ctl .= pack 'v', length $comment ;  # file comment length
    $ctl .= pack 'v', 0          ; # disk number start
    $ctl .= pack 'v', $ifa       ; # internal file attributes
    $ctl .= pack 'V', $extFileAttr   ; # external file attributes

    # offset to local hdr
    if (*$self->{ZipData}{LocalHdrOffset}->is64bit() ) {
        $ctl .= pack 'V', IO::Compress::Base::Common::MAX32 ;
    }
    else {
        $ctl .= *$self->{ZipData}{LocalHdrOffset}->getPacked_V32() ;
    }

    $ctl .= $filename ;

    *$self->{ZipData}{Offset}->add32(length $hdr) ;

    *$self->{ZipData}{CentralHeader} = [ $ctl, $ctlExtra, $comment];

    return $hdr;
}

sub mkTrailer
{
    my $self = shift ;

    my $crc32 ;
    if (*$self->{ZipData}{Method} == ZIP_CM_DEFLATE) {
        $crc32 = pack "V", *$self->{Compress}->crc32();
    }
    else {
        $crc32 = pack "V", *$self->{ZipData}{CRC32};
    }

    my ($ctl, $ctlExtra, $comment) = @{ *$self->{ZipData}{CentralHeader} };

    my $sizes ;
    if (! *$self->{ZipData}{Zip64}) {
        $sizes .= *$self->{CompSize}->getPacked_V32() ;   # Compressed size
        $sizes .= *$self->{UnCompSize}->getPacked_V32() ; # Uncompressed size
    }
    else {
        $sizes .= *$self->{CompSize}->getPacked_V64() ;   # Compressed size
        $sizes .= *$self->{UnCompSize}->getPacked_V64() ; # Uncompressed size
    }

    my $data = $crc32 . $sizes ;

    my $xtrasize  = *$self->{UnCompSize}->getPacked_V64() ; # Uncompressed size
       $xtrasize .= *$self->{CompSize}->getPacked_V64() ;   # Compressed size

    my $hdr = '';

    if (*$self->{ZipData}{Stream}) {
        $hdr  = pack "V", ZIP_DATA_HDR_SIG ;                       # signature
        $hdr .= $data ;
    }
    else {
        $self->writeAt(*$self->{ZipData}{LocalHdrOffset}->get64bit() + 14,  $crc32)
            or return undef;
        $self->writeAt(*$self->{ZipData}{SizesOffset},
                *$self->{ZipData}{Zip64} ? $xtrasize : $sizes)
            or return undef;
    }

    # Central Header Record/Zip64 extended field

    substr($ctl, 16, length $crc32) = $crc32 ;

    my $zip64Payload = '';

    # uncompressed length - only set zip64 if needed
    if (*$self->{UnCompSize}->isAlmost64bit()) { #  || *$self->{ZipData}{Zip64}) {
        $zip64Payload .= *$self->{UnCompSize}->getPacked_V64() ;
    } else {
        substr($ctl, 24, 4) = *$self->{UnCompSize}->getPacked_V32() ;
    }

    # compressed length - only set zip64 if needed
    if (*$self->{CompSize}->isAlmost64bit()) { # || *$self->{ZipData}{Zip64}) {
        $zip64Payload .= *$self->{CompSize}->getPacked_V64() ;
    } else {
        substr($ctl, 20, 4) = *$self->{CompSize}->getPacked_V32() ;
    }

    # Local Header offset
    $zip64Payload .= *$self->{ZipData}{LocalHdrOffset}->getPacked_V64()
        if *$self->{ZipData}{LocalHdrOffset}->is64bit() ;

    # disk no - always zero, so don't need to include it.
    #$zip64Payload .= pack "V", 0    ;

    my $zip64Xtra = '';

    if (length $zip64Payload) {
        $zip64Xtra = IO::Compress::Zlib::Extra::mkSubField(ZIP_EXTRA_ID_ZIP64, $zip64Payload);

        substr($ctl, *$self->{ZipData}{ExtraOffset}, 2) =
             pack 'v', *$self->{ZipData}{ExtraSize} + length $zip64Xtra;

        *$self->{ZipData}{AnyZip64} = 1;
    }

    # Zip64 needs to be first in extra field to workaround a Windows Explorer Bug
    # See http://www.info-zip.org/phpBB3/viewtopic.php?f=3&t=440 for details
    $ctl .= $zip64Xtra . $ctlExtra . $comment;

    *$self->{ZipData}{Offset}->add32(length($hdr));
    *$self->{ZipData}{Offset}->add( *$self->{CompSize} );
    push @{ *$self->{ZipData}{CentralDir} }, $ctl ;

    return $hdr;
}

sub mkFinalTrailer
{
    my $self = shift ;

    my $comment = '';
    $comment = *$self->{ZipData}{ZipComment} ;

    my $cd_offset = *$self->{ZipData}{Offset}->get32bit() ; # offset to start central dir

    my $entries = @{ *$self->{ZipData}{CentralDir} };

    *$self->{ZipData}{AnyZip64} = 1
        if *$self->{ZipData}{Offset}->is64bit || $entries >= 0xFFFF ;

    my $cd = join '', @{ *$self->{ZipData}{CentralDir} };
    my $cd_len = length $cd ;

    my $z64e = '';

    if ( *$self->{ZipData}{AnyZip64} ) {

        my $v  = *$self->{ZipData}{Version} ;
        my $mb = *$self->{ZipData}{MadeBy} ;
        $z64e .= pack 'v', $mb            ; # Version made by
        $z64e .= pack 'v', $v             ; # Version to extract
        $z64e .= pack 'V', 0              ; # number of disk
        $z64e .= pack 'V', 0              ; # number of disk with central dir
        $z64e .= U64::pack_V64 $entries   ; # entries in central dir on this disk
        $z64e .= U64::pack_V64 $entries   ; # entries in central dir
        $z64e .= U64::pack_V64 $cd_len    ; # size of central dir
        $z64e .= *$self->{ZipData}{Offset}->getPacked_V64() ; # offset to start central dir
        $z64e .= *$self->{ZipData}{extrafieldzip64}  # otional extra field
            if defined *$self->{ZipData}{extrafieldzip64} ;

        $z64e  = pack("V", ZIP64_END_CENTRAL_REC_HDR_SIG) # signature
              .  U64::pack_V64(length $z64e)
              .  $z64e ;

        *$self->{ZipData}{Offset}->add32(length $cd) ;

        $z64e .= pack "V", ZIP64_END_CENTRAL_LOC_HDR_SIG; # signature
        $z64e .= pack 'V', 0              ; # number of disk with central dir
        $z64e .= *$self->{ZipData}{Offset}->getPacked_V64() ; # offset to end zip64 central dir
        $z64e .= pack 'V', 1              ; # Total number of disks

        $cd_offset = IO::Compress::Base::Common::MAX32 ;
        $cd_len = IO::Compress::Base::Common::MAX32 if IO::Compress::Base::Common::isGeMax32 $cd_len ;
        $entries = 0xFFFF if $entries >= 0xFFFF ;
    }

    my $ecd = '';
    $ecd .= pack "V", ZIP_END_CENTRAL_HDR_SIG ; # signature
    $ecd .= pack 'v', 0          ; # number of disk
    $ecd .= pack 'v', 0          ; # number of disk with central dir
    $ecd .= pack 'v', $entries   ; # entries in central dir on this disk
    $ecd .= pack 'v', $entries   ; # entries in central dir
    $ecd .= pack 'V', $cd_len    ; # size of central dir
    $ecd .= pack 'V', $cd_offset ; # offset to start central dir
    $ecd .= pack 'v', length $comment ; # zipfile comment length
    $ecd .= $comment;

    return $cd . $z64e . $ecd ;
}

sub ckParams
{
    my $self = shift ;
    my $got = shift;

    $got->setValue('crc32' => 1);

    if (! $got->parsed('time') ) {
        # Modification time defaults to now.
        $got->setValue('time' => time) ;
    }

    if ($got->parsed('extime') ) {
        my $timeRef = $got->getValue('extime');
        if ( defined $timeRef) {
            return $self->saveErrorString(undef, "exTime not a 3-element array ref")
                if ref $timeRef ne 'ARRAY' || @$timeRef != 3;
        }

        $got->setValue("mtime", $timeRef->[1]);
        $got->setValue("atime", $timeRef->[0]);
        $got->setValue("ctime", $timeRef->[2]);
    }

    # Unix2/3 Extended Attribute
    for my $name (qw(exunix2 exunixn))
    {
        if ($got->parsed($name) ) {
            my $idRef = $got->getValue($name);
            if ( defined $idRef) {
                return $self->saveErrorString(undef, "$name not a 2-element array ref")
                    if ref $idRef ne 'ARRAY' || @$idRef != 2;
            }

            $got->setValue("uid", $idRef->[0]);
            $got->setValue("gid", $idRef->[1]);
            $got->setValue("want_$name", $idRef);
        }
    }

    *$self->{ZipData}{AnyZip64} = 1
        if $got->getValue('zip64') || $got->getValue('extrafieldzip64') ;
    *$self->{ZipData}{Zip64} = $got->getValue('zip64');
    *$self->{ZipData}{Stream} = $got->getValue('stream');

    my $method = $got->getValue('method');
    return $self->saveErrorString(undef, "Unknown Method '$method'")
        if ! defined $ZIP_CM_MIN_VERSIONS{$method};

    return $self->saveErrorString(undef, "Bzip2 not available")
        if $method == ZIP_CM_BZIP2 and
           ! defined $IO::Compress::Adapter::Bzip2::VERSION;

    return $self->saveErrorString(undef, "Lzma not available")
        if $method == ZIP_CM_LZMA
        and ! defined $IO::Compress::Adapter::Lzma::VERSION;

    *$self->{ZipData}{Method} = $method;

    *$self->{ZipData}{ZipComment} = $got->getValue('zipcomment') ;

    for my $name (qw( extrafieldlocal extrafieldcentral extrafieldzip64))
    {
        my $data = $got->getValue($name) ;
        if (defined $data) {
            my $bad = IO::Compress::Zlib::Extra::parseExtraField($data, 1, 0) ;
            return $self->saveErrorString(undef, "Error with $name Parameter: $bad")
                if $bad ;

            $got->setValue($name, $data) ;
            *$self->{ZipData}{$name} = $data;
        }
    }

    return undef
        if defined $IO::Compress::Bzip2::VERSION
            and ! IO::Compress::Bzip2::ckParams($self, $got);

    if ($got->parsed('sparse') ) {
        *$self->{ZipData}{Sparse} = $got->getValue('sparse') ;
        *$self->{ZipData}{Method} = ZIP_CM_STORE;
    }

    if ($got->parsed('filtername')) {
        my $v = $got->getValue('filtername') ;
        *$self->{ZipData}{FilterName} = $v
            if ref $v eq 'CODE' ;
    }

    return 1 ;
}

sub outputPayload
{
    my $self = shift ;
    return 1 if *$self->{ZipData}{Sparse} ;
    return $self->output(@_);
}


#sub newHeader
#{
#    my $self = shift ;
#
#    return $self->mkHeader(*$self->{Got});
#}


our %PARAMS = (
            'stream'    => [IO::Compress::Base::Common::Parse_boolean,   1],
           #'store'     => [IO::Compress::Base::Common::Parse_boolean,   0],
            'method'    => [IO::Compress::Base::Common::Parse_unsigned,  ZIP_CM_DEFLATE],

#            # Zip header fields
            'minimal'   => [IO::Compress::Base::Common::Parse_boolean,   0],
            'zip64'     => [IO::Compress::Base::Common::Parse_boolean,   0],
            'comment'   => [IO::Compress::Base::Common::Parse_any,       ''],
            'zipcomment'=> [IO::Compress::Base::Common::Parse_any,       ''],
            'name'      => [IO::Compress::Base::Common::Parse_any,       ''],
            'filtername'=> [IO::Compress::Base::Common::Parse_code,      undef],
            'canonicalname'=> [IO::Compress::Base::Common::Parse_boolean,   0],
            'efs'       => [IO::Compress::Base::Common::Parse_boolean,   0],
            'time'      => [IO::Compress::Base::Common::Parse_any,       undef],
            'extime'    => [IO::Compress::Base::Common::Parse_any,       undef],
            'exunix2'   => [IO::Compress::Base::Common::Parse_any,       undef],
            'exunixn'   => [IO::Compress::Base::Common::Parse_any,       undef],
            'extattr'   => [IO::Compress::Base::Common::Parse_any,
                    $Compress::Raw::Zlib::gzip_os_code == 3
                        ? 0100644 << 16
                        : 0],
            'os_code'   => [IO::Compress::Base::Common::Parse_unsigned,  $Compress::Raw::Zlib::gzip_os_code],

            'textflag'  => [IO::Compress::Base::Common::Parse_boolean,   0],
            'extrafieldlocal'  => [IO::Compress::Base::Common::Parse_any,    undef],
            'extrafieldcentral'=> [IO::Compress::Base::Common::Parse_any,    undef],
            'extrafieldzip64'  => [IO::Compress::Base::Common::Parse_any,    undef],

            # Lzma
            'preset'   => [IO::Compress::Base::Common::Parse_unsigned, 6],
            'extreme'  => [IO::Compress::Base::Common::Parse_boolean,  0],

            # For internal use only
            'sparse'    => [IO::Compress::Base::Common::Parse_unsigned,  0],

            IO::Compress::RawDeflate::getZlibParams(),
            defined $IO::Compress::Bzip2::VERSION
                ? IO::Compress::Bzip2::getExtraParams()
                : ()


                );

sub getExtraParams
{
    return %PARAMS ;
}

sub getInverseClass
{
    no warnings 'once';
    return ('IO::Uncompress::Unzip',
                \$IO::Uncompress::Unzip::UnzipError);
}

sub getFileInfo
{
    my $self = shift ;
    my $params = shift;
    my $filename = shift ;

    if (IO::Compress::Base::Common::isaScalar($filename))
    {
        $params->setValue(zip64 => 1)
            if IO::Compress::Base::Common::isGeMax32 length (${ $filename }) ;

        return ;
    }

    my ($mode, $uid, $gid, $size, $atime, $mtime, $ctime) ;
    if ( $params->parsed('storelinks') )
    {
        ($mode, $uid, $gid, $size, $atime, $mtime, $ctime)
                = (lstat($filename))[2, 4,5,7, 8,9,10] ;
    }
    else
    {
        ($mode, $uid, $gid, $size, $atime, $mtime, $ctime)
                = (stat($filename))[2, 4,5,7, 8,9,10] ;
    }

    $params->setValue(textflag => -T $filename )
        if ! $params->parsed('textflag');

    $params->setValue(zip64 => 1)
        if IO::Compress::Base::Common::isGeMax32 $size ;

    $params->setValue('name' => $filename)
        if ! $params->parsed('name') ;

    $params->setValue('time' => $mtime)
        if ! $params->parsed('time') ;

    if ( ! $params->parsed('extime'))
    {
        $params->setValue('mtime' => $mtime) ;
        $params->setValue('atime' => $atime) ;
        $params->setValue('ctime' => undef) ; # No Creation time
        # TODO - see if can fillout creation time on non-Unix
    }

    # NOTE - Unix specific code alert
    if (! $params->parsed('extattr'))
    {
        use Fcntl qw(:mode) ;
        my $attr = $mode << 16;
        $attr |= ZIP_A_RONLY if ($mode & S_IWRITE) == 0 ;
        $attr |= ZIP_A_DIR   if ($mode & S_IFMT  ) == S_IFDIR ;

        $params->setValue('extattr' => $attr);
    }

    $params->setValue('want_exunixn', [$uid, $gid]);
    $params->setValue('uid' => $uid) ;
    $params->setValue('gid' => $gid) ;

}

sub mkExtendedTime
{
    # order expected is m, a, c

    my $times = '';
    my $bit = 1 ;
    my $flags = 0;

    for my $time (@_)
    {
        if (defined $time)
        {
            $flags |= $bit;
            $times .= pack("V", $time);
        }

        $bit <<= 1 ;
    }

    return IO::Compress::Zlib::Extra::mkSubField(ZIP_EXTRA_ID_EXT_TIMESTAMP,
                                                 pack("C", $flags) .  $times);
}

sub mkUnix2Extra
{
    my $ids = '';
    for my $id (@_)
    {
        $ids .= pack("v", $id);
    }

    return IO::Compress::Zlib::Extra::mkSubField(ZIP_EXTRA_ID_INFO_ZIP_UNIX2,
                                                 $ids);
}

sub mkUnixNExtra
{
    my $uid = shift;
    my $gid = shift;

    # Assumes UID/GID are 32-bit
    my $ids ;
    $ids .= pack "C", 1; # version
    $ids .= pack "C", $Config{uidsize};
    $ids .= pack "V", $uid;
    $ids .= pack "C", $Config{gidsize};
    $ids .= pack "V", $gid;

    return IO::Compress::Zlib::Extra::mkSubField(ZIP_EXTRA_ID_INFO_ZIP_UNIXN,
                                                 $ids);
}


# from Archive::Zip
sub _unixToDosTime    # Archive::Zip::Member
{
	my $time_t = shift;

    # TODO - add something to cope with unix time < 1980
	my ( $sec, $min, $hour, $mday, $mon, $year ) = localtime($time_t);
	my $dt = 0;
	$dt += ( $sec >> 1 );
	$dt += ( $min << 5 );
	$dt += ( $hour << 11 );
	$dt += ( $mday << 16 );
	$dt += ( ( $mon + 1 ) << 21 );
	$dt += ( ( $year - 80 ) << 25 );
	return $dt;
}

1;

__END__

=head1 NAME

IO::Compress::Zip - Write zip files/buffers

=head1 SYNOPSIS

    use IO::Compress::Zip qw(zip $ZipError) ;

    my $status = zip $input => $output [,OPTS]
        or die "zip failed: $ZipError\n";

    my $z = IO::Compress::Zip->new( $output [,OPTS] )
        or die "zip failed: $ZipError\n";

    $z->print($string);
    $z->printf($format, $string);
    $z->write($string);
    $z->syswrite($string [, $length, $offset]);
    $z->flush();
    $z->tell();
    $z->eof();
    $z->seek($position, $whence);
    $z->binmode();
    $z->fileno();
    $z->opened();
    $z->autoflush();
    $z->input_line_number();
    $z->newStream( [OPTS] );

    $z->deflateParams();

    $z->close() ;

    $ZipError ;

    # IO::File mode

    print $z $string;
    printf $z $format, $string;
    tell $z
    eof $z
    seek $z, $position, $whence
    binmode $z
    fileno $z
    close $z ;

=head1 DESCRIPTION

This module provides a Perl interface that allows writing zip
compressed data to files or buffer.

The primary purpose of this module is to provide streaming write access to
zip files and buffers.

At present the following compression methods are supported by IO::Compress::Zip

=over 5

=item Store (0)

=item Deflate (8)

=item Bzip2 (12)

To write Bzip2 content, the module C<IO::Uncompress::Bunzip2> must
be installed.

=item Lzma (14)

To write LZMA content, the module C<IO::Uncompress::UnLzma> must
be installed.

=item Zstandard (93)

To write Zstandard content, the module C<IO::Compress::Zstd> must
be installed.

=item Xz (95)

To write Xz content, the module C<IO::Uncompress::UnXz> must
be installed.

=back

For reading zip files/buffers, see the companion module
L<IO::Uncompress::Unzip|IO::Uncompress::Unzip>.

=head1 Functional Interface

A top-level function, C<zip>, is provided to carry out
"one-shot" compression between buffers and/or files. For finer
control over the compression process, see the L</"OO Interface">
section.

    use IO::Compress::Zip qw(zip $ZipError) ;

    zip $input_filename_or_reference => $output_filename_or_reference [,OPTS]
        or die "zip failed: $ZipError\n";

The functional interface needs Perl5.005 or better.

=head2 zip $input_filename_or_reference => $output_filename_or_reference [, OPTS]

C<zip> expects at least two parameters,
C<$input_filename_or_reference> and C<$output_filename_or_reference>
and zero or more optional parameters (see L</Optional Parameters>)

=head3 The C<$input_filename_or_reference> parameter

The parameter, C<$input_filename_or_reference>, is used to define the
source of the uncompressed data.

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
contains valid filenames before any data is compressed.

=item An Input FileGlob string

If C<$input_filename_or_reference> is a string that is delimited by the
characters "<" and ">" C<zip> will assume that it is an
I<input fileglob string>. The input is the list of files that match the
fileglob.

See L<File::GlobMapper|File::GlobMapper> for more details.

=back

If the C<$input_filename_or_reference> parameter is any other type,
C<undef> will be returned.

In addition, if C<$input_filename_or_reference> corresponds to a filename
from the filesystem, a number of zip file header fields will be populated by default
using the following attributes from the input file

=over 5

=item * the full filename contained in C<$input_filename_or_reference>

=item * the file protection attributes

=item * the UID/GID for the file

=item * the file timestamps

=back

If you do not want to use these defaults they can be overridden by
explicitly setting one, or more, of the C<Name>, C<Time>, C<TextFlag>, C<ExtAttr>, C<exUnixN> and C<exTime> options or by setting the
C<Minimal> parameter.

=head3 The C<$output_filename_or_reference> parameter

The parameter C<$output_filename_or_reference> is used to control the
destination of the compressed data. This parameter can take one of
these forms.

=over 5

=item A filename

If the C<$output_filename_or_reference> parameter is a simple scalar, it is
assumed to be a filename.  This file will be opened for writing and the
compressed data will be written to it.

=item A filehandle

If the C<$output_filename_or_reference> parameter is a filehandle, the
compressed data will be written to it.  The string '-' can be used as
an alias for standard output.

=item A scalar reference

If C<$output_filename_or_reference> is a scalar reference, the
compressed data will be stored in C<$$output_filename_or_reference>.

=item An Array Reference

If C<$output_filename_or_reference> is an array reference,
the compressed data will be pushed onto the array.

=item An Output FileGlob

If C<$output_filename_or_reference> is a string that is delimited by the
characters "<" and ">" C<zip> will assume that it is an
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

When C<$input_filename_or_reference> maps to multiple files/buffers and
C<$output_filename_or_reference> is a single
file/buffer the input files/buffers will each be stored
in C<$output_filename_or_reference> as a distinct entry.

=head2 Optional Parameters

The optional parameters for the one-shot function C<zip>
are (for the most part) identical to those used with the OO interface defined in the
L</"Constructor Options"> section. The exceptions are listed below

=over 5

=item C<< AutoClose => 0|1 >>

This option applies to any input or output data streams to
C<zip> that are filehandles.

If C<AutoClose> is specified, and the value is true, it will result in all
input and/or output filehandles being closed once C<zip> has
completed.

This parameter defaults to 0.

=item C<< BinModeIn => 0|1 >>

This option is now a no-op. All files will be read in binmode.

=item C<< Append => 0|1 >>

The behaviour of this option is dependent on the type of output data
stream.

=over 5

=item * A Buffer

If C<Append> is enabled, all compressed data will be append to the end of
the output buffer. Otherwise the output buffer will be cleared before any
compressed data is written to it.

=item * A Filename

If C<Append> is enabled, the file will be opened in append mode. Otherwise
the contents of the file, if any, will be truncated before any compressed
data is written to it.

=item * A Filehandle

If C<Append> is enabled, the filehandle will be positioned to the end of
the file via a call to C<seek> before any compressed data is
written to it.  Otherwise the file pointer will not be moved.

=back

When C<Append> is specified, and set to true, it will I<append> all compressed
data to the output data stream.

So when the output is a filehandle it will carry out a seek to the eof
before writing any compressed data. If the output is a filename, it will be opened for
appending. If the output is a buffer, all compressed data will be
appended to the existing buffer.

Conversely when C<Append> is not specified, or it is present and is set to
false, it will operate as follows.

When the output is a filename, it will truncate the contents of the file
before writing any compressed data. If the output is a filehandle
its position will not be changed. If the output is a buffer, it will be
wiped before any compressed data is output.

Defaults to 0.

=back

=head2 Examples

Here are a few example that show the capabilities of the module.

=head3 Streaming

This very simple command line example demonstrates the streaming capabilities of the module.
The code reads data from STDIN, compresses it, and writes the compressed data to STDOUT.

    $ echo hello world | perl -MIO::Compress::Zip=zip -e 'zip \*STDIN => \*STDOUT' >output.zip

The special filename "-" can be used as a standin for both C<\*STDIN> and C<\*STDOUT>,
so the above can be rewritten as

    $ echo hello world | perl -MIO::Compress::Zip=zip -e 'zip "-" => "-"' >output.zip

One problem with creating a zip archive directly from STDIN can be demonstrated by looking at
the contents of the zip file, output.zip, that we have just created.

    $ unzip -l output.zip
    Archive:  output.zip
    Length      Date    Time    Name
    ---------  ---------- -----   ----
        12  2019-08-16 22:21
    ---------                     -------
        12                     1 file

The archive member (filename) used is the empty string.

If that doesn't suit your needs, you can explicitly set the filename used
in the zip archive by specifying the L<Name|"File Naming Options"> option, like so

    echo hello world | perl -MIO::Compress::Zip=zip -e 'zip "-" => "-", Name => "hello.txt"' >output.zip

Now the contents of the zip file looks like this

    $ unzip -l output.zip
    Archive:  output.zip
    Length      Date    Time    Name
    ---------  ---------- -----   ----
        12  2019-08-16 22:22   hello.txt
    ---------                     -------
        12                     1 file

=head3 Compressing a file from the filesystem

To read the contents of the file C<file1.txt> and write the compressed
data to the file C<file1.txt.zip>.

    use strict ;
    use warnings ;
    use IO::Compress::Zip qw(zip $ZipError) ;

    my $input = "file1.txt";
    zip $input => "$input.zip"
        or die "zip failed: $ZipError\n";

=head3 Reading from a Filehandle and writing to an in-memory buffer

To read from an existing Perl filehandle, C<$input>, and write the
compressed data to a buffer, C<$buffer>.

    use strict ;
    use warnings ;
    use IO::Compress::Zip qw(zip $ZipError) ;
    use IO::File ;

    my $input = IO::File->new( "<file1.txt" )
        or die "Cannot open 'file1.txt': $!\n" ;
    my $buffer ;
    zip $input => \$buffer
        or die "zip failed: $ZipError\n";

=head3 Compressing multiple files

To create a zip file, C<output.zip>, that contains the compressed contents
of the files C<alpha.txt> and C<beta.txt>

    use strict ;
    use warnings ;
    use IO::Compress::Zip qw(zip $ZipError) ;

    zip [ 'alpha.txt', 'beta.txt' ] => 'output.zip'
        or die "zip failed: $ZipError\n";

Alternatively, rather than having to explicitly name each of the files that
you want to compress, you could use a fileglob to select all the C<txt>
files in the current directory, as follows

    use strict ;
    use warnings ;
    use IO::Compress::Zip qw(zip $ZipError) ;

    my @files = <*.txt>;
    zip \@files => 'output.zip'
        or die "zip failed: $ZipError\n";

or more succinctly

    zip [ <*.txt> ] => 'output.zip'
        or die "zip failed: $ZipError\n";

=head1 OO Interface

=head2 Constructor

The format of the constructor for C<IO::Compress::Zip> is shown below

    my $z = IO::Compress::Zip->new( $output [,OPTS] )
        or die "IO::Compress::Zip failed: $ZipError\n";

It returns an C<IO::Compress::Zip> object on success and undef on failure.
The variable C<$ZipError> will contain an error message on failure.

If you are running Perl 5.005 or better the object, C<$z>, returned from
IO::Compress::Zip can be used exactly like an L<IO::File|IO::File> filehandle.
This means that all normal output file operations can be carried out
with C<$z>.
For example, to write to a compressed file/buffer you can use either of
these forms

    $z->print("hello world\n");
    print $z "hello world\n";

The mandatory parameter C<$output> is used to control the destination
of the compressed data. This parameter can take one of these forms.

=over 5

=item A filename

If the C<$output> parameter is a simple scalar, it is assumed to be a
filename. This file will be opened for writing and the compressed data
will be written to it.

=item A filehandle

If the C<$output> parameter is a filehandle, the compressed data will be
written to it.
The string '-' can be used as an alias for standard output.

=item A scalar reference

If C<$output> is a scalar reference, the compressed data will be stored
in C<$$output>.

=back

If the C<$output> parameter is any other type, C<IO::Compress::Zip>::new will
return undef.

=head2 Constructor Options

C<OPTS> is any combination of zero or more the following options:

=over 5

=item C<< AutoClose => 0|1 >>

This option is only valid when the C<$output> parameter is a filehandle. If
specified, and the value is true, it will result in the C<$output> being
closed once either the C<close> method is called or the C<IO::Compress::Zip>
object is destroyed.

This parameter defaults to 0.

=item C<< Append => 0|1 >>

Opens C<$output> in append mode.

The behaviour of this option is dependent on the type of C<$output>.

=over 5

=item * A Buffer

If C<$output> is a buffer and C<Append> is enabled, all compressed data
will be append to the end of C<$output>. Otherwise C<$output> will be
cleared before any data is written to it.

=item * A Filename

If C<$output> is a filename and C<Append> is enabled, the file will be
opened in append mode. Otherwise the contents of the file, if any, will be
truncated before any compressed data is written to it.

=item * A Filehandle

If C<$output> is a filehandle, the file pointer will be positioned to the
end of the file via a call to C<seek> before any compressed data is written
to it.  Otherwise the file pointer will not be moved.

=back

This parameter defaults to 0.

=back

=head3 File Naming Options

A quick bit of zip file terminology -- A zip archive consists of one or more I<archive members>, where each member has an associated
filename, known as the I<archive member name>.

The options listed in this section control how the I<archive member name> (or filename) is stored the zip archive.

=over 5

=item C<< Name => $string >>

This option is used to explicitly set the I<archive member name> in
the zip archive to C<$string>.
Most of the time you don't need to make use of this option.
By default when adding a filename to the zip archive, the I<archive member name> will match the filename.

You should only need to use this option if you want the I<archive member name>
to be different from the uncompressed filename or when the input is a filehandle or a buffer.

The default behaviour for what I<archive member name> is used when the C<Name> option
is I<not> specified depends on the form of the C<$input> parameter:

=over 5

=item *

If the C<$input> parameter is a filename, the
value of C<$input> will be used for the I<archive member name> .

=item *
If the C<$input> parameter is not a filename,
the I<archive member name> will be an empty string.

=back

Note that both the C<CanonicalName> and C<FilterName> options
can modify the value used for the I<archive member name>.

Also note that you should set the C<Efs> option to true if you are working
with UTF8 filenames.

=item C<< CanonicalName => 0|1 >>

This option controls whether the I<archive member name> is
I<normalized> into Unix format before being written to the zip file.

It is recommended that you enable this option unless you really need
to create a non-standard Zip file.

This is what APPNOTE.TXT has to say on what should be stored in the zip
filename header field.

    The name of the file, with optional relative path.
    The path stored should not contain a drive or
    device letter, or a leading slash.  All slashes
    should be forward slashes '/' as opposed to
    backwards slashes '\' for compatibility with Amiga
    and UNIX file systems etc.

This option defaults to B<false>.

=item C<< FilterName => sub { ... }  >>

This option allow the I<archive member> name to be modified
before it is written to the zip file.

This option takes a parameter that must be a reference to a sub.  On entry
to the sub the C<$_> variable will contain the name to be filtered. If no
filename is available C<$_> will contain an empty string.

The value of C<$_> when the sub returns will be  used as the I<archive member name>.

Note that if C<CanonicalName> is enabled, a
normalized filename will be passed to the sub.

If you use C<FilterName> to modify the filename, it is your responsibility
to keep the filename in Unix format.

Although this option can be used with the OO interface, it is of most use
with the one-shot interface. For example, the code below shows how
C<FilterName> can be used to remove the path component from a series of
filenames before they are stored in C<$zipfile>.

    sub compressTxtFiles
    {
        my $zipfile = shift ;
        my $dir     = shift ;

        zip [ <$dir/*.txt> ] => $zipfile,
            FilterName => sub { s[^$dir/][] } ;
    }

=item C<< Efs => 0|1 >>

This option controls setting of the "Language Encoding Flag" (EFS) in the zip
archive. When set, the filename and comment fields for the zip archive MUST
be valid UTF-8.

If the string used for the filename and/or comment is not valid UTF-8 when this option
is true, the script will die with a "wide character" error.

Note that this option only works with Perl 5.8.4 or better.

This option defaults to B<false>.

=back

=head3 Overall Zip Archive Structure

=over 5

=item C<< Minimal => 1|0 >>

If specified, this option will disable the creation of all extra fields
in the zip local and central headers. So the C<exTime>, C<exUnix2>,
C<exUnixN>, C<ExtraFieldLocal> and C<ExtraFieldCentral> options will
be ignored.

This parameter defaults to 0.

=item C<< Stream => 0|1 >>

This option controls whether the zip file/buffer output is created in
streaming mode.

Note that when outputting to a file with streaming mode disabled (C<Stream>
is 0), the output file must be seekable.

The default is 1.

=item C<< Zip64 => 0|1 >>

Create a Zip64 zip file/buffer. This option is used if you want
to store files larger than 4 Gig or store more than 64K files in a single
zip archive.

C<Zip64> will be automatically set, as needed, if working with the one-shot
interface when the input is either a filename or a scalar reference.

If you intend to manipulate the Zip64 zip files created with this module
using an external zip/unzip, make sure that it supports Zip64.

In particular, if you are using Info-Zip you need to have zip version 3.x
or better to update a Zip64 archive and unzip version 6.x to read a zip64
archive.

The default is 0.

=back

=head3 Deflate Compression Options

=over 5

=item -Level

Defines the compression level used by zlib. The value should either be
a number between 0 and 9 (0 means no compression and 9 is maximum
compression), or one of the symbolic constants defined below.

   Z_NO_COMPRESSION
   Z_BEST_SPEED
   Z_BEST_COMPRESSION
   Z_DEFAULT_COMPRESSION

The default is Z_DEFAULT_COMPRESSION.

Note, these constants are not imported by C<IO::Compress::Zip> by default.

    use IO::Compress::Zip qw(:strategy);
    use IO::Compress::Zip qw(:constants);
    use IO::Compress::Zip qw(:all);

=item -Strategy

Defines the strategy used to tune the compression. Use one of the symbolic
constants defined below.

   Z_FILTERED
   Z_HUFFMAN_ONLY
   Z_RLE
   Z_FIXED
   Z_DEFAULT_STRATEGY

The default is Z_DEFAULT_STRATEGY.

=back

=head3 Bzip2 Compression Options

=over 5

=item C<< BlockSize100K => number >>

Specify the number of 100K blocks bzip2 uses during compression.

Valid values are from 1 to 9, where 9 is best compression.

This option is only valid if the C<Method> is ZIP_CM_BZIP2. It is ignored
otherwise.

The default is 1.

=item C<< WorkFactor => number >>

Specifies how much effort bzip2 should take before resorting to a slower
fallback compression algorithm.

Valid values range from 0 to 250, where 0 means use the default value 30.

This option is only valid if the C<Method> is ZIP_CM_BZIP2. It is ignored
otherwise.

The default is 0.

=back

=head3 Lzma and Xz Compression Options

=over 5

=item C<< Preset => number >>

Used to choose the LZMA compression preset.

Valid values are 0-9 and C<LZMA_PRESET_DEFAULT>.

0 is the fastest compression with the lowest memory usage and the lowest
compression.

9 is the slowest compression with the highest memory usage but with the best
compression.

This option is only valid if the C<Method> is ZIP_CM_LZMA. It is ignored
otherwise.

Defaults to C<LZMA_PRESET_DEFAULT> (6).

=item C<< Extreme => 0|1 >>

Makes LZMA compression a lot slower, but a small compression gain.

This option is only valid if the C<Method> is ZIP_CM_LZMA. It is ignored
otherwise.

Defaults to 0.

=back

=head3 Other Options

=over 5

=item C<< Time => $number >>

Sets the last modified time field in the zip header to $number.

This field defaults to the time the C<IO::Compress::Zip> object was created
if this option is not specified and the C<$input> parameter is not a
filename.

=item C<< ExtAttr => $attr >>

This option controls the "external file attributes" field in the central
header of the zip file. This is a 4 byte field.

If you are running a Unix derivative this value defaults to

    0100644 << 16

This should allow read/write access to any files that are extracted from
the zip file/buffer`.

For all other systems it defaults to 0.

=item C<< exTime => [$atime, $mtime, $ctime] >>

This option expects an array reference with exactly three elements:
C<$atime>, C<mtime> and C<$ctime>. These correspond to the last access
time, last modification time and creation time respectively.

It uses these values to set the extended timestamp field (ID is "UT") in
the local zip header using the three values, $atime, $mtime, $ctime. In
addition it sets the extended timestamp field in the central zip header
using C<$mtime>.

If any of the three values is C<undef> that time value will not be used.
So, for example, to set only the C<$mtime> you would use this

    exTime => [undef, $mtime, undef]

If the C<Minimal> option is set to true, this option will be ignored.

By default no extended time field is created.

=item C<< exUnix2 => [$uid, $gid] >>

This option expects an array reference with exactly two elements: C<$uid>
and C<$gid>. These values correspond to the numeric User ID (UID) and Group ID
(GID) of the owner of the files respectively.

When the C<exUnix2> option is present it will trigger the creation of a
Unix2 extra field (ID is "Ux") in the local zip header. This will be populated
with C<$uid> and C<$gid>. An empty Unix2 extra field will also
be created in the central zip header.

Note - The UID & GID are stored as 16-bit
integers in the "Ux" field. Use C<< exUnixN >> if your UID or GID are
32-bit.

If the C<Minimal> option is set to true, this option will be ignored.

By default no Unix2 extra field is created.

=item C<< exUnixN => [$uid, $gid] >>

This option expects an array reference with exactly two elements: C<$uid>
and C<$gid>. These values correspond to the numeric User ID (UID) and Group ID
(GID) of the owner of the files respectively.

When the C<exUnixN> option is present it will trigger the creation of a
UnixN extra field (ID is "ux") in both the local and central zip headers.
This will be populated with C<$uid> and C<$gid>.
The UID & GID are stored as 32-bit integers.

If the C<Minimal> option is set to true, this option will be ignored.

By default no UnixN extra field is created.

=item C<< Comment => $comment >>

Stores the contents of C<$comment> in the Central File Header of
the zip file.

Set the C<Efs> option to true if you want to store a UTF8 comment.

By default, no comment field is written to the zip file.

=item C<< ZipComment => $comment >>

Stores the contents of C<$comment> in the End of Central Directory record
of the zip file.

By default, no comment field is written to the zip file.

=item C<< Method => $method >>

Controls which compression method is used. At present the compression
methods supported are: Store (no compression at all), Deflate,
Bzip2, Zstd, Xz and Lzma.

The symbols ZIP_CM_STORE, ZIP_CM_DEFLATE, ZIP_CM_BZIP2, ZIP_CM_ZSTD, ZIP_CM_XZ and ZIP_CM_LZMA
are used to select the compression method.

These constants are not imported by C<IO::Compress::Zip> by default.

    use IO::Compress::Zip qw(:zip_method);
    use IO::Compress::Zip qw(:constants);
    use IO::Compress::Zip qw(:all);

Note that to create Bzip2 content, the module C<IO::Compress::Bzip2> must
be installed. A fatal error will be thrown if you attempt to create Bzip2
content when C<IO::Compress::Bzip2> is not available.

Note that to create Lzma content, the module C<IO::Compress::Lzma> must
be installed. A fatal error will be thrown if you attempt to create Lzma
content when C<IO::Compress::Lzma> is not available.

Note that to create Xz content, the module C<IO::Compress::Xz> must
be installed. A fatal error will be thrown if you attempt to create Xz
content when C<IO::Compress::Xz> is not available.

Note that to create Zstd content, the module C<IO::Compress::Zstd> must
be installed. A fatal error will be thrown if you attempt to create Zstd
content when C<IO::Compress::Zstd> is not available.

The default method is ZIP_CM_DEFLATE.

=item C<< TextFlag => 0|1 >>

This parameter controls the setting of a bit in the zip central header. It
is used to signal that the data stored in the zip file/buffer is probably
text.

In one-shot mode this flag will be set to true if the Perl C<-T> operator thinks
the file contains text.

The default is 0.

=item C<< ExtraFieldLocal => $data >>

=item C<< ExtraFieldCentral => $data >>

The C<ExtraFieldLocal> option is used to store additional metadata in the
local header for the zip file/buffer. The C<ExtraFieldCentral> does the
same for the matching central header.

An extra field consists of zero or more subfields. Each subfield consists
of a two byte header followed by the subfield data.

The list of subfields can be supplied in any of the following formats

    ExtraFieldLocal => [$id1, $data1,
                        $id2, $data2,
                         ...
                       ]

    ExtraFieldLocal => [ [$id1 => $data1],
                         [$id2 => $data2],
                         ...
                       ]

    ExtraFieldLocal => { $id1 => $data1,
                         $id2 => $data2,
                         ...
                       }

Where C<$id1>, C<$id2> are two byte subfield ID's.

If you use the hash syntax, you have no control over the order in which
the ExtraSubFields are stored, plus you cannot have SubFields with
duplicate ID.

Alternatively the list of subfields can by supplied as a scalar, thus

    ExtraField => $rawdata

In this case C<IO::Compress::Zip> will check that C<$rawdata> consists of
zero or more conformant sub-fields.

The Extended Time field (ID "UT"), set using the C<exTime> option, and the
Unix2 extra field (ID "Ux), set using the C<exUnix2> option, are examples
of extra fields.

If the C<Minimal> option is set to true, this option will be ignored.

The maximum size of an extra field 65535 bytes.

=item C<< Strict => 0|1 >>

This is a placeholder option.

=back

=head2 Examples

TODO

=head1 Methods

=head2 print

Usage is

    $z->print($data)
    print $z $data

Compresses and outputs the contents of the C<$data> parameter. This
has the same behaviour as the C<print> built-in.

Returns true if successful.

=head2 printf

Usage is

    $z->printf($format, $data)
    printf $z $format, $data

Compresses and outputs the contents of the C<$data> parameter.

Returns true if successful.

=head2 syswrite

Usage is

    $z->syswrite $data
    $z->syswrite $data, $length
    $z->syswrite $data, $length, $offset

Compresses and outputs the contents of the C<$data> parameter.

Returns the number of uncompressed bytes written, or C<undef> if
unsuccessful.

=head2 write

Usage is

    $z->write $data
    $z->write $data, $length
    $z->write $data, $length, $offset

Compresses and outputs the contents of the C<$data> parameter.

Returns the number of uncompressed bytes written, or C<undef> if
unsuccessful.

=head2 flush

Usage is

    $z->flush;
    $z->flush($flush_type);

Flushes any pending compressed data to the output file/buffer.

This method takes an optional parameter, C<$flush_type>, that controls
how the flushing will be carried out. By default the C<$flush_type>
used is C<Z_FINISH>. Other valid values for C<$flush_type> are
C<Z_NO_FLUSH>, C<Z_SYNC_FLUSH>, C<Z_FULL_FLUSH> and C<Z_BLOCK>. It is
strongly recommended that you only set the C<flush_type> parameter if
you fully understand the implications of what it does - overuse of C<flush>
can seriously degrade the level of compression achieved. See the C<zlib>
documentation for details.

Returns true on success.

=head2 tell

Usage is

    $z->tell()
    tell $z

Returns the uncompressed file offset.

=head2 eof

Usage is

    $z->eof();
    eof($z);

Returns true if the C<close> method has been called.

=head2 seek

    $z->seek($position, $whence);
    seek($z, $position, $whence);

Provides a sub-set of the C<seek> functionality, with the restriction
that it is only legal to seek forward in the output file/buffer.
It is a fatal error to attempt to seek backward.

Empty parts of the file/buffer will have NULL (0x00) bytes written to them.

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

This method always returns C<undef> when compressing.

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

Flushes any pending compressed data and then closes the output file/buffer.

For most versions of Perl this method will be automatically invoked if
the IO::Compress::Zip object is destroyed (either explicitly or by the
variable with the reference to the object going out of scope). The
exceptions are Perl versions 5.005 through 5.00504 and 5.8.0. In
these cases, the C<close> method will be called automatically, but
not until global destruction of all live objects when the program is
terminating.

Therefore, if you want your scripts to be able to run on all versions
of Perl, you should call C<close> explicitly and not rely on automatic
closing.

Returns true on success, otherwise 0.

If the C<AutoClose> option has been enabled when the IO::Compress::Zip
object was created, and the object is associated with a file, the
underlying file will also be closed.

=head2 newStream([OPTS])

Usage is

    $z->newStream( [OPTS] )

Closes the current compressed data stream and starts a new one.

OPTS consists of any of the options that are available when creating
the C<$z> object.

See the L</"Constructor Options"> section for more details.

=head2 deflateParams

Usage is

    $z->deflateParams

TODO

=head1 Importing

A number of symbolic constants are required by some methods in
C<IO::Compress::Zip>. None are imported by default.

=over 5

=item :all

Imports C<zip>, C<$ZipError> and all symbolic
constants that can be used by C<IO::Compress::Zip>. Same as doing this

    use IO::Compress::Zip qw(zip $ZipError :constants) ;

=item :constants

Import all symbolic constants. Same as doing this

    use IO::Compress::Zip qw(:flush :level :strategy :zip_method) ;

=item :flush

These symbolic constants are used by the C<flush> method.

    Z_NO_FLUSH
    Z_PARTIAL_FLUSH
    Z_SYNC_FLUSH
    Z_FULL_FLUSH
    Z_FINISH
    Z_BLOCK

=item :level

These symbolic constants are used by the C<Level> option in the constructor.

    Z_NO_COMPRESSION
    Z_BEST_SPEED
    Z_BEST_COMPRESSION
    Z_DEFAULT_COMPRESSION

=item :strategy

These symbolic constants are used by the C<Strategy> option in the constructor.

    Z_FILTERED
    Z_HUFFMAN_ONLY
    Z_RLE
    Z_FIXED
    Z_DEFAULT_STRATEGY

=item :zip_method

These symbolic constants are used by the C<Method> option in the
constructor.

    ZIP_CM_STORE
    ZIP_CM_DEFLATE
    ZIP_CM_BZIP2

=back

=head1 EXAMPLES

=head2 Apache::GZip Revisited

See L<IO::Compress::FAQ|IO::Compress::FAQ/"Apache::GZip Revisited">

=head2 Working with Net::FTP

See L<IO::Compress::FAQ|IO::Compress::FAQ/"Compressed files and Net::FTP">

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
