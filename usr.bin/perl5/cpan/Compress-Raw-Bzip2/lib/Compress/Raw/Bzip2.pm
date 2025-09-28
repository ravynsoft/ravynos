
package Compress::Raw::Bzip2;

use strict ;
use warnings ;

require 5.006 ;
require Exporter;
use Carp ;

use bytes ;
our ($VERSION, $XS_VERSION, @ISA, @EXPORT, $AUTOLOAD);

$VERSION = '2.204_001';
$XS_VERSION = $VERSION;
$VERSION = eval $VERSION;

@ISA = qw(Exporter);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
		BZ_RUN
		BZ_FLUSH
		BZ_FINISH

		BZ_OK
		BZ_RUN_OK
		BZ_FLUSH_OK
		BZ_FINISH_OK
		BZ_STREAM_END
		BZ_SEQUENCE_ERROR
		BZ_PARAM_ERROR
		BZ_MEM_ERROR
		BZ_DATA_ERROR
		BZ_DATA_ERROR_MAGIC
		BZ_IO_ERROR
		BZ_UNEXPECTED_EOF
		BZ_OUTBUFF_FULL
		BZ_CONFIG_ERROR

    );

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

eval {
    require XSLoader;
    XSLoader::load('Compress::Raw::Bzip2', $XS_VERSION);
    1;
}
or do {
    require DynaLoader;
    local @ISA = qw(DynaLoader);
    bootstrap Compress::Raw::Bzip2 $XS_VERSION ;
};

#sub Compress::Raw::Bzip2::new
#{
#    my $class = shift ;
#    my ($ptr, $status) = _new(@_);
#    return wantarray ? (undef, $status) : undef
#        unless $ptr ;
#    my $obj = bless [$ptr], $class ;
#    return wantarray ? ($obj, $status) : $obj;
#}
#
#package Compress::Raw::Bunzip2 ;
#
#sub Compress::Raw::Bunzip2::new
#{
#    my $class = shift ;
#    my ($ptr, $status) = _new(@_);
#    return wantarray ? (undef, $status) : undef
#        unless $ptr ;
#    my $obj = bless [$ptr], $class ;
#    return wantarray ? ($obj, $status) : $obj;
#}

sub Compress::Raw::Bzip2::STORABLE_freeze
{
    my $type = ref shift;
    croak "Cannot freeze $type object\n";
}

sub Compress::Raw::Bzip2::STORABLE_thaw
{
    my $type = ref shift;
    croak "Cannot thaw $type object\n";
}

sub Compress::Raw::Bunzip2::STORABLE_freeze
{
    my $type = ref shift;
    croak "Cannot freeze $type object\n";
}

sub Compress::Raw::Bunzip2::STORABLE_thaw
{
    my $type = ref shift;
    croak "Cannot thaw $type object\n";
}


package Compress::Raw::Bzip2;

1;

__END__


=head1 NAME

Compress::Raw::Bzip2 - Low-Level Interface to bzip2 compression library

=head1 SYNOPSIS

    use Compress::Raw::Bzip2 ;

    my ($bz, $status) = new Compress::Raw::Bzip2 [OPTS]
        or die "Cannot create bzip2 object: $bzerno\n";

    $status = $bz->bzdeflate($input, $output);
    $status = $bz->bzflush($output);
    $status = $bz->bzclose($output);

    my ($bz, $status) = new Compress::Raw::Bunzip2 [OPTS]
        or die "Cannot create bunzip2 object: $bzerno\n";

    $status = $bz->bzinflate($input, $output);

    my $version = Compress::Raw::Bzip2::bzlibversion();

=head1 DESCRIPTION

C<Compress::Raw::Bzip2> provides an interface to the in-memory
compression/uncompression functions from the bzip2 compression library.

Although the primary purpose for the existence of C<Compress::Raw::Bzip2>
is for use by the  C<IO::Compress::Bzip2> and C<IO::Compress::Bunzip2>
modules, it can be used on its own for simple compression/uncompression
tasks.

=head1 Compression

=head2 ($z, $status) = new Compress::Raw::Bzip2 $appendOutput, $blockSize100k, $workfactor;

Creates a new compression object.

If successful, it will return the initialised compression object, C<$z>
and a C<$status> of C<BZ_OK> in a list context. In scalar context it
returns the deflation object, C<$z>, only.

If not successful, the returned compression object, C<$z>, will be
I<undef> and C<$status> will hold the a I<bzip2> error code.

Below is a list of the valid options:

=over 5

=item B<$appendOutput>

Controls whether the compressed data is appended to the output buffer in
the C<bzdeflate>, C<bzflush> and C<bzclose> methods.

Defaults to 1.

=item B<$blockSize100k>

To quote the bzip2 documentation

    blockSize100k specifies the block size to be used for compression. It
    should be a value between 1 and 9 inclusive, and the actual block size
    used is 100000 x this figure. 9 gives the best compression but takes
    most memory.

Defaults to 1.

=item B<$workfactor>

To quote the bzip2 documentation

    This parameter controls how the compression phase behaves when
    presented with worst case, highly repetitive, input data. If
    compression runs into difficulties caused by repetitive data, the
    library switches from the standard sorting algorithm to a fallback
    algorithm. The fallback is slower than the standard algorithm by
    perhaps a factor of three, but always behaves reasonably, no matter how
    bad the input.

    Lower values of workFactor reduce the amount of effort the standard
    algorithm will expend before resorting to the fallback. You should set
    this parameter carefully; too low, and many inputs will be handled by
    the fallback algorithm and so compress rather slowly, too high, and
    your average-to-worst case compression times can become very large. The
    default value of 30 gives reasonable behaviour over a wide range of
    circumstances.

    Allowable values range from 0 to 250 inclusive. 0 is a special case,
    equivalent to using the default value of 30.

Defaults to 0.

=back

=head2 $status = $bz->bzdeflate($input, $output);

Reads the contents of C<$input>, compresses it and writes the compressed
data to C<$output>.

Returns C<BZ_RUN_OK> on success and a C<bzip2> error code on failure.

If C<appendOutput> is enabled in the constructor for the bzip2 object, the
compressed data will be appended to C<$output>. If not enabled, C<$output>
will be truncated before the compressed data is written to it.

=head2 $status = $bz->bzflush($output);

Flushes any pending compressed data to C<$output>.

Returns C<BZ_RUN_OK> on success and a C<bzip2> error code on failure.

=head2 $status = $bz->bzclose($output);

Terminates the compressed data stream and flushes any pending compressed
data to C<$output>.

Returns C<BZ_STREAM_END> on success and a C<bzip2> error code on failure.

=head2 Example

=head1 Uncompression

=head2 ($z, $status) = new Compress::Raw::Bunzip2 $appendOutput, $consumeInput, $small, $verbosity, $limitOutput;

If successful, it will return the initialised uncompression object, C<$z>
and a C<$status> of C<BZ_OK> in a list context. In scalar context it
returns the deflation object, C<$z>, only.

If not successful, the returned uncompression object, C<$z>, will be
I<undef> and C<$status> will hold the a I<bzip2> error code.

Below is a list of the valid options:

=over 5

=item B<$appendOutput>

Controls whether the compressed data is appended to the output buffer in the
C<bzinflate>, C<bzflush> and C<bzclose> methods.

Defaults to 1.

=item B<$consumeInput>

=item B<$small>

To quote the bzip2 documentation

    If small is nonzero, the library will use an alternative decompression
    algorithm which uses less memory but at the cost of decompressing more
    slowly (roughly speaking, half the speed, but the maximum memory
    requirement drops to around 2300k).

Defaults to 0.

=item B<$limitOutput>

The C<LimitOutput> option changes the behavior of the C<< $i->bzinflate >>
method so that the amount of memory used by the output buffer can be
limited.

When C<LimitOutput> is used the size of the output buffer used will either
be the 16k or the amount of memory already allocated to C<$output>,
whichever is larger. Predicting the output size available is tricky, so
don't rely on getting an exact output buffer size.

When C<LimitOutout> is not specified C<< $i->bzinflate >> will use as much
memory as it takes to write all the uncompressed data it creates by
uncompressing the input buffer.

If C<LimitOutput> is enabled, the C<ConsumeInput> option will also be
enabled.

This option defaults to false.

=item B<$verbosity>

This parameter is ignored.

Defaults to 0.

=back

=head2 $status = $z->bzinflate($input, $output);

Uncompresses C<$input> and writes the uncompressed data to C<$output>.

Returns C<BZ_OK> if the uncompression was successful, but the end of the
compressed data stream has not been reached. Returns C<BZ_STREAM_END> on
successful uncompression and the end of the compression stream has been
reached.

If C<consumeInput> is enabled in the constructor for the bunzip2 object,
C<$input> will have all compressed data removed from it after
uncompression. On C<BZ_OK> return this will mean that C<$input> will be an
empty string; when C<BZ_STREAM_END> C<$input> will either be an empty
string or will contain whatever data immediately followed the compressed
data stream.

If C<appendOutput> is enabled in the constructor for the bunzip2 object,
the uncompressed data will be appended to C<$output>. If not enabled,
C<$output> will be truncated before the uncompressed data is written to it.

=head1 Misc

=head2 my $version = Compress::Raw::Bzip2::bzlibversion();

Returns the version of the underlying bzip2 library.

=head1 Constants

The following bzip2 constants are exported by this module

		BZ_RUN
		BZ_FLUSH
		BZ_FINISH

		BZ_OK
		BZ_RUN_OK
		BZ_FLUSH_OK
		BZ_FINISH_OK
		BZ_STREAM_END
		BZ_SEQUENCE_ERROR
		BZ_PARAM_ERROR
		BZ_MEM_ERROR
		BZ_DATA_ERROR
		BZ_DATA_ERROR_MAGIC
		BZ_IO_ERROR
		BZ_UNEXPECTED_EOF
		BZ_OUTBUFF_FULL
		BZ_CONFIG_ERROR

=head1 SUPPORT

General feedback/questions/bug reports should be sent to
L<https://github.com/pmqs/Compress-Raw-Bzip2/issues> (preferred) or
L<https://rt.cpan.org/Public/Dist/Display.html?Name=Compress-Raw-Bzip2>.

=head1 SEE ALSO

L<Compress::Zlib>, L<IO::Compress::Gzip>, L<IO::Uncompress::Gunzip>, L<IO::Compress::Deflate>, L<IO::Uncompress::Inflate>, L<IO::Compress::RawDeflate>, L<IO::Uncompress::RawInflate>, L<IO::Compress::Bzip2>, L<IO::Uncompress::Bunzip2>, L<IO::Compress::Lzma>, L<IO::Uncompress::UnLzma>, L<IO::Compress::Xz>, L<IO::Uncompress::UnXz>, L<IO::Compress::Lzip>, L<IO::Uncompress::UnLzip>, L<IO::Compress::Lzop>, L<IO::Uncompress::UnLzop>, L<IO::Compress::Lzf>, L<IO::Uncompress::UnLzf>, L<IO::Compress::Zstd>, L<IO::Uncompress::UnZstd>, L<IO::Uncompress::AnyInflate>, L<IO::Uncompress::AnyUncompress>

L<IO::Compress::FAQ|IO::Compress::FAQ>

L<File::GlobMapper|File::GlobMapper>, L<Archive::Zip|Archive::Zip>,
L<Archive::Tar|Archive::Tar>,
L<IO::Zlib|IO::Zlib>

The primary site for the bzip2 program is L<https://sourceware.org/bzip2/>.

See the module L<Compress::Bzip2|Compress::Bzip2>

=head1 AUTHOR

This module was written by Paul Marquess, C<pmqs@cpan.org>.

=head1 MODIFICATION HISTORY

See the Changes file.

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2005-2023 Paul Marquess. All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.
