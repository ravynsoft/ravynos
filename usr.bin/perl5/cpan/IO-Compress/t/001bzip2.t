BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir 't' if -d 't';
        @INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);

use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 841 + $extra ;
};


use IO::Compress::Bzip2     qw($Bzip2Error) ;
use IO::Uncompress::Bunzip2 qw($Bunzip2Error) ;


my $CompressClass   = 'IO::Compress::Bzip2';
my $UncompressClass = getInverse($CompressClass);
my $Error           = getErrorRef($CompressClass);
my $UnError         = getErrorRef($UncompressClass);

sub myBZreadFile
{
    my $filename = shift ;
    my $init = shift ;


    my $fil = $UncompressClass->can('new')->( $UncompressClass, $filename,
                                    -Strict   => 1,
                                    -Append   => 1
                                    );

    my $data = '';
    $data = $init if defined $init ;
    1 while $fil->read($data) > 0;

    $fil->close ;
    return $data ;
}


{

    title "Testing $CompressClass Errors";

    my $buffer ;

    for my $value (undef, -1, 'fred')
    {
        my $stringValue = defined $value ? $value : 'undef';
        title "BlockSize100K => $stringValue";
        my $err = "Parameter 'BlockSize100K' must be an unsigned int, got '$stringValue'";
        my $bz ;
        eval { $bz = IO::Compress::Bzip2->new(\$buffer, BlockSize100K => $value) };
        like $@,  mkErr("IO::Compress::Bzip2: $err"),
            "  value $stringValue is bad";
        is $Bzip2Error, "IO::Compress::Bzip2: $err",
            "  value $stringValue is bad";
        ok ! $bz, "  no bz object";
    }

    for my $value (0, 10, 99999)
    {
        my $stringValue = defined $value ? $value : 'undef';
        title "BlockSize100K => $stringValue";
        my $err = "Parameter 'BlockSize100K' not between 1 and 9, got $stringValue";
        my $bz ;
        eval { $bz = IO::Compress::Bzip2->new(\$buffer, BlockSize100K => $value) };
        like $@,  mkErr("IO::Compress::Bzip2: $err"),
            "  value $stringValue is bad";
        is $Bzip2Error,  "IO::Compress::Bzip2: $err",
            "  value $stringValue is bad";
        ok ! $bz, "  no bz object";
    }

    for my $value (undef, -1, 'fred')
    {
        my $stringValue = defined $value ? $value : 'undef';
        title "WorkFactor => $stringValue";
        my $err = "Parameter 'WorkFactor' must be an unsigned int, got '$stringValue'";
        my $bz ;
        eval { $bz = IO::Compress::Bzip2->new(\$buffer, WorkFactor => $value) };
        like $@,  mkErr("IO::Compress::Bzip2: $err"),
            "  value $stringValue is bad";
        is $Bzip2Error, "IO::Compress::Bzip2: $err",
            "  value $stringValue is bad";
        ok ! $bz, "  no bz object";
    }

    for my $value (251, 99999)
    {
        my $stringValue = defined $value ? $value : 'undef';
        title "WorkFactor => $stringValue";
        my $err = "Parameter 'WorkFactor' not between 0 and 250, got $stringValue";
        my $bz ;
        eval { $bz = IO::Compress::Bzip2->new(\$buffer, WorkFactor => $value) };
        like $@,  mkErr("IO::Compress::Bzip2: $err"),
            "  value $stringValue is bad";
        is $Bzip2Error,  "IO::Compress::Bzip2: $err",
            "  value $stringValue is bad";
        ok ! $bz, "  no bz object";
    }

}


{
    title "Testing $UncompressClass Errors";

    my $buffer ;

    for my $value (-1, 'fred')
    {
        my $stringValue = defined $value ? $value : 'undef';
        title "Small => $stringValue";
        my $err = "Parameter 'Small' must be an int, got '$stringValue'";
        my $bz ;
        eval { $bz = IO::Uncompress::Bunzip2->new(\$buffer, Small => $value) };
        like $@,  mkErr("IO::Uncompress::Bunzip2: $err"),
            "  value $stringValue is bad";
        is $Bunzip2Error, "IO::Uncompress::Bunzip2: $err",
            "  value $stringValue is bad";
        ok ! $bz, "  no bz object";
    }

}

{
    title "Testing $CompressClass and $UncompressClass";

    my $hello = <<EOM ;
hello world
this is a test
EOM

    for my $value ( 1 .. 9 )
    {
        title "$CompressClass - BlockSize100K => $value";
        my $lex = LexFile->new( my $name );
        my $bz ;
        $bz = IO::Compress::Bzip2->new($name, BlockSize100K => $value)
            or diag $IO::Compress::Bzip2::Bzip2Error ;
        ok $bz, "  bz object ok";
        $bz->write($hello);
        $bz->close($hello);

        is myBZreadFile($name), $hello, "  got expected content";
    }

    for my $value ( 0 .. 250 )
    {
        title "$CompressClass - WorkFactor => $value";
        my $lex = LexFile->new( my $name );
        my $bz ;
        $bz = IO::Compress::Bzip2->new($name, WorkFactor => $value);
        ok $bz, "  bz object ok";
        $bz->write($hello);
        $bz->close($hello);

        is myBZreadFile($name), $hello, "  got expected content";
    }

    for my $value ( 0 .. 1 )
    {
        title "$UncompressClass - Small => $value";
        my $lex = LexFile->new( my $name );
        my $bz ;
        $bz = IO::Compress::Bzip2->new($name);
        ok $bz, "  bz object ok";
        $bz->write($hello);
        $bz->close($hello);

        my $fil = $UncompressClass->can('new')->( $UncompressClass, $name,
                                       Append  => 1,
                                       Small   => $value );

        my $data = '';
        1 while $fil->read($data) > 0;

        $fil->close ;

        is $data, $hello, " got expected";
    }
}


1;
