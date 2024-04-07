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
use File::Spec ;
use CompTestUtils;

BEGIN {
    plan(skip_all => "oneshot needs Perl 5.005 or better - you have Perl $]" )
        if $] < 5.005 ;


    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 230 + $extra ;

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

{
    title "Check zip header default NAME & MTIME settings" ;

    my $lex = LexFile->new( my $file1 );

    my $content = "hello ";
    my $hdr ;
    my $mtime ;

    writeFile($file1, $content);
    $mtime = (stat($file1))[9];
    # make sure that the zip file isn't created in the same
    # second as the input file
    sleep 3 ;
    $hdr = zipGetHeader($file1, $content);

    is $hdr->{Name}, $file1, "  Name is '$file1'";
    is $hdr->{Time}>>1, $mtime>>1, "  Time is ok";

    title "Override Name" ;

    writeFile($file1, $content);
    $mtime = (stat($file1))[9];
    sleep 3 ;
    $hdr = zipGetHeader($file1, $content, Name => "abcde");

    is $hdr->{Name}, "abcde", "  Name is 'abcde'" ;
    is $hdr->{Time} >> 1, $mtime >> 1, "  Time is ok";

    title "Override Time" ;

    writeFile($file1, $content);
    my $useTime = time + 2000 ;
    $hdr = zipGetHeader($file1, $content, Time => $useTime);

    is $hdr->{Name}, $file1, "  Name is '$file1'" ;
    is $hdr->{Time} >> 1 , $useTime >> 1 ,  "  Time is $useTime";

    title "Override Name and Time" ;

    $useTime = time + 5000 ;
    writeFile($file1, $content);
    $hdr = zipGetHeader($file1, $content, Time => $useTime, Name => "abcde");

    is $hdr->{Name}, "abcde", "  Name is 'abcde'" ;
    is $hdr->{Time} >> 1 , $useTime >> 1 , "  Time is $useTime";

    title "Filehandle doesn't have default Name or Time" ;
    my $fh = IO::File->new( "< $file1" )
        or diag "Cannot open '$file1': $!\n" ;
    sleep 3 ;
    my $before = time ;
    $hdr = zipGetHeader($fh, $content);
    my $after = time ;

    ok ! defined $hdr->{Name}, "  Name is undef";
    cmp_ok $hdr->{Time} >> 1, '>=', $before >> 1, "  Time is ok";
    cmp_ok $hdr->{Time} >> 1, '<=', $after >> 1, "  Time is ok";

    $fh->close;

    title "Buffer doesn't have default Name or Time" ;
    my $buffer = $content;
    $before = time ;
    $hdr = zipGetHeader(\$buffer, $content);
    $after = time ;

    ok ! defined $hdr->{Name}, "  Name is undef";
    cmp_ok $hdr->{Time} >> 1, '>=', $before >> 1, "  Time is ok";
    cmp_ok $hdr->{Time} >> 1, '<=', $after >> 1, "  Time is ok";
}

{
    title "Check CanonicalName & FilterName";

    my $lex = LexFile->new( my $file1 );

    my $content = "hello" ;
    writeFile($file1, $content);
    my $hdr;

    my $abs = File::Spec->catfile("", "fred", "joe");
    $hdr = zipGetHeader($file1, $content, Name => $abs, CanonicalName => 1) ;
    is $hdr->{Name}, "fred/joe", "  Name is 'fred/joe'" ;

    $hdr = zipGetHeader($file1, $content, Name => $abs, CanonicalName => 0) ;
    is $hdr->{Name}, File::Spec->catfile("", "fred", "joe"), "  Name is '/fred/joe'" ;

    $hdr = zipGetHeader($file1, $content, FilterName => sub {$_ = "abcde"});
    is $hdr->{Name}, "abcde", "  Name is 'abcde'" ;

    $hdr = zipGetHeader($file1, $content, Name => $abs,
         CanonicalName => 1,
         FilterName => sub { s/joe/jim/ });
    is $hdr->{Name}, "fred/jim", "  Name is 'fred/jim'" ;

    $hdr = zipGetHeader($file1, $content, Name => $abs,
         CanonicalName => 0,
         FilterName => sub { s/joe/jim/ });
    is $hdr->{Name}, File::Spec->catfile("", "fred", "jim"), "  Name is '/fred/jim'" ;
}

{
    title "Detect encrypted zip file";

    my $files = "./t/" ;
    $files = "./" if $ENV{PERL_CORE} ;
    $files .= "files/";

    my $zipfile = "$files/encrypt-standard.zip" ;
    my $output;

    ok ! unzip "$files/encrypt-standard.zip" => \$output ;
    like $UnzipError, qr/Encrypted content not supported/ ;

    ok ! unzip "$files/encrypt-aes.zip" => \$output ;
    like $UnzipError, qr/Encrypted content not supported/ ;
}

{
    title "jar file with deflated directory";

    # Create Jar as follow
    #   echo test > file && jar c file > jar.zip

    # Note the deflated directory META-INF with length 0 & size 2
    #
    # $ unzip -vl t/files/jar.zip
    # Archive:  t/files/jar.zip
    #  Length   Method    Size  Cmpr    Date    Time   CRC-32   Name
    # --------  ------  ------- ---- ---------- ----- --------  ----
    #        0  Defl:N        2   0% 2019-09-07 22:35 00000000  META-INF/
    #       54  Defl:N       53   2% 2019-09-07 22:35 934e49ff  META-INF/MANIFEST.MF
    #        5  Defl:N        7 -40% 2019-09-07 22:35 3bb935c6  file
    # --------          -------  ---                            -------
    #       59               62  -5%                            3 files


    my $files = "./t/" ;
    $files = "./" if $ENV{PERL_CORE} ;
    $files .= "files/";

    my $zipfile = "$files/jar.zip" ;
    my $output;

    ok unzip $zipfile => \$output ;

    is $output, "" ;

}

for my $stream (0, 1)
{
    for my $zip64 (0, 1)
    {
        #next if $zip64 && ! $stream;

        for my $method (ZIP_CM_STORE, ZIP_CM_DEFLATE)
        {

            title "Stream $stream, Zip64 $zip64, Method $method";

            my $lex = LexFile->new( my $file1 );

            my $content = "hello ";
            #writeFile($file1, $content);

            my $status = zip(\$content => $file1 ,
                               Method => $method,
                               Stream => $stream,
                               Zip64  => $zip64);

             ok $status, "  zip ok"
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

for my $stream (0, 1)
{
    for my $zip64 (0, 1)
    {
        next if $zip64 && ! $stream;
        for my $method (ZIP_CM_STORE, ZIP_CM_DEFLATE)
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

my $ebcdic_skip_msg = "Input file is in an alien character set";

SKIP: {
    skip $ebcdic_skip_msg, 3 if ord "A" != 65;

    title "Regression: ods streaming issue";

    # To execute this test on a non-ASCII machine, we could open the zip file
    # without using the Name parameter, or xlate the parameter to ASCII, and
    # also xlate the contents to native.

    # The file before meta.xml in test.ods is content.xml.
    # Issue was triggered because content.xml was stored
    # as streamed and the code to walk the compressed streaming
    # content assumed that all of the input buffer was consumed
    # in a single call to "uncompr".

    my $files = "./t/" ;
    $files = "./" if $ENV{PERL_CORE} ;
    $files .= "files/";

    my $zipfile = "$files/test.ods" ;
    my $file = "meta.xml";

    my $got;

    ok unzip($zipfile => \$got, Name => $file), "  unzip $file ok"
        or diag $UnzipError ;

    my $meta = '<?xml version="1.0" encoding="UTF-8"?>
<office:document-meta xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:grddl="http://www.w3.org/2003/g/data-view#" office:version="1.2"><office:meta><meta:creation-date>2018-12-25T11:36:11.437260543</meta:creation-date><dc:date>2018-12-25T11:36:55.657945697</dc:date><meta:editing-duration>PT54S</meta:editing-duration><meta:editing-cycles>1</meta:editing-cycles><meta:document-statistic meta:table-count="1" meta:cell-count="3" meta:object-count="0"/><meta:generator>LibreOffice/6.0.7.3$Linux_X86_64 LibreOffice_project/00m0$Build-3</meta:generator></office:meta></office:document-meta>';

    is $got, $meta, "  content ok";

}

SKIP: {
    skip $ebcdic_skip_msg, 3 if ord "A" != 65;

    title "Regression: odt non-streaming issue";
    # https://github.com/pmqs/IO-Compress/issues/13

    # Some programs (LibreOffice) mark entries as Streamed (bit 3 of the General Purpose Bit Flags field is set) ,
    # but still fill out the Compressed Length, Uncompressed Length & CRC32 fields in the local file header

    my $files = "./t/" ;
    $files = "./" if $ENV{PERL_CORE} ;
    $files .= "files/";

    my $zipfile = "$files/testfile1.odt" ;
    my $file = "manifest.rdf";

    my $got;

    ok unzip($zipfile => \$got, Name => $file), "  unzip $file ok"
        or diag $UnzipError ;

    my $meta = <<'EOM';
<?xml version="1.0" encoding="utf-8"?>
<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about="">
    <rdf:type rdf:resource="http://docs.oasis-open.org/ns/office/1.2/meta/pkg#Document"/>
  </rdf:Description>
</rdf:RDF>
EOM
    is $got, $meta, "  content ok";
}

# TODO add more error cases
