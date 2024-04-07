package CompTestUtils;

package main ;

use strict ;
use warnings;
use bytes;

#use lib qw(t t/compress);

use Carp ;
#use Test::More ;



sub title
{
    #diag "" ;
    ok(1, $_[0]) ;
    #diag "" ;
}

sub like_eval
{
    like $@, @_ ;
}

BEGIN {
    eval {
       require File::Temp;
     } ;

}

sub test_zlib_header_matches_library
{
SKIP: {
    skip "TEST_SKIP_VERSION_CHECK is set", 1
        if $ENV{TEST_SKIP_VERSION_CHECK};

    if (Compress::Raw::Zlib::is_zlibng_native())
    {
        my $zlibng_h = Compress::Raw::Zlib::ZLIBNG_VERSION ;
        my $libzng   = Compress::Raw::Zlib::zlibng_version();
        is($zlibng_h, $libzng, "ZLIBNG_VERSION ($zlibng_h) matches Compress::Raw::Zlib::zlibng_version")
            or diag <<EOM;

The version of zlib-ng.h does not match the version of libz-ng

You have zlib-ng.h version $zlibng_h
     and libz-ng   version $libzng

You probably have two versions of zlib-ng installed on your system.
Try removing the one you don't want to use and rebuild.
EOM
    }
    else
    {
        my $zlib_h = ZLIB_VERSION ;
        my $libz   = Compress::Raw::Zlib::zlib_version();
        is($zlib_h, $libz, "ZLIB_VERSION ($zlib_h) matches Compress::Raw::Zlib::zlib_version")
            or diag <<EOM;

The version of zlib.h does not match the version of libz

You have zlib.h version $zlib_h
     and libz   version $libz

You probably have two versions of zlib installed on your system.
Try removing the one you don't want to use and rebuild.
EOM
    }
    }
}


{
    package LexFile ;

    our ($index);
    $index = '00000';

    sub new
    {
        my $self = shift ;
        foreach (@_)
        {
            Carp::croak "NO!!!!" if defined $_;
            # autogenerate the name if none supplied
            $_ = "tst" . $$ . "X" . $index ++ . ".tmp"
                unless defined $_;
        }
        chmod 0777, @_;
        for (@_) { 1 while unlink $_ } ;
        bless [ @_ ], $self ;
    }

    sub DESTROY
    {
        my $self = shift ;
        chmod 0777, @{ $self } ;
        for (@$self) { 1 while unlink $_ } ;
    }

}

{
    package LexDir ;

    use File::Path;

    our ($index);
    $index = '00000';
    our ($useTempFile);
    our ($useTempDir);

    sub new
    {
        my $self = shift ;

        if ( $useTempDir)
        {
            foreach (@_)
            {
                Carp::croak "NO!!!!" if defined $_;
                $_ = File::Temp->newdir(DIR => '.');
                # Subsequent manipulations assume Unix syntax, metacharacters, etc.
                if ($^O eq 'VMS')
                {
                    $_->{DIRNAME} = VMS::Filespec::unixify($_->{DIRNAME});
                    $_->{DIRNAME} =~ s/\/$//;
                }
            }
            bless [ @_ ], $self ;
        }
        elsif ( $useTempFile)
        {
            foreach (@_)
            {
                Carp::croak "NO!!!!" if defined $_;
                $_ = File::Temp::tempdir(DIR => '.', CLEANUP => 1);
                # Subsequent manipulations assume Unix syntax, metacharacters, etc.
                if ($^O eq 'VMS')
                {
                    $_ = VMS::Filespec::unixify($_);
                    $_ =~ s/\/$//;
                }
            }
            bless [ @_ ], $self ;
        }
        else
        {
            foreach (@_)
            {
                Carp::croak "NO!!!!" if defined $_;
                # autogenerate the name if none supplied
                $_ = "tmpdir" . $$ . "X" . $index ++ . ".tmp" ;
            }
            foreach (@_)
            {
                rmtree $_, {verbose => 0, safe => 1}
                    if -d $_;
                mkdir $_, 0777
            }
            bless [ @_ ], $self ;
        }

    }

    sub DESTROY
    {
        if (! $useTempFile)
        {
            my $self = shift ;
            foreach (@$self)
            {
                rmtree $_, {verbose => 0, safe => 1}
                    if -d $_ ;
            }
        }
    }
}

sub readFile
{
    my $f = shift ;

    my @strings ;

    if (IO::Compress::Base::Common::isaFilehandle($f))
    {
        my $pos = tell($f);
        seek($f, 0,0);
        @strings = <$f> ;
        seek($f, 0, $pos);
    }
    else
    {
        open (F, "<$f")
            or croak "Cannot open $f: $!\n" ;
        binmode F;
        @strings = <F> ;
        close F ;
    }

    return @strings if wantarray ;
    return join "", @strings ;
}

sub touch
{
    foreach (@_) { writeFile($_, '') }
}

sub writeFile
{
    my($filename, @strings) = @_ ;
    1 while unlink $filename ;
    open (F, ">$filename")
        or croak "Cannot open $filename: $!\n" ;
    binmode F;
    foreach (@strings) {
        no warnings ;
        print F $_ ;
    }
    close F ;
}

sub GZreadFile
{
    my ($filename) = shift ;

    my ($uncomp) = "" ;
    my $line = "" ;
    my $fil = gzopen($filename, "rb")
        or croak "Cannopt open '$filename': $Compress::Zlib::gzerrno" ;

    $uncomp .= $line
        while $fil->gzread($line) > 0;

    $fil->gzclose ;
    return $uncomp ;
}

sub hexDump
{
    my $d = shift ;

    if (IO::Compress::Base::Common::isaFilehandle($d))
    {
        $d = readFile($d);
    }
    elsif (IO::Compress::Base::Common::isaFilename($d))
    {
        $d = readFile($d);
    }
    else
    {
        $d = $$d ;
    }

    my $offset = 0 ;

    $d = '' unless defined $d ;
    #while (read(STDIN, $data, 16)) {
    while (my $data = substr($d, 0, 16)) {
        substr($d, 0, 16) = '' ;
        printf "# %8.8lx    ", $offset;
        $offset += 16;

        my @array = unpack('C*', $data);
        foreach (@array) {
            printf('%2.2x ', $_);
        }
        print "   " x (16 - @array)
            if @array < 16 ;
        $data =~ tr/\0-\37\177-\377/./;
        print "  $data\n";
    }

}

sub readHeaderInfo
{
    my $name = shift ;
    my %opts = @_ ;

    my $string = <<EOM;
some text
EOM

    ok my $x = new IO::Compress::Gzip $name, %opts
        or diag "GzipError is $IO::Compress::Gzip::GzipError" ;
    ok $x->write($string) ;
    ok $x->close ;

    #is GZreadFile($name), $string ;

    ok my $gunz = new IO::Uncompress::Gunzip $name, Strict => 0
        or diag "GunzipError is $IO::Uncompress::Gunzip::GunzipError" ;
    ok my $hdr = $gunz->getHeaderInfo();
    my $uncomp ;
    ok $gunz->read($uncomp) ;
    ok $uncomp eq $string;
    ok $gunz->close ;

    return $hdr ;
}

sub cmpFile
{
    my ($filename, $uue) = @_ ;
    return readFile($filename) eq unpack("u", $uue) ;
}

#sub isRawFormat
#{
#    my $class = shift;
#    # TODO -- add Lzma here?
#    my %raw = map { $_ => 1 } qw( RawDeflate );
#
#    return defined $raw{$class};
#}



my %TOP = (
    'IO::Uncompress::AnyInflate' => { Inverse  => 'IO::Compress::Gzip',
                                      Error    => 'AnyInflateError',
                                      TopLevel => 'anyinflate',
                                      Raw      => 0,
                            },

    'IO::Uncompress::AnyUncompress' => { Inverse  => 'IO::Compress::Gzip',
                                         Error    => 'AnyUncompressError',
                                         TopLevel => 'anyuncompress',
                                         Raw      => 0,
                            },

    'IO::Compress::Gzip' => { Inverse  => 'IO::Uncompress::Gunzip',
                              Error    => 'GzipError',
                              TopLevel => 'gzip',
                              Raw      => 0,
                            },
    'IO::Uncompress::Gunzip' => { Inverse  => 'IO::Compress::Gzip',
                                  Error    => 'GunzipError',
                                  TopLevel => 'gunzip',
                                  Raw      => 0,
                            },

    'IO::Compress::Deflate' => { Inverse  => 'IO::Uncompress::Inflate',
                                 Error    => 'DeflateError',
                                 TopLevel => 'deflate',
                                 Raw      => 0,
                            },
    'IO::Uncompress::Inflate' => { Inverse  => 'IO::Compress::Deflate',
                                   Error    => 'InflateError',
                                   TopLevel => 'inflate',
                                   Raw      => 0,
                            },

    'IO::Compress::RawDeflate' => { Inverse  => 'IO::Uncompress::RawInflate',
                                    Error    => 'RawDeflateError',
                                    TopLevel => 'rawdeflate',
                                    Raw      => 1,
                            },
    'IO::Uncompress::RawInflate' => { Inverse  => 'IO::Compress::RawDeflate',
                                      Error    => 'RawInflateError',
                                      TopLevel => 'rawinflate',
                                      Raw      => 1,
                            },

    'IO::Compress::Zip' => { Inverse  => 'IO::Uncompress::Unzip',
                             Error    => 'ZipError',
                             TopLevel => 'zip',
                             Raw      => 0,
                            },
    'IO::Uncompress::Unzip' => { Inverse  => 'IO::Compress::Zip',
                                 Error    => 'UnzipError',
                                 TopLevel => 'unzip',
                                 Raw      => 0,
                            },

    'IO::Compress::Bzip2' => { Inverse  => 'IO::Uncompress::Bunzip2',
                               Error    => 'Bzip2Error',
                               TopLevel => 'bzip2',
                               Raw      => 0,
                            },
    'IO::Uncompress::Bunzip2' => { Inverse  => 'IO::Compress::Bzip2',
                                   Error    => 'Bunzip2Error',
                                   TopLevel => 'bunzip2',
                                   Raw      => 0,
                            },

    'IO::Compress::Lzop' => { Inverse  => 'IO::Uncompress::UnLzop',
                              Error    => 'LzopError',
                              TopLevel => 'lzop',
                              Raw      => 0,
                            },
    'IO::Uncompress::UnLzop' => { Inverse  => 'IO::Compress::Lzop',
                                  Error    => 'UnLzopError',
                                  TopLevel => 'unlzop',
                                  Raw      => 0,
                            },

    'IO::Compress::Lzf' => { Inverse  => 'IO::Uncompress::UnLzf',
                             Error    => 'LzfError',
                             TopLevel => 'lzf',
                             Raw      => 0,
                            },
    'IO::Uncompress::UnLzf' => { Inverse  => 'IO::Compress::Lzf',
                                 Error    => 'UnLzfError',
                                 TopLevel => 'unlzf',
                                 Raw      => 0,
                            },

    'IO::Compress::Lzma' => { Inverse  => 'IO::Uncompress::UnLzma',
                              Error    => 'LzmaError',
                              TopLevel => 'lzma',
                              Raw      => 1,
                            },
    'IO::Uncompress::UnLzma' => { Inverse  => 'IO::Compress::Lzma',
                                  Error    => 'UnLzmaError',
                                  TopLevel => 'unlzma',
                                  Raw      => 1,
                                },

    'IO::Compress::Xz' => { Inverse  => 'IO::Uncompress::UnXz',
                            Error    => 'XzError',
                            TopLevel => 'xz',
                            Raw      => 0,
                          },
    'IO::Uncompress::UnXz' => { Inverse  => 'IO::Compress::Xz',
                                Error    => 'UnXzError',
                                TopLevel => 'unxz',
                                Raw      => 0,
                              },

    'IO::Compress::Lzip' => { Inverse  => 'IO::Uncompress::UnLzip',
                            Error    => 'LzipError',
                            TopLevel => 'lzip',
                            Raw      => 0,
                          },
    'IO::Uncompress::UnLzip' => { Inverse  => 'IO::Compress::Lzip',
                                Error    => 'UnLzipError',
                                TopLevel => 'unlzip',
                                Raw      => 0,
                              },

    'IO::Compress::PPMd' => { Inverse  => 'IO::Uncompress::UnPPMd',
                              Error    => 'PPMdError',
                              TopLevel => 'ppmd',
                              Raw      => 0,
                            },
    'IO::Uncompress::UnPPMd' => { Inverse  => 'IO::Compress::PPMd',
                                  Error    => 'UnPPMdError',
                                  TopLevel => 'unppmd',
                                  Raw      => 0,
                                },
    'IO::Compress::Zstd' => { Inverse  => 'IO::Uncompress::UnZstd',
                              Error    => 'ZstdError',
                              TopLevel => 'zstd',
                              Raw      => 0,
                            },
    'IO::Uncompress::UnZstd' => { Inverse  => 'IO::Compress::Zstd',
                                  Error    => 'UnZstdError',
                                  TopLevel => 'unzstd',
                                  Raw      => 0,
                                },

    'IO::Compress::DummyComp' => { Inverse  => 'IO::Uncompress::DummyUnComp',
                                   Error    => 'DummyCompError',
                                   TopLevel => 'dummycomp',
                                   Raw      => 0,
                                 },
    'IO::Uncompress::DummyUnComp' => { Inverse  => 'IO::Compress::DummyComp',
                                       Error    => 'DummyUnCompError',
                                       TopLevel => 'dummyunComp',
                                       Raw      => 0,
                                     },
);


for my $key (keys %TOP)
{
    no strict;
    no warnings;
    $TOP{$key}{Error}    = \${ $key . '::' . $TOP{$key}{Error}    };
    $TOP{$key}{TopLevel} =     $key . '::' . $TOP{$key}{TopLevel}  ;

    # Silence used once warning in really old perl
    my $dummy            = \${ $key . '::' . $TOP{$key}{Error}    };

    #$TOP{$key . "::" . $TOP{$key}{TopLevel} } = $TOP{$key};
}

sub uncompressBuffer
{
    my $compWith = shift ;
    my $buffer = shift ;


    my $out ;
    my $obj = $TOP{$compWith}{Inverse}->new( \$buffer, -Append => 1);
    1 while $obj->read($out) > 0 ;
    return $out ;

}


sub getInverse
{
    my $class = shift ;

    return $TOP{$class}{Inverse};
}

sub getErrorRef
{
    my $class = shift ;

    return $TOP{$class}{Error};
}

sub getTopFuncRef
{
    my $class = shift ;

    die "Cannot find $class"
        if ! defined $TOP{$class}{TopLevel};
    return \&{ $TOP{$class}{TopLevel} } ;
}

sub getTopFuncName
{
    my $class = shift ;

    return $TOP{$class}{TopLevel} ;
}

sub compressBuffer
{
    my $compWith = shift ;
    my $buffer = shift ;


    my $out ;
    die "Cannot find $compWith"
        if ! defined $TOP{$compWith}{Inverse};
    my $obj = $TOP{$compWith}{Inverse}->new( \$out);
    $obj->write($buffer) ;
    $obj->close();
    return $out ;
}

our ($AnyUncompressError);
BEGIN
{
    eval ' use IO::Uncompress::AnyUncompress qw(anyuncompress $AnyUncompressError); ';
}

sub anyUncompress
{
    my $buffer = shift ;
    my $already = shift;

    my @opts = ();
    if (ref $buffer && ref $buffer eq 'ARRAY')
    {
        @opts = @$buffer;
        $buffer = shift @opts;
    }

    if (ref $buffer)
    {
        croak "buffer is undef" unless defined $$buffer;
        croak "buffer is empty" unless length $$buffer;

    }


    my $data ;
    if (IO::Compress::Base::Common::isaFilehandle($buffer))
    {
        $data = readFile($buffer);
    }
    elsif (IO::Compress::Base::Common::isaFilename($buffer))
    {
        $data = readFile($buffer);
    }
    else
    {
        $data = $$buffer ;
    }

    if (defined $already && length $already)
    {

        my $got = substr($data, 0, length($already));
        substr($data, 0, length($already)) = '';

        is $got, $already, '  Already OK' ;
    }

    my $out = '';
    my $o = new IO::Uncompress::AnyUncompress \$data,
                    Append => 1,
                    Transparent => 0,
                    RawInflate => 1,
                    UnLzma     => 1,
                    @opts
        or croak "Cannot open buffer/file: $AnyUncompressError" ;

    1 while $o->read($out) > 0 ;

    croak "Error uncompressing -- " . $o->error()
        if $o->error() ;

    return $out ;
}

sub getHeaders
{
    my $buffer = shift ;
    my $already = shift;

    my @opts = ();
    if (ref $buffer && ref $buffer eq 'ARRAY')
    {
        @opts = @$buffer;
        $buffer = shift @opts;
    }

    if (ref $buffer)
    {
        croak "buffer is undef" unless defined $$buffer;
        croak "buffer is empty" unless length $$buffer;

    }


    my $data ;
    if (IO::Compress::Base::Common::isaFilehandle($buffer))
    {
        $data = readFile($buffer);
    }
    elsif (IO::Compress::Base::Common::isaFilename($buffer))
    {
        $data = readFile($buffer);
    }
    else
    {
        $data = $$buffer ;
    }

    if (defined $already && length $already)
    {

        my $got = substr($data, 0, length($already));
        substr($data, 0, length($already)) = '';

        is $got, $already, '  Already OK' ;
    }

    my $out = '';
    my $o = new IO::Uncompress::AnyUncompress \$data,
                MultiStream => 1,
                Append => 1,
                Transparent => 0,
                RawInflate => 1,
                UnLzma     => 1,
                @opts
        or croak "Cannot open buffer/file: $AnyUncompressError" ;

    1 while $o->read($out) > 0 ;

    croak "Error uncompressing -- " . $o->error()
        if $o->error() ;

    return ($o->getHeaderInfo()) ;

}

sub mkComplete
{
    my $class = shift ;
    my $data = shift;
    my $Error = getErrorRef($class);

    my $buffer ;
    my %params = ();

    if ($class eq 'IO::Compress::Gzip') {
        %params = (
            Name       => "My name",
            Comment    => "a comment",
            ExtraField => ['ab' => "extra"],
            HeaderCRC  => 1);
    }
    elsif ($class eq 'IO::Compress::Zip'){
        %params = (
            Name              => "My name",
            Comment           => "a comment",
            ZipComment        => "last comment",
            exTime            => [100, 200, 300],
            ExtraFieldLocal   => ["ab" => "extra1"],
            ExtraFieldCentral => ["cd" => "extra2"],
        );
    }

    my $z = new $class( \$buffer, %params)
        or croak "Cannot create $class object: $$Error";
    $z->write($data);
    $z->close();

    my $unc = getInverse($class);
    anyUncompress(\$buffer) eq $data
        or die "bad bad bad";
    my $u = new $unc( \$buffer);
    my $info = $u->getHeaderInfo() ;


    return wantarray ? ($info, $buffer) : $buffer ;
}

sub mkErr
{
    my $string = shift ;
    my ($dummy, $file, $line) = caller ;
    -- $line ;

    $file = quotemeta($file);

    #return "/$string\\s+at $file line $line/" if $] >= 5.006 ;
    return "/$string\\s+at /" ;
}

sub mkEvalErr
{
    my $string = shift ;

    #return "/$string\\s+at \\(eval /" if $] > 5.006 ;
    return "/$string\\s+at /" ;
}

sub dumpObj
{
    my $obj = shift ;

    my ($dummy, $file, $line) = caller ;

    if (@_)
    {
        print "#\n# dumpOBJ from $file line $line @_\n" ;
    }
    else
    {
        print "#\n# dumpOBJ from $file line $line \n" ;
    }

    my $max = 0 ;;
    foreach my $k (keys %{ *$obj })
    {
        $max = length $k if length $k > $max ;
    }

    foreach my $k (sort keys %{ *$obj })
    {
        my $v = $obj->{$k} ;
        $v = '-undef-' unless defined $v;
        my $pad = ' ' x ($max - length($k) + 2) ;
        print "# $k$pad: [$v]\n";
    }
    print "#\n" ;
}


sub getMultiValues
{
    my $class = shift ;

    return (0,0) if $class =~ /lzf|lzma|zstd/i;
    return (1,0);
}


sub gotScalarUtilXS
{
    eval ' use Scalar::Util "dualvar" ';
    return $@ ? 0 : 1 ;
}

package CompTestUtils;

1;
__END__
	t/Test/Builder.pm
	t/Test/More.pm
	t/Test/Simple.pm
	t/compress/CompTestUtils.pm
	t/compress/any.pl
	t/compress/anyunc.pl
	t/compress/destroy.pl
	t/compress/generic.pl
	t/compress/merge.pl
	t/compress/multi.pl
	t/compress/newtied.pl
	t/compress/oneshot.pl
	t/compress/prime.pl
	t/compress/tied.pl
	t/compress/truncate.pl
	t/compress/zlib-generic.plParsing config.in...
Building Zlib enabled
Auto Detect Gzip OS Code..
Setting Gzip OS Code to 3 [Unix/Default]
Looks Good.
