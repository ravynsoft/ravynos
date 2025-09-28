
package Compress::Raw::Zlib;

require 5.006 ;
require Exporter;
use Carp ;

use strict ;
use warnings ;
use bytes ;
our ($VERSION, $XS_VERSION, @ISA, @EXPORT, %EXPORT_TAGS, @EXPORT_OK, $AUTOLOAD, %DEFLATE_CONSTANTS, @DEFLATE_CONSTANTS);

$VERSION = '2.204_001';
$XS_VERSION = $VERSION;
$VERSION = eval $VERSION;

@ISA = qw(Exporter);
%EXPORT_TAGS = ( flush     => [qw{
                                    Z_NO_FLUSH
                                    Z_PARTIAL_FLUSH
                                    Z_SYNC_FLUSH
                                    Z_FULL_FLUSH
                                    Z_FINISH
                                    Z_BLOCK
                              }],
                 level     => [qw{
                                    Z_NO_COMPRESSION
                                    Z_BEST_SPEED
                                    Z_BEST_COMPRESSION
                                    Z_DEFAULT_COMPRESSION
                              }],
                 strategy  => [qw{
                                    Z_FILTERED
                                    Z_HUFFMAN_ONLY
                                    Z_RLE
                                    Z_FIXED
                                    Z_DEFAULT_STRATEGY
                              }],
                 status   => [qw{
                                    Z_OK
                                    Z_STREAM_END
                                    Z_NEED_DICT
                                    Z_ERRNO
                                    Z_STREAM_ERROR
                                    Z_DATA_ERROR
                                    Z_MEM_ERROR
                                    Z_BUF_ERROR
                                    Z_VERSION_ERROR
                              }],
              );

%DEFLATE_CONSTANTS = %EXPORT_TAGS;

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@DEFLATE_CONSTANTS =
@EXPORT = qw(
        ZLIB_VERSION
        ZLIB_VERNUM


        OS_CODE

        MAX_MEM_LEVEL
        MAX_WBITS

        Z_ASCII
        Z_BEST_COMPRESSION
        Z_BEST_SPEED
        Z_BINARY
        Z_BLOCK
        Z_BUF_ERROR
        Z_DATA_ERROR
        Z_DEFAULT_COMPRESSION
        Z_DEFAULT_STRATEGY
        Z_DEFLATED
        Z_ERRNO
        Z_FILTERED
        Z_FIXED
        Z_FINISH
        Z_FULL_FLUSH
        Z_HUFFMAN_ONLY
        Z_MEM_ERROR
        Z_NEED_DICT
        Z_NO_COMPRESSION
        Z_NO_FLUSH
        Z_NULL
        Z_OK
        Z_PARTIAL_FLUSH
        Z_RLE
        Z_STREAM_END
        Z_STREAM_ERROR
        Z_SYNC_FLUSH
        Z_TREES
        Z_UNKNOWN
        Z_VERSION_ERROR

        ZLIBNG_VERSION
        ZLIBNG_VERNUM
        ZLIBNG_VER_MAJOR
        ZLIBNG_VER_MINOR
        ZLIBNG_VER_REVISION
        ZLIBNG_VER_STATUS
        ZLIBNG_VER_MODIFIED

        WANT_GZIP
        WANT_GZIP_OR_ZLIB
);

push @EXPORT, qw(crc32 adler32 DEF_WBITS);

use constant WANT_GZIP           => 16;
use constant WANT_GZIP_OR_ZLIB   => 32;

sub AUTOLOAD {
    my($constname);
    ($constname = $AUTOLOAD) =~ s/.*:://;
    my ($error, $val) = constant($constname);
    Carp::croak $error if $error;
    no strict 'refs';
    *{$AUTOLOAD} = sub { $val };
    goto &{$AUTOLOAD};
}

use constant FLAG_APPEND             => 1 ;
use constant FLAG_CRC                => 2 ;
use constant FLAG_ADLER              => 4 ;
use constant FLAG_CONSUME_INPUT      => 8 ;
use constant FLAG_LIMIT_OUTPUT       => 16 ;

eval {
    require XSLoader;
    XSLoader::load('Compress::Raw::Zlib', $XS_VERSION);
    1;
}
or do {
    require DynaLoader;
    local @ISA = qw(DynaLoader);
    bootstrap Compress::Raw::Zlib $XS_VERSION ;
};


use constant Parse_any      => 0x01;
use constant Parse_unsigned => 0x02;
use constant Parse_signed   => 0x04;
use constant Parse_boolean  => 0x08;
#use constant Parse_string   => 0x10;
#use constant Parse_custom   => 0x12;

#use constant Parse_store_ref => 0x100 ;

use constant OFF_PARSED     => 0 ;
use constant OFF_TYPE       => 1 ;
use constant OFF_DEFAULT    => 2 ;
use constant OFF_FIXED      => 3 ;
use constant OFF_FIRST_ONLY => 4 ;
use constant OFF_STICKY     => 5 ;



sub ParseParameters
{
    my $level = shift || 0 ;

    my $sub = (caller($level + 1))[3] ;
    #local $Carp::CarpLevel = 1 ;
    my $p = new Compress::Raw::Zlib::Parameters() ;
    $p->parse(@_)
        or croak "$sub: $p->{Error}" ;

    return $p;
}


sub Compress::Raw::Zlib::Parameters::new
{
    my $class = shift ;

    my $obj = { Error => '',
                Got   => {},
              } ;

    #return bless $obj, ref($class) || $class || __PACKAGE__ ;
    return bless $obj, 'Compress::Raw::Zlib::Parameters' ;
}

sub Compress::Raw::Zlib::Parameters::setError
{
    my $self = shift ;
    my $error = shift ;
    my $retval = @_ ? shift : undef ;

    $self->{Error} = $error ;
    return $retval;
}

#sub getError
#{
#    my $self = shift ;
#    return $self->{Error} ;
#}

sub Compress::Raw::Zlib::Parameters::parse
{
    my $self = shift ;

    my $default = shift ;

    my $got = $self->{Got} ;
    my $firstTime = keys %{ $got } == 0 ;

    my (@Bad) ;
    my @entered = () ;

    # Allow the options to be passed as a hash reference or
    # as the complete hash.
    if (@_ == 0) {
        @entered = () ;
    }
    elsif (@_ == 1) {
        my $href = $_[0] ;
        return $self->setError("Expected even number of parameters, got 1")
            if ! defined $href or ! ref $href or ref $href ne "HASH" ;

        foreach my $key (keys %$href) {
            push @entered, $key ;
            push @entered, \$href->{$key} ;
        }
    }
    else {
        my $count = @_;
        return $self->setError("Expected even number of parameters, got $count")
            if $count % 2 != 0 ;

        for my $i (0.. $count / 2 - 1) {
            push @entered, $_[2* $i] ;
            push @entered, \$_[2* $i+1] ;
        }
    }


    while (my ($key, $v) = each %$default)
    {
        croak "need 4 params [@$v]"
            if @$v != 4 ;

        my ($first_only, $sticky, $type, $value) = @$v ;
        my $x ;
        $self->_checkType($key, \$value, $type, 0, \$x)
            or return undef ;

        $key = lc $key;

        if ($firstTime || ! $sticky) {
            $got->{$key} = [0, $type, $value, $x, $first_only, $sticky] ;
        }

        $got->{$key}[OFF_PARSED] = 0 ;
    }

    for my $i (0.. @entered / 2 - 1) {
        my $key = $entered[2* $i] ;
        my $value = $entered[2* $i+1] ;

        #print "Key [$key] Value [$value]" ;
        #print defined $$value ? "[$$value]\n" : "[undef]\n";

        $key =~ s/^-// ;
        my $canonkey = lc $key;

        if ($got->{$canonkey} && ($firstTime ||
                                  ! $got->{$canonkey}[OFF_FIRST_ONLY]  ))
        {
            my $type = $got->{$canonkey}[OFF_TYPE] ;
            my $s ;
            $self->_checkType($key, $value, $type, 1, \$s)
                or return undef ;
            #$value = $$value unless $type & Parse_store_ref ;
            $value = $$value ;
            $got->{$canonkey} = [1, $type, $value, $s] ;
        }
        else
          { push (@Bad, $key) }
    }

    if (@Bad) {
        my ($bad) = join(", ", @Bad) ;
        return $self->setError("unknown key value(s) @Bad") ;
    }

    return 1;
}

sub Compress::Raw::Zlib::Parameters::_checkType
{
    my $self = shift ;

    my $key   = shift ;
    my $value = shift ;
    my $type  = shift ;
    my $validate  = shift ;
    my $output  = shift;

    #local $Carp::CarpLevel = $level ;
    #print "PARSE $type $key $value $validate $sub\n" ;
#    if ( $type & Parse_store_ref)
#    {
#        #$value = $$value
#        #    if ref ${ $value } ;
#
#        $$output = $value ;
#        return 1;
#    }

    $value = $$value ;

    if ($type & Parse_any)
    {
        $$output = $value ;
        return 1;
    }
    elsif ($type & Parse_unsigned)
    {
        return $self->setError("Parameter '$key' must be an unsigned int, got 'undef'")
            if $validate && ! defined $value ;
        return $self->setError("Parameter '$key' must be an unsigned int, got '$value'")
            if $validate && $value !~ /^\d+$/;

        $$output = defined $value ? $value : 0 ;
        return 1;
    }
    elsif ($type & Parse_signed)
    {
        return $self->setError("Parameter '$key' must be a signed int, got 'undef'")
            if $validate && ! defined $value ;
        return $self->setError("Parameter '$key' must be a signed int, got '$value'")
            if $validate && $value !~ /^-?\d+$/;

        $$output = defined $value ? $value : 0 ;
        return 1 ;
    }
    elsif ($type & Parse_boolean)
    {
        return $self->setError("Parameter '$key' must be an int, got '$value'")
            if $validate && defined $value && $value !~ /^\d*$/;
        $$output =  defined $value ? $value != 0 : 0 ;
        return 1;
    }
#    elsif ($type & Parse_string)
#    {
#        $$output = defined $value ? $value : "" ;
#        return 1;
#    }

    $$output = $value ;
    return 1;
}



sub Compress::Raw::Zlib::Parameters::parsed
{
    my $self = shift ;
    my $name = shift ;

    return $self->{Got}{lc $name}[OFF_PARSED] ;
}

sub Compress::Raw::Zlib::Parameters::value
{
    my $self = shift ;
    my $name = shift ;

    if (@_)
    {
        $self->{Got}{lc $name}[OFF_PARSED]  = 1;
        $self->{Got}{lc $name}[OFF_DEFAULT] = $_[0] ;
        $self->{Got}{lc $name}[OFF_FIXED]   = $_[0] ;
    }

    return $self->{Got}{lc $name}[OFF_FIXED] ;
}

our $OPTIONS_deflate =
    {
        'AppendOutput'  => [1, 1, Parse_boolean,  0],
        'CRC32'         => [1, 1, Parse_boolean,  0],
        'ADLER32'       => [1, 1, Parse_boolean,  0],
        'Bufsize'       => [1, 1, Parse_unsigned, 4096],

        'Level'         => [1, 1, Parse_signed,   Z_DEFAULT_COMPRESSION()],
        'Method'        => [1, 1, Parse_unsigned, Z_DEFLATED()],
        'WindowBits'    => [1, 1, Parse_signed,   MAX_WBITS()],
        'MemLevel'      => [1, 1, Parse_unsigned, MAX_MEM_LEVEL()],
        'Strategy'      => [1, 1, Parse_unsigned, Z_DEFAULT_STRATEGY()],
        'Dictionary'    => [1, 1, Parse_any,      ""],
    };

sub Compress::Raw::Zlib::Deflate::new
{
    my $pkg = shift ;
    my ($got) = ParseParameters(0, $OPTIONS_deflate, @_);

    croak "Compress::Raw::Zlib::Deflate::new: Bufsize must be >= 1, you specified " .
            $got->value('Bufsize')
        unless $got->value('Bufsize') >= 1;

    my $flags = 0 ;
    $flags |= FLAG_APPEND if $got->value('AppendOutput') ;
    $flags |= FLAG_CRC    if $got->value('CRC32') ;
    $flags |= FLAG_ADLER  if $got->value('ADLER32') ;

    my $windowBits =  $got->value('WindowBits');
    $windowBits += MAX_WBITS()
        if ($windowBits & MAX_WBITS()) == 0 ;

    _deflateInit($flags,
                $got->value('Level'),
                $got->value('Method'),
                $windowBits,
                $got->value('MemLevel'),
                $got->value('Strategy'),
                $got->value('Bufsize'),
                $got->value('Dictionary')) ;

}

sub Compress::Raw::Zlib::deflateStream::STORABLE_freeze
{
    my $type = ref shift;
    croak "Cannot freeze $type object\n";
}

sub Compress::Raw::Zlib::deflateStream::STORABLE_thaw
{
    my $type = ref shift;
    croak "Cannot thaw $type object\n";
}


our $OPTIONS_inflate =
    {
        'AppendOutput'  => [1, 1, Parse_boolean,  0],
        'LimitOutput'   => [1, 1, Parse_boolean,  0],
        'CRC32'         => [1, 1, Parse_boolean,  0],
        'ADLER32'       => [1, 1, Parse_boolean,  0],
        'ConsumeInput'  => [1, 1, Parse_boolean,  1],
        'Bufsize'       => [1, 1, Parse_unsigned, 4096],

        'WindowBits'    => [1, 1, Parse_signed,   MAX_WBITS()],
        'Dictionary'    => [1, 1, Parse_any,      ""],
    } ;

sub Compress::Raw::Zlib::Inflate::new
{
    my $pkg = shift ;
    my ($got) = ParseParameters(0, $OPTIONS_inflate, @_);

    croak "Compress::Raw::Zlib::Inflate::new: Bufsize must be >= 1, you specified " .
            $got->value('Bufsize')
        unless $got->value('Bufsize') >= 1;

    my $flags = 0 ;
    $flags |= FLAG_APPEND if $got->value('AppendOutput') ;
    $flags |= FLAG_CRC    if $got->value('CRC32') ;
    $flags |= FLAG_ADLER  if $got->value('ADLER32') ;
    $flags |= FLAG_CONSUME_INPUT if $got->value('ConsumeInput') ;
    $flags |= FLAG_LIMIT_OUTPUT if $got->value('LimitOutput') ;


    my $windowBits =  $got->value('WindowBits');
    $windowBits += MAX_WBITS()
        if ($windowBits & MAX_WBITS()) == 0 ;

    _inflateInit($flags, $windowBits, $got->value('Bufsize'),
                 $got->value('Dictionary')) ;
}

sub Compress::Raw::Zlib::inflateStream::STORABLE_freeze
{
    my $type = ref shift;
    croak "Cannot freeze $type object\n";
}

sub Compress::Raw::Zlib::inflateStream::STORABLE_thaw
{
    my $type = ref shift;
    croak "Cannot thaw $type object\n";
}

sub Compress::Raw::Zlib::InflateScan::new
{
    my $pkg = shift ;
    my ($got) = ParseParameters(0,
                    {
                        'CRC32'         => [1, 1, Parse_boolean,  0],
                        'ADLER32'       => [1, 1, Parse_boolean,  0],
                        'Bufsize'       => [1, 1, Parse_unsigned, 4096],

                        'WindowBits'    => [1, 1, Parse_signed,   -MAX_WBITS()],
                        'Dictionary'    => [1, 1, Parse_any,      ""],
            }, @_) ;


    croak "Compress::Raw::Zlib::InflateScan::new: Bufsize must be >= 1, you specified " .
            $got->value('Bufsize')
        unless $got->value('Bufsize') >= 1;

    my $flags = 0 ;
    #$flags |= FLAG_APPEND if $got->value('AppendOutput') ;
    $flags |= FLAG_CRC    if $got->value('CRC32') ;
    $flags |= FLAG_ADLER  if $got->value('ADLER32') ;
    #$flags |= FLAG_CONSUME_INPUT if $got->value('ConsumeInput') ;

    _inflateScanInit($flags, $got->value('WindowBits'), $got->value('Bufsize'),
                 '') ;
}

sub Compress::Raw::Zlib::inflateScanStream::createDeflateStream
{
    my $pkg = shift ;
    my ($got) = ParseParameters(0,
            {
                'AppendOutput'  => [1, 1, Parse_boolean,  0],
                'CRC32'         => [1, 1, Parse_boolean,  0],
                'ADLER32'       => [1, 1, Parse_boolean,  0],
                'Bufsize'       => [1, 1, Parse_unsigned, 4096],

                'Level'         => [1, 1, Parse_signed,   Z_DEFAULT_COMPRESSION()],
                'Method'        => [1, 1, Parse_unsigned, Z_DEFLATED()],
                'WindowBits'    => [1, 1, Parse_signed,   - MAX_WBITS()],
                'MemLevel'      => [1, 1, Parse_unsigned, MAX_MEM_LEVEL()],
                'Strategy'      => [1, 1, Parse_unsigned, Z_DEFAULT_STRATEGY()],
            }, @_) ;

    croak "Compress::Raw::Zlib::InflateScan::createDeflateStream: Bufsize must be >= 1, you specified " .
            $got->value('Bufsize')
        unless $got->value('Bufsize') >= 1;

    my $flags = 0 ;
    $flags |= FLAG_APPEND if $got->value('AppendOutput') ;
    $flags |= FLAG_CRC    if $got->value('CRC32') ;
    $flags |= FLAG_ADLER  if $got->value('ADLER32') ;

    $pkg->_createDeflateStream($flags,
                $got->value('Level'),
                $got->value('Method'),
                $got->value('WindowBits'),
                $got->value('MemLevel'),
                $got->value('Strategy'),
                $got->value('Bufsize'),
                ) ;

}

sub Compress::Raw::Zlib::inflateScanStream::inflate
{
    my $self = shift ;
    my $buffer = $_[1];
    my $eof = $_[2];

    my $status = $self->scan(@_);

    if ($status == Z_OK() && $_[2]) {
        my $byte = ' ';

        $status = $self->scan(\$byte, $_[1]) ;
    }

    return $status ;
}

sub Compress::Raw::Zlib::deflateStream::deflateParams
{
    my $self = shift ;
    my ($got) = ParseParameters(0, {
                'Level'      => [1, 1, Parse_signed,   undef],
                'Strategy'   => [1, 1, Parse_unsigned, undef],
                'Bufsize'    => [1, 1, Parse_unsigned, undef],
                },
                @_) ;

    croak "Compress::Raw::Zlib::deflateParams needs Level and/or Strategy"
        unless $got->parsed('Level') + $got->parsed('Strategy') +
            $got->parsed('Bufsize');

    croak "Compress::Raw::Zlib::Inflate::deflateParams: Bufsize must be >= 1, you specified " .
            $got->value('Bufsize')
        if $got->parsed('Bufsize') && $got->value('Bufsize') <= 1;

    my $flags = 0;
    $flags |= 1 if $got->parsed('Level') ;
    $flags |= 2 if $got->parsed('Strategy') ;
    $flags |= 4 if $got->parsed('Bufsize') ;

    $self->_deflateParams($flags, $got->value('Level'),
                          $got->value('Strategy'), $got->value('Bufsize'));

}


1;
__END__


=head1 NAME

Compress::Raw::Zlib - Low-Level Interface to zlib or zlib-ng compression library

=head1 SYNOPSIS

    use Compress::Raw::Zlib ;

    ($d, $status) = new Compress::Raw::Zlib::Deflate( [OPT] ) ;
    $status = $d->deflate($input, $output) ;
    $status = $d->flush($output [, $flush_type]) ;
    $d->deflateReset() ;
    $d->deflateParams(OPTS) ;
    $d->deflateTune(OPTS) ;
    $d->dict_adler() ;
    $d->crc32() ;
    $d->adler32() ;
    $d->total_in() ;
    $d->total_out() ;
    $d->msg() ;
    $d->get_Strategy();
    $d->get_Level();
    $d->get_BufSize();

    ($i, $status) = new Compress::Raw::Zlib::Inflate( [OPT] ) ;
    $status = $i->inflate($input, $output [, $eof]) ;
    $status = $i->inflateSync($input) ;
    $i->inflateReset() ;
    $i->dict_adler() ;
    $d->crc32() ;
    $d->adler32() ;
    $i->total_in() ;
    $i->total_out() ;
    $i->msg() ;
    $d->get_BufSize();

    $crc = adler32($buffer [,$crc]) ;
    $crc = crc32($buffer [,$crc]) ;

    $crc = crc32_combine($crc1, $crc2, $len2);
    $adler = adler32_combine($adler1, $adler2, $len2);

    my $version = Compress::Raw::Zlib::zlib_version();
    my $flags = Compress::Raw::Zlib::zlibCompileFlags();

    is_zlib_native();
    is_zlibng_native();
    is_zlibng_compat();
    is_zlibng();

=head1 DESCRIPTION

The I<Compress::Raw::Zlib> module provides a Perl interface to the I<zlib> or I<zlib-ng>
compression libraries (see L</SEE ALSO> for details about where to get
I<zlib> or I<zlib-ng>).

In the text below all references to I<zlib> are also applicable to I<zlib-ng> unless otherwise stated.

=head1 Compress::Raw::Zlib::Deflate

This section defines an interface that allows in-memory compression using
the I<deflate> interface provided by zlib.

Here is a definition of the interface available:

=head2 B<($d, $status) = new Compress::Raw::Zlib::Deflate( [OPT] ) >

Initialises a deflation object.

If you are familiar with the I<zlib> library, it combines the
features of the I<zlib> functions C<deflateInit>, C<deflateInit2>
and C<deflateSetDictionary>.

If successful, it will return the initialised deflation object, C<$d>
and a C<$status> of C<Z_OK> in a list context. In scalar context it
returns the deflation object, C<$d>, only.

If not successful, the returned deflation object, C<$d>, will be
I<undef> and C<$status> will hold the a I<zlib> error code.

The function optionally takes a number of named options specified as
C<< Name => value >> pairs. This allows individual options to be
tailored without having to specify them all in the parameter list.

For backward compatibility, it is also possible to pass the parameters
as a reference to a hash containing the name=>value pairs.

Below is a list of the valid options:

=over 5

=item B<-Level>

Defines the compression level. Valid values are 0 through 9,
C<Z_NO_COMPRESSION>, C<Z_BEST_SPEED>, C<Z_BEST_COMPRESSION>, and
C<Z_DEFAULT_COMPRESSION>.

The default is C<Z_DEFAULT_COMPRESSION>.

=item B<-Method>

Defines the compression method. The only valid value at present (and
the default) is C<Z_DEFLATED>.

=item B<-WindowBits>

To compress an RFC 1950 data stream, set C<WindowBits> to a positive
number between 8 and 15.

To compress an RFC 1951 data stream, set C<WindowBits> to C<-MAX_WBITS>.

To compress an RFC 1952 data stream (i.e. gzip), set C<WindowBits> to
C<WANT_GZIP>.

For a definition of the meaning and valid values for C<WindowBits>
refer to the I<zlib> documentation for I<deflateInit2>.

Defaults to C<MAX_WBITS>.

=item B<-MemLevel>

For a definition of the meaning and valid values for C<MemLevel>
refer to the I<zlib> documentation for I<deflateInit2>.

Defaults to MAX_MEM_LEVEL.

=item B<-Strategy>

Defines the strategy used to tune the compression. The valid values are
C<Z_DEFAULT_STRATEGY>, C<Z_FILTERED>, C<Z_RLE>, C<Z_FIXED> and
C<Z_HUFFMAN_ONLY>.

The default is C<Z_DEFAULT_STRATEGY>.

=item B<-Dictionary>

When a dictionary is specified I<Compress::Raw::Zlib> will automatically
call C<deflateSetDictionary> directly after calling C<deflateInit>. The
Adler32 value for the dictionary can be obtained by calling the method
C<$d-E<gt>dict_adler()>.

The default is no dictionary.

=item B<-Bufsize>

Sets the initial size for the output buffer used by the C<$d-E<gt>deflate>
and C<$d-E<gt>flush> methods. If the buffer has to be
reallocated to increase the size, it will grow in increments of
C<Bufsize>.

The default buffer size is 4096.

=item B<-AppendOutput>

This option controls how data is written to the output buffer by the
C<$d-E<gt>deflate> and C<$d-E<gt>flush> methods.

If the C<AppendOutput> option is set to false, the output buffers in the
C<$d-E<gt>deflate> and C<$d-E<gt>flush>  methods will be truncated before
uncompressed data is written to them.

If the option is set to true, uncompressed data will be appended to the
output buffer in the C<$d-E<gt>deflate> and C<$d-E<gt>flush> methods.

This option defaults to false.

=item B<-CRC32>

If set to true, a crc32 checksum of the uncompressed data will be
calculated. Use the C<$d-E<gt>crc32> method to retrieve this value.

This option defaults to false.

=item B<-ADLER32>

If set to true, an adler32 checksum of the uncompressed data will be
calculated. Use the C<$d-E<gt>adler32> method to retrieve this value.

This option defaults to false.

=back

Here is an example of using the C<Compress::Raw::Zlib::Deflate> optional
parameter list to override the default buffer size and compression
level. All other options will take their default values.

    my $d = new Compress::Raw::Zlib::Deflate ( -Bufsize => 300,
                                               -Level   => Z_BEST_SPEED ) ;

=head2 B<$status = $d-E<gt>deflate($input, $output)>

Deflates the contents of C<$input> and writes the compressed data to
C<$output>.

The C<$input> and C<$output> parameters can be either scalars or scalar
references.

When finished, C<$input> will be completely processed (assuming there
were no errors). If the deflation was successful it writes the deflated
data to C<$output> and returns a status value of C<Z_OK>.

On error, it returns a I<zlib> error code.

If the C<AppendOutput> option is set to true in the constructor for
the C<$d> object, the compressed data will be appended to C<$output>. If
it is false, C<$output> will be truncated before any compressed data is
written to it.

B<Note>: This method will not necessarily write compressed data to
C<$output> every time it is called. So do not assume that there has been
an error if the contents of C<$output> is empty on returning from
this method. As long as the return code from the method is C<Z_OK>,
the deflate has succeeded.

=head2 B<$status = $d-E<gt>flush($output [, $flush_type]) >

Typically used to finish the deflation. Any pending output will be
written to C<$output>.

Returns C<Z_OK> if successful.

Note that flushing can seriously degrade the compression ratio, so it
should only be used to terminate a decompression (using C<Z_FINISH>) or
when you want to create a I<full flush point> (using C<Z_FULL_FLUSH>).

By default the C<flush_type> used is C<Z_FINISH>. Other valid values
for C<flush_type> are C<Z_NO_FLUSH>, C<Z_PARTIAL_FLUSH>, C<Z_SYNC_FLUSH>
and C<Z_FULL_FLUSH>. It is strongly recommended that you only set the
C<flush_type> parameter if you fully understand the implications of
what it does. See the C<zlib> documentation for details.

If the C<AppendOutput> option is set to true in the constructor for
the C<$d> object, the compressed data will be appended to C<$output>. If
it is false, C<$output> will be truncated before any compressed data is
written to it.

=head2 B<$status = $d-E<gt>deflateReset() >

This method will reset the deflation object C<$d>. It can be used when you
are compressing multiple data streams and want to use the same object to
compress each of them. It should only be used once the previous data stream
has been flushed successfully, i.e. a call to C<< $d->flush(Z_FINISH) >> has
returned C<Z_OK>.

Returns C<Z_OK> if successful.

=head2 B<$status = $d-E<gt>deflateParams([OPT])>

Change settings for the deflate object C<$d>.

The list of the valid options is shown below. Options not specified
will remain unchanged.

=over 5

=item B<-Level>

Defines the compression level. Valid values are 0 through 9,
C<Z_NO_COMPRESSION>, C<Z_BEST_SPEED>, C<Z_BEST_COMPRESSION>, and
C<Z_DEFAULT_COMPRESSION>.

=item B<-Strategy>

Defines the strategy used to tune the compression. The valid values are
C<Z_DEFAULT_STRATEGY>, C<Z_FILTERED> and C<Z_HUFFMAN_ONLY>.

=item B<-BufSize>

Sets the initial size for the output buffer used by the C<$d-E<gt>deflate>
and C<$d-E<gt>flush> methods. If the buffer has to be
reallocated to increase the size, it will grow in increments of
C<Bufsize>.

=back

=head2 B<$status = $d-E<gt>deflateTune($good_length, $max_lazy, $nice_length, $max_chain)>

Tune the internal settings for the deflate object C<$d>. This option is
only available if you are running zlib 1.2.2.3 or better.

Refer to the documentation in zlib.h for instructions on how to fly
C<deflateTune>.

=head2 B<$d-E<gt>dict_adler()>

Returns the adler32 value for the dictionary.

=head2 B<$d-E<gt>crc32()>

Returns the crc32 value for the uncompressed data to date.

If the C<CRC32> option is not enabled in the constructor for this object,
this method will always return 0;

=head2 B<$d-E<gt>adler32()>

Returns the adler32 value for the uncompressed data to date.

=head2 B<$d-E<gt>msg()>

Returns the last error message generated by zlib.

=head2 B<$d-E<gt>total_in()>

Returns the total number of bytes uncompressed bytes input to deflate.

=head2 B<$d-E<gt>total_out()>

Returns the total number of compressed bytes output from deflate.

=head2 B<$d-E<gt>get_Strategy()>

Returns the deflation strategy currently used. Valid values are
C<Z_DEFAULT_STRATEGY>, C<Z_FILTERED> and C<Z_HUFFMAN_ONLY>.

=head2 B<$d-E<gt>get_Level()>

Returns the compression level being used.

=head2 B<$d-E<gt>get_BufSize()>

Returns the buffer size used to carry out the compression.

=head2 Example

Here is a trivial example of using C<deflate>. It simply reads standard
input, deflates it and writes it to standard output.

    use strict ;
    use warnings ;

    use Compress::Raw::Zlib ;

    binmode STDIN;
    binmode STDOUT;
    my $x = new Compress::Raw::Zlib::Deflate
       or die "Cannot create a deflation stream\n" ;

    my ($output, $status) ;
    while (<>)
    {
        $status = $x->deflate($_, $output) ;

        $status == Z_OK
            or die "deflation failed\n" ;

        print $output ;
    }

    $status = $x->flush($output) ;

    $status == Z_OK
        or die "deflation failed\n" ;

    print $output ;

=head1 Compress::Raw::Zlib::Inflate

This section defines an interface that allows in-memory uncompression using
the I<inflate> interface provided by zlib.

Here is a definition of the interface:

=head2 B< ($i, $status) = new Compress::Raw::Zlib::Inflate( [OPT] ) >

Initialises an inflation object.

In a list context it returns the inflation object, C<$i>, and the
I<zlib> status code (C<$status>). In a scalar context it returns the
inflation object only.

If successful, C<$i> will hold the inflation object and C<$status> will
be C<Z_OK>.

If not successful, C<$i> will be I<undef> and C<$status> will hold the
I<zlib> error code.

The function optionally takes a number of named options specified as
C<< -Name => value >> pairs. This allows individual options to be
tailored without having to specify them all in the parameter list.

For backward compatibility, it is also possible to pass the parameters
as a reference to a hash containing the C<< name=>value >> pairs.

Here is a list of the valid options:

=over 5

=item B<-WindowBits>

To uncompress an RFC 1950 data stream, set C<WindowBits> to a positive
number between 8 and 15.

To uncompress an RFC 1951 data stream, set C<WindowBits> to C<-MAX_WBITS>.

To uncompress an RFC 1952 data stream (i.e. gzip), set C<WindowBits> to
C<WANT_GZIP>.

To auto-detect and uncompress an RFC 1950 or RFC 1952 data stream (i.e.
gzip), set C<WindowBits> to C<WANT_GZIP_OR_ZLIB>.

For a full definition of the meaning and valid values for C<WindowBits>
refer to the I<zlib> documentation for I<inflateInit2>.

Defaults to C<MAX_WBITS>.

=item B<-Bufsize>

Sets the initial size for the output buffer used by the C<$i-E<gt>inflate>
method. If the output buffer in this method has to be reallocated to
increase the size, it will grow in increments of C<Bufsize>.

Default is 4096.

=item B<-Dictionary>

The default is no dictionary.

=item B<-AppendOutput>

This option controls how data is written to the output buffer by the
C<$i-E<gt>inflate> method.

If the option is set to false, the output buffer in the C<$i-E<gt>inflate>
method will be truncated before uncompressed data is written to it.

If the option is set to true, uncompressed data will be appended to the
output buffer by the C<$i-E<gt>inflate> method.

This option defaults to false.

=item B<-CRC32>

If set to true, a crc32 checksum of the uncompressed data will be
calculated. Use the C<$i-E<gt>crc32> method to retrieve this value.

This option defaults to false.

=item B<-ADLER32>

If set to true, an adler32 checksum of the uncompressed data will be
calculated. Use the C<$i-E<gt>adler32> method to retrieve this value.

This option defaults to false.

=item B<-ConsumeInput>

If set to true, this option will remove compressed data from the input
buffer of the C<< $i->inflate >> method as the inflate progresses.

This option can be useful when you are processing compressed data that is
embedded in another file/buffer. In this case the data that immediately
follows the compressed stream will be left in the input buffer.

This option defaults to true.

=item B<-LimitOutput>

The C<LimitOutput> option changes the behavior of the C<< $i->inflate >>
method so that the amount of memory used by the output buffer can be
limited.

When C<LimitOutput> is used the size of the output buffer used will either
be the value of the C<Bufsize> option or the amount of memory already
allocated to C<$output>, whichever is larger. Predicting the output size
available is tricky, so don't rely on getting an exact output buffer size.

When C<LimitOutout> is not specified C<< $i->inflate >> will use as much
memory as it takes to write all the uncompressed data it creates by
uncompressing the input buffer.

If C<LimitOutput> is enabled, the C<ConsumeInput> option will also be
enabled.

This option defaults to false.

See L</The LimitOutput option> for a discussion on why C<LimitOutput> is
needed and how to use it.

=back

Here is an example of using an optional parameter to override the default
buffer size.

    my ($i, $status) = new Compress::Raw::Zlib::Inflate( -Bufsize => 300 ) ;

=head2 B< $status = $i-E<gt>inflate($input, $output [,$eof]) >

Inflates the complete contents of C<$input> and writes the uncompressed
data to C<$output>. The C<$input> and C<$output> parameters can either be
scalars or scalar references.

Returns C<Z_OK> if successful and C<Z_STREAM_END> if the end of the
compressed data has been successfully reached.

If not successful C<$status> will hold the I<zlib> error code.

If the C<ConsumeInput> option has been set to true when the
C<Compress::Raw::Zlib::Inflate> object is created, the C<$input> parameter
is modified by C<inflate>. On completion it will contain what remains
of the input buffer after inflation. In practice, this means that when
the return status is C<Z_OK> the C<$input> parameter will contain an
empty string, and when the return status is C<Z_STREAM_END> the C<$input>
parameter will contains what (if anything) was stored in the input buffer
after the deflated data stream.

This feature is useful when processing a file format that encapsulates
a compressed data stream (e.g. gzip, zip) and there is useful data
immediately after the deflation stream.

If the C<AppendOutput> option is set to true in the constructor for
this object, the uncompressed data will be appended to C<$output>. If
it is false, C<$output> will be truncated before any uncompressed data
is written to it.

The C<$eof> parameter needs a bit of explanation.

Prior to version 1.2.0, zlib assumed that there was at least one trailing
byte immediately after the compressed data stream when it was carrying out
decompression. This normally isn't a problem because the majority of zlib
applications guarantee that there will be data directly after the
compressed data stream.  For example, both gzip (RFC 1950) and zip both
define trailing data that follows the compressed data stream.

The C<$eof> parameter only needs to be used if B<all> of the following
conditions apply

=over 5

=item 1

You are either using a copy of zlib that is older than version 1.2.0 or you
want your application code to be able to run with as many different
versions of zlib as possible.

=item 2

You have set the C<WindowBits> parameter to C<-MAX_WBITS> in the constructor
for this object, i.e. you are uncompressing a raw deflated data stream
(RFC 1951).

=item 3

There is no data immediately after the compressed data stream.

=back

If B<all> of these are the case, then you need to set the C<$eof> parameter
to true on the final call (and only the final call) to C<$i-E<gt>inflate>.

If you have built this module with zlib >= 1.2.0, the C<$eof> parameter is
ignored. You can still set it if you want, but it won't be used behind the
scenes.

=head2 B<$status = $i-E<gt>inflateSync($input)>

This method can be used to attempt to recover good data from a compressed
data stream that is partially corrupt.
It scans C<$input> until it reaches either a I<full flush point> or the
end of the buffer.

If a I<full flush point> is found, C<Z_OK> is returned and C<$input>
will be have all data up to the flush point removed. This data can then be
passed to the C<$i-E<gt>inflate> method to be uncompressed.

Any other return code means that a flush point was not found. If more
data is available, C<inflateSync> can be called repeatedly with more
compressed data until the flush point is found.

Note I<full flush points> are not present by default in compressed
data streams. They must have been added explicitly when the data stream
was created by calling C<Compress::Deflate::flush>  with C<Z_FULL_FLUSH>.

=head2 B<$status = $i-E<gt>inflateReset() >

This method will reset the inflation object C<$i>. It can be used when you
are uncompressing multiple data streams and want to use the same object to
uncompress each of them.

Returns C<Z_OK> if successful.

=head2 B<$i-E<gt>dict_adler()>

Returns the adler32 value for the dictionary.

=head2 B<$i-E<gt>crc32()>

Returns the crc32 value for the uncompressed data to date.

If the C<CRC32> option is not enabled in the constructor for this object,
this method will always return 0;

=head2 B<$i-E<gt>adler32()>

Returns the adler32 value for the uncompressed data to date.

If the C<ADLER32> option is not enabled in the constructor for this object,
this method will always return 0;

=head2 B<$i-E<gt>msg()>

Returns the last error message generated by zlib.

=head2 B<$i-E<gt>total_in()>

Returns the total number of bytes compressed bytes input to inflate.

=head2 B<$i-E<gt>total_out()>

Returns the total number of uncompressed bytes output from inflate.

=head2 B<$d-E<gt>get_BufSize()>

Returns the buffer size used to carry out the decompression.

=head2 Examples

Here is an example of using C<inflate>.

    use strict ;
    use warnings ;

    use Compress::Raw::Zlib;

    my $x = new Compress::Raw::Zlib::Inflate()
       or die "Cannot create a inflation stream\n" ;

    my $input = '' ;
    binmode STDIN;
    binmode STDOUT;

    my ($output, $status) ;
    while (read(STDIN, $input, 4096))
    {
        $status = $x->inflate($input, $output) ;

        print $output ;

        last if $status != Z_OK ;
    }

    die "inflation failed\n"
        unless $status == Z_STREAM_END ;

The next example show how to use the C<LimitOutput> option. Notice the use
of two nested loops in this case. The outer loop reads the data from the
input source - STDIN and the inner loop repeatedly calls C<inflate> until
C<$input> is exhausted, we get an error, or the end of the stream is
reached. One point worth remembering is by using the C<LimitOutput> option
you also get C<ConsumeInput> set as well - this makes the code below much
simpler.

    use strict ;
    use warnings ;

    use Compress::Raw::Zlib;

    my $x = new Compress::Raw::Zlib::Inflate(LimitOutput => 1)
       or die "Cannot create a inflation stream\n" ;

    my $input = '' ;
    binmode STDIN;
    binmode STDOUT;

    my ($output, $status) ;

  OUTER:
    while (read(STDIN, $input, 4096))
    {
        do
        {
            $status = $x->inflate($input, $output) ;

            print $output ;

            last OUTER
                unless $status == Z_OK || $status == Z_BUF_ERROR ;
        }
        while ($status == Z_OK && length $input);
    }

    die "inflation failed\n"
        unless $status == Z_STREAM_END ;

=head1 CHECKSUM FUNCTIONS

Two functions are provided by I<zlib> to calculate checksums. For the
Perl interface, the order of the two parameters in both functions has
been reversed. This allows both running checksums and one off
calculations to be done.

    $crc = adler32($buffer [,$crc]) ;
    $crc = crc32($buffer [,$crc]) ;

The buffer parameters can either be a scalar or a scalar reference.

If the $crc parameters is C<undef>, the crc value will be reset.

If you have built this module with zlib 1.2.3 or better, two more
CRC-related functions are available.

    $crc = crc32_combine($crc1, $crc2, $len2);
    $adler = adler32_combine($adler1, $adler2, $len2);

These functions allow checksums to be merged.
Refer to the I<zlib> documentation for more details.

=head1 Misc

=head2 my $version = Compress::Raw::Zlib::zlib_version();

Returns the version of the I<zlib> library if this module has been built with the I<zlib> library.
If this module has been built with I<zlib-ng> in native mode, this function will return a empty string.
If this module has been built with I<zlib-ng> in compat mode, this function will return the Izlib> API
verion that I<zlib-ng> is supporting.

=head2 my $version = Compress::Raw::Zlib::zlibng_version();

Returns the version of the zlib-ng library if this module has been built with the I<zlib-ng> library.
If this module has been built with I<zlib>, this function will return a empty string.

=head2  my $flags = Compress::Raw::Zlib::zlibCompileFlags();

Returns the flags indicating compile-time options that were used to build
the zlib or zlib-ng library. See the zlib documentation for a description of the flags
returned by C<zlibCompileFlags>.

Note that when the zlib sources are built along with this module the
C<sprintf> flags (bits 24, 25 and 26) should be ignored.

If you are using zlib 1.2.0 or older, C<zlibCompileFlags> will return 0.

=head2 is_zlib_native();
=head2 is_zlibng_native();
=head2 is_zlibng_compat();
=head2 is_zlibng();

These function can use used to check if C<Compress::Raw::Zlib> was been built with I<zlib> or I<zlib-ng>.

The function C<is_zlib_native> returns true if C<Compress::Raw::Zlib> was built with I<zlib>.
The function C<is_zlibng> returns true if C<Compress::Raw::Zlib> was built with I<zlib-ng>.

The I<zlib-ng> library has an option to build with a zlib-compataible API.
The c<is_zlibng_compat> function retuens true if zlib-ng has ben built with this API.

Finally, C<is_zlibng_native> returns true if I<zlib-ng> was built with its native API.

=head1 The LimitOutput option.

By default C<< $i->inflate($input, $output) >> will uncompress I<all> data
in C<$input> and write I<all> of the uncompressed data it has generated to
C<$output>. This makes the interface to C<inflate> much simpler - if the
method has uncompressed C<$input> successfully I<all> compressed data in
C<$input> will have been dealt with. So if you are reading from an input
source and uncompressing as you go the code will look something like this

    use strict ;
    use warnings ;

    use Compress::Raw::Zlib;

    my $x = new Compress::Raw::Zlib::Inflate()
       or die "Cannot create a inflation stream\n" ;

    my $input = '' ;

    my ($output, $status) ;
    while (read(STDIN, $input, 4096))
    {
        $status = $x->inflate($input, $output) ;

        print $output ;

        last if $status != Z_OK ;
    }

    die "inflation failed\n"
        unless $status == Z_STREAM_END ;

The points to note are

=over 5

=item *

The main processing loop in the code handles reading of compressed data
from STDIN.

=item *

The status code returned from C<inflate> will only trigger termination of
the main processing loop if it isn't C<Z_OK>. When C<LimitOutput> has not
been used the C<Z_OK> status means that the end of the compressed
data stream has been reached or there has been an error in uncompression.

=item *

After the call to C<inflate> I<all> of the uncompressed data in C<$input>
will have been processed. This means the subsequent call to C<read> can
overwrite it's contents without any problem.

=back

For most use-cases the behavior described above is acceptable (this module
and it's predecessor, C<Compress::Zlib>, have used it for over 10 years
without an issue), but in a few very specific use-cases the amount of
memory required for C<$output> can prohibitively large. For example, if the
compressed data stream contains the same pattern repeated thousands of
times, a relatively small compressed data stream can uncompress into
hundreds of megabytes.  Remember C<inflate> will keep allocating memory
until I<all> the uncompressed data has been written to the output buffer -
the size of C<$output> is unbounded.

The C<LimitOutput> option is designed to help with this use-case.

The main difference in your code when using C<LimitOutput> is having to
deal with cases where the C<$input> parameter still contains some
uncompressed data that C<inflate> hasn't processed yet. The status code
returned from C<inflate> will be C<Z_OK> if uncompression took place and
C<Z_BUF_ERROR> if the output buffer is full.

Below is typical code that shows how to use C<LimitOutput>.

    use strict ;
    use warnings ;

    use Compress::Raw::Zlib;

    my $x = new Compress::Raw::Zlib::Inflate(LimitOutput => 1)
       or die "Cannot create a inflation stream\n" ;

    my $input = '' ;
    binmode STDIN;
    binmode STDOUT;

    my ($output, $status) ;

  OUTER:
    while (read(STDIN, $input, 4096))
    {
        do
        {
            $status = $x->inflate($input, $output) ;

            print $output ;

            last OUTER
                unless $status == Z_OK || $status == Z_BUF_ERROR ;
        }
        while ($status == Z_OK && length $input);
    }

    die "inflation failed\n"
        unless $status == Z_STREAM_END ;

Points to note this time:

=over 5

=item *

There are now two nested loops in the code: the outer loop for reading the
compressed data from STDIN, as before; and the inner loop to carry out the
uncompression.

=item *

There are two exit points from the inner uncompression loop.

Firstly when C<inflate> has returned a status other than C<Z_OK> or
C<Z_BUF_ERROR>.  This means that either the end of the compressed data
stream has been reached (C<Z_STREAM_END>) or there is an error in the
compressed data. In either of these cases there is no point in continuing
with reading the compressed data, so both loops are terminated.

The second exit point tests if there is any data left in the input buffer,
C<$input> - remember that the C<ConsumeInput> option is automatically
enabled when C<LimitOutput> is used.  When the input buffer has been
exhausted, the outer loop can run again and overwrite a now empty
C<$input>.

=back

=head1 ACCESSING ZIP FILES

Although it is possible (with some effort on your part) to use this module
to access .zip files, there are other perl modules available that will do
all the hard work for you. Check out C<Archive::Zip>,
C<Archive::Zip::SimpleZip>, C<IO::Compress::Zip> and
C<IO::Uncompress::Unzip>.

=head1 FAQ

=head2 Compatibility with Unix compress/uncompress.

This module is not compatible with Unix C<compress>.

If you have the C<uncompress> program available, you can use this to read
compressed files

    open F, "uncompress -c $filename |";
    while (<F>)
    {
        ...

Alternatively, if you have the C<gunzip> program available, you can use
this to read compressed files

    open F, "gunzip -c $filename |";
    while (<F>)
    {
        ...

and this to write compress files, if you have the C<compress> program
available

    open F, "| compress -c $filename ";
    print F "data";
    ...
    close F ;

=head2 Accessing .tar.Z files

See previous FAQ item.

If the C<Archive::Tar> module is installed and either the C<uncompress> or
C<gunzip> programs are available, you can use one of these workarounds to
read C<.tar.Z> files.

Firstly with C<uncompress>

    use strict;
    use warnings;
    use Archive::Tar;

    open F, "uncompress -c $filename |";
    my $tar = Archive::Tar->new(*F);
    ...

and this with C<gunzip>

    use strict;
    use warnings;
    use Archive::Tar;

    open F, "gunzip -c $filename |";
    my $tar = Archive::Tar->new(*F);
    ...

Similarly, if the C<compress> program is available, you can use this to
write a C<.tar.Z> file

    use strict;
    use warnings;
    use Archive::Tar;
    use IO::File;

    my $fh = new IO::File "| compress -c >$filename";
    my $tar = Archive::Tar->new();
    ...
    $tar->write($fh);
    $fh->close ;

=head2 Zlib Library Version Support

By default C<Compress::Raw::Zlib> will build with a private copy of version
1.2.5 of the zlib library. (See the F<README> file for details of
how to override this behaviour)

If you decide to use a different version of the zlib library, you need to be
aware of the following issues

=over 5

=item *

First off, you must have zlib 1.0.5 or better.

=item *

You need to have zlib 1.2.1 or better if you want to use the C<-Merge>
option with C<IO::Compress::Gzip>, C<IO::Compress::Deflate> and
C<IO::Compress::RawDeflate>.

=back

=head1 CONSTANTS

All the I<zlib> constants are automatically imported when you make use
of I<Compress::Raw::Zlib>.

=head1 SUPPORT

General feedback/questions/bug reports should be sent to
L<https://github.com/pmqs/Compress-Raw-Zlib/issues> (preferred) or
L<https://rt.cpan.org/Public/Dist/Display.html?Name=Compress-Raw-Zlib>.

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
