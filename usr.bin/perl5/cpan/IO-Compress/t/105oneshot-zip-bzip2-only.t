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

BEGIN {
    plan(skip_all => "oneshot needs Perl 5.005 or better - you have Perl $]" )
        if $] < 5.005 ;

    plan(skip_all => "IO::Compress::Bzip2 not available" )
        unless eval { require IO::Compress::Bzip2;
                      require IO::Uncompress::Bunzip2;
                      1
                    } ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 248 + $extra ;

    #use_ok('IO::Compress::Zip', qw(zip $ZipError :zip_method)) ;
    use_ok('IO::Compress::Zip', qw(:all)) ;
    use_ok('IO::Uncompress::Unzip', qw(unzip $UnzipError)) ;


}


sub zipGetHeader
{
    my $in = shift;
    my $content = shift ;
    my %opts = @_ ;

    my $out ;
    my $got ;

    ok zip($in, \$out, %opts), "  zip ok" ;
    ok unzip(\$out, \$got), "  unzip ok"
        or diag $UnzipError ;
    is $got, $content, "  got expected content" ;

    my $gunz = IO::Uncompress::Unzip->new( \$out, Strict => 0 )
        or diag "UnzipError is $IO::Uncompress::Unzip::UnzipError" ;
    ok $gunz, "  Created IO::Uncompress::Unzip object";
    my $hdr = $gunz->getHeaderInfo();
    ok $hdr, "  got Header info";
    my $uncomp ;
    ok $gunz->read($uncomp), " read ok" ;
    is $uncomp, $content, "  got expected content";
    ok $gunz->close, "  closed ok" ;

    return $hdr ;

}


for my $input (0, 1)
{
    for my $stream (0, 1)
    {
        for my $zip64 (0, 1)
        {
            #next if $zip64 && ! $stream;

            for my $method (ZIP_CM_STORE, ZIP_CM_DEFLATE, ZIP_CM_BZIP2)
            {
                title "Input $input, Stream $stream, Zip64 $zip64, Method $method";

                my $lex1 = LexFile->new( my $file1 );
                my $lex2 = LexFile->new( my $file2 );
                my $content = "hello ";
                my $in ;

                if ($input)
                {
                    writeFile($file2, $content);
                    $in = $file2;
                }
                else
                {
                    $in = \$content;
                }


                ok zip($in => $file1 , Method => $method,
                                       Zip64  => $zip64,
                                       Stream => $stream), " zip ok"
                    or diag $ZipError ;

                my $got ;
                ok unzip($file1 => \$got), "  unzip ok"
                    or diag $UnzipError ;

                is $got, $content, "  content ok";

                my $u = IO::Uncompress::Unzip->new( $file1 )
                    or diag $ZipError ;

                my $hdr = $u->getHeaderInfo();
                ok $hdr, "  got header";

                is $hdr->{Stream}, $stream, "  stream is $stream" ;
                is $hdr->{MethodID}, $method, "  MethodID is $method" ;
                is $hdr->{Zip64}, $zip64, "  Zip64 is $zip64" ;
            }
        }
    }
}

for my $stream (0, 1)
{
    for my $zip64 (0, 1)
    {
        next if $zip64 && ! $stream;

        for my $method (ZIP_CM_STORE, ZIP_CM_DEFLATE, ZIP_CM_BZIP2)
        {
            title "Stream $stream, Zip64 $zip64, Method $method";

            my $file1;
            my $file2;
            my $zipfile;
            my $lex = LexFile->new( $file1, $file2, $zipfile );

            my $content1 = "hello ";
            writeFile($file1, $content1);

            my $content2 = "goodbye ";
            writeFile($file2, $content2);

            my %content = ( $file1 => $content1,
                            $file2 => $content2,
                          );

            ok zip([$file1, $file2] => $zipfile , Method => $method,
                                                  Zip64  => $zip64,
                                                  Stream => $stream), " zip ok"
                or diag $ZipError ;

            for my $file ($file1, $file2)
            {
                my $got ;
                ok unzip($zipfile => \$got, Name => $file), "  unzip $file ok"
                    or diag $UnzipError ;

                is $got, $content{$file}, "  content ok";
            }
        }
    }
}

# TODO add more error cases
