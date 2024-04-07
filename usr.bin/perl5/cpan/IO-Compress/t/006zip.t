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
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 108 + $extra ;

    use_ok('IO::Compress::Zip', qw(:all)) ;
    use_ok('IO::Uncompress::Unzip', qw(unzip $UnzipError)) ;

    eval {
           require IO::Compress::Bzip2 ;
           IO::Compress::Bzip2->import( 2.010 );
           require IO::Uncompress::Bunzip2 ;
           IO::Uncompress::Bunzip2->import( 2.010 );
         } ;

}


sub getContent
{
    my $filename = shift;

    my $u = IO::Uncompress::Unzip->new( $filename, Append => 1, @_ )
        or die "Cannot open $filename: $UnzipError";

    isa_ok $u, "IO::Uncompress::Unzip";

    my @content;
    my $status ;

    for ($status = 1; $status > 0 ; $status = $u->nextStream())
    {
        my $name = $u->getHeaderInfo()->{Name};
        #warn "Processing member $name\n" ;

        my $buff = '';
        1 while ($status = $u->read($buff)) ;

        push @content, $buff;
        last unless $status == 0;
    }

    die "Error processing $filename: $status $!\n"
        if $status < 0 ;

    return @content;
}



{
    title "Create a simple zip - All Deflate";

    my $lex = LexFile->new( my $file1 );

    my @content = (
                   'hello',
                   '',
                   'goodbye ',
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_DEFLATE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";
    $zip->newStream(Name=> "two", Method => ZIP_CM_DEFLATE);
    is $zip->write($content[1]), length($content[1]), "write";
    $zip->newStream(Name=> "three", Method => ZIP_CM_DEFLATE);
    is $zip->write($content[2]), length($content[2]), "write";
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], $content[0], "Got 1st entry";
    is $got[1], $content[1], "Got 2nd entry";
    is $got[2], $content[2], "Got 3nd entry";
}

SKIP:
{
    title "Create a simple zip - All Bzip2";

    skip "IO::Compress::Bzip2 not available", 9
        unless defined $IO::Compress::Bzip2::VERSION;

    my $lex = LexFile->new( my $file1 );

    my @content = (
                   'hello',
                   '',
                   'goodbye ',
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_BZIP2, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";
    $zip->newStream(Name=> "two", Method => ZIP_CM_BZIP2);
    is $zip->write($content[1]), length($content[1]), "write";
    $zip->newStream(Name=> "three", Method => ZIP_CM_BZIP2);
    is $zip->write($content[2]), length($content[2]), "write";
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], $content[0], "Got 1st entry";
    is $got[1], $content[1], "Got 2nd entry";
    is $got[2], $content[2], "Got 3nd entry";
}

SKIP:
{
    title "Create a simple zip - Deflate + Bzip2";

    skip "IO::Compress::Bzip2 not available", 9
        unless $IO::Compress::Bzip2::VERSION;

    my $lex = LexFile->new( my $file1 );

    my @content = (
                   'hello',
                   'and',
                   'goodbye ',
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_DEFLATE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";
    $zip->newStream(Name=> "two", Method => ZIP_CM_BZIP2);
    is $zip->write($content[1]), length($content[1]), "write";
    $zip->newStream(Name=> "three", Method => ZIP_CM_DEFLATE);
    is $zip->write($content[2]), length($content[2]), "write";
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], $content[0], "Got 1st entry";
    is $got[1], $content[1], "Got 2nd entry";
    is $got[2], $content[2], "Got 3nd entry";
}

{
    title "Create a simple zip - All STORE";

    my $lex = LexFile->new( my $file1 );

    my @content = (
                   'hello',
                   '',
                   'goodbye ',
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_STORE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";
    $zip->newStream(Name=> "two", Method => ZIP_CM_STORE);
    is $zip->write($content[1]), length($content[1]), "write";
    $zip->newStream(Name=> "three", Method => ZIP_CM_STORE);
    is $zip->write($content[2]), length($content[2]), "write";
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], $content[0], "Got 1st entry";
    is $got[1], $content[1], "Got 2nd entry";
    is $got[2], $content[2], "Got 3nd entry";
}

{
    title "Create a simple zip - Deflate + STORE";

    my $lex = LexFile->new( my $file1 );

    my @content = qw(
                   hello
                       and
                   goodbye
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_DEFLATE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";
    $zip->newStream(Name=> "two", Method => ZIP_CM_STORE);
    is $zip->write($content[1]), length($content[1]), "write";
    $zip->newStream(Name=> "three", Method => ZIP_CM_DEFLATE);
    is $zip->write($content[2]), length($content[2]), "write";
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], $content[0], "Got 1st entry";
    is $got[1], $content[1], "Got 2nd entry";
    is $got[2], $content[2], "Got 3nd entry";
}

{
    title "Create a simple zip - Deflate + zero length STORE";

    my $lex = LexFile->new( my $file1 );

    my @content = (
                   'hello ',
                   '',
                   'goodbye ',
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_DEFLATE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";
    $zip->newStream(Name=> "two", Method => ZIP_CM_STORE);
    is $zip->write($content[1]), length($content[1]), "write";
    $zip->newStream(Name=> "three", Method => ZIP_CM_DEFLATE);
    is $zip->write($content[2]), length($content[2]), "write";
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], $content[0], "Got 1st entry";
    ok $got[1] eq $content[1], "Got 2nd entry";
    is $got[2], $content[2], "Got 3nd entry";
}

{
    title "RT #72548";

    my $lex = LexFile->new( my $file1 );

    my $blockSize = 1024 * 16;

    my @content = (
                   'hello',
                   "x" x ($blockSize + 1)
                   );

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_STORE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content[0]), length($content[0]), "write";

    $zip->newStream(Name=> "two", Method => ZIP_CM_STORE);
    is $zip->write($content[1]), length($content[1]), "write";

    ok $zip->close(), "closed";

    my @got = getContent($file1, BlockSize => $blockSize);

    is $got[0], $content[0], "Got 1st entry";
    is $got[1], $content[1], "Got 2nd entry";
}

{
    title "Zip file with a single zero-length file";

    my $lex = LexFile->new( my $file1 );


    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one", Method => ZIP_CM_STORE, Stream => 0 );
    isa_ok $zip, "IO::Compress::Zip";

    $zip->newStream(Name=> "two", Method => ZIP_CM_STORE);
    ok $zip->close(), "closed";

    my @got = getContent($file1);

    is $got[0], "", "no content";
    is $got[1], "", "no content";
}

SKIP:
for my $method (ZIP_CM_DEFLATE, ZIP_CM_STORE, ZIP_CM_BZIP2)
{
    title "Read a line from zip, Method $method";

    skip "IO::Compress::Bzip2 not available", 14
        unless defined $IO::Compress::Bzip2::VERSION;

    my $content = "a single line\n";
    my $zip ;

    my $status = zip \$content => \$zip,
                    Method => $method,
                    Stream => 0,
                    Name => "123";
    is $status, 1, "  Created a zip file";

    my $u = IO::Uncompress::Unzip->new( \$zip );
    isa_ok $u, "IO::Uncompress::Unzip";

    is $u->getline, $content, "  Read first line ok";
    ok ! $u->getline, "  Second line doesn't exist";


}

{
    title "isMethodAvailable" ;

    ok IO::Compress::Zip::isMethodAvailable(ZIP_CM_STORE), "ZIP_CM_STORE available";
    ok IO::Compress::Zip::isMethodAvailable(ZIP_CM_DEFLATE), "ZIP_CM_DEFLATE available";
    #ok IO::Compress::Zip::isMethodAvailable(ZIP_CM_STORE), "ZIP_CM_STORE available";

    ok ! IO::Compress::Zip::isMethodAvailable(999), "999 not available";
}

{
    title "Member & Comment 0";

    my $lex = LexFile->new( my $file1 );

    my $content = 'hello' ;


    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "0", Comment => "0" );
    isa_ok $zip, "IO::Compress::Zip";

    is $zip->write($content), length($content), "write";

    ok $zip->close(), "closed";



    my $u = IO::Uncompress::Unzip->new( $file1, Append => 1, @_ )
        or die "Cannot open $file1: $UnzipError";

    isa_ok $u, "IO::Uncompress::Unzip";

    my $name = $u->getHeaderInfo()->{Name};

    is $u->getHeaderInfo()->{Name}, "0", "Name is '0'";
}


{
    title "nexStream regression";
    # https://github.com/pmqs/IO-Compress/issues/3

    my $lex = LexFile->new( my $file1 );

    my $content1 = qq["organisation_path","collection_occasion_key","episode_key"\n] ;

    my $zip = IO::Compress::Zip->new( $file1,
                    Name => "one" );
    isa_ok $zip, "IO::Compress::Zip";

    print $zip $content1;

    $zip->newStream(Name=> "two");

    my $content2 = <<EOM;
"key","value"
"version","2"
"type","PMHC"
EOM
    print $zip $content2;

    ok $zip->close(), "closed";


    my $u = IO::Uncompress::Unzip->new( $file1, Append => 1, @_ )
        or die "Cannot open $file1: $UnzipError";

    isa_ok $u, "IO::Uncompress::Unzip";

    my $name = $u->getHeaderInfo()->{Name};

    is $u->getHeaderInfo()->{Name}, "one", "Name is 'one'";

    ok $u->nextStream(), "nextStream OK";

    my $line = <$u>;

    is $line, qq["key","value"\n], "got line 1 from second member";
}