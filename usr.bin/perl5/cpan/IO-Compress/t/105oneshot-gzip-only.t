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


    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 70 + $extra ;

    use_ok('IO::Compress::Gzip', qw($GzipError)) ;
    use_ok('IO::Uncompress::Gunzip', qw($GunzipError)) ;


}


sub gzipGetHeader
{
    my $in = shift;
    my $content = shift ;
    my %opts = @_ ;

    my $out ;
    my $got ;

    ok IO::Compress::Gzip::gzip($in, \$out, %opts), "  gzip ok" ;
    ok IO::Uncompress::Gunzip::gunzip(\$out, \$got), "  gunzip ok"
        or diag $GunzipError ;
    is $got, $content, "  got expected content" ;

    my $gunz = IO::Uncompress::Gunzip->new( \$out, Strict => 0 )
        or diag "GunzipError is $IO::Uncompress::Gunzip::GunzipError" ;
    ok $gunz, "  Created IO::Uncompress::Gunzip object";
    my $hdr = $gunz->getHeaderInfo();
    ok $hdr, "  got Header info";
    my $uncomp ;
    ok $gunz->read($uncomp), " read ok" ;
    is $uncomp, $content, "  got expected content";
    ok $gunz->close, "  closed ok" ;

    return $hdr ;

}

{
    title "Check gzip header default NAME & MTIME settings" ;

    my $lex = LexFile->new( my $file1 );

    my $content = "hello ";
    my $hdr ;
    my $mtime ;

    writeFile($file1, $content);
    $mtime = (stat($file1))[9];
    # make sure that the gzip file isn't created in the same
    # second as the input file
    sleep 3 ;
    $hdr = gzipGetHeader($file1, $content);

    is $hdr->{Name}, $file1, "  Name is '$file1'";
    is $hdr->{Time}, $mtime, "  Time is ok";

    title "Override Name" ;

    writeFile($file1, $content);
    $mtime = (stat($file1))[9];
    sleep 3 ;
    $hdr = gzipGetHeader($file1, $content, Name => "abcde");

    is $hdr->{Name}, "abcde", "  Name is 'abcde'" ;
    is $hdr->{Time}, $mtime, "  Time is ok";

    title "Override Time" ;

    writeFile($file1, $content);
    $hdr = gzipGetHeader($file1, $content, Time => 1234);

    is $hdr->{Name}, $file1, "  Name is '$file1'" ;
    is $hdr->{Time}, 1234,  "  Time is 1234";

    title "Override Name and Time" ;

    writeFile($file1, $content);
    $hdr = gzipGetHeader($file1, $content, Time => 4321, Name => "abcde");

    is $hdr->{Name}, "abcde", "  Name is 'abcde'" ;
    is $hdr->{Time}, 4321, "  Time is 4321";

    title "Filehandle doesn't have default Name or Time" ;
    my $fh = IO::File->new( "< $file1" )
        or diag "Cannot open '$file1': $!\n" ;
    sleep 3 ;
    my $before = time ;
    $hdr = gzipGetHeader($fh, $content);
    my $after = time ;

    ok ! defined $hdr->{Name}, "  Name is undef";
    cmp_ok $hdr->{Time}, '>=', $before, "  Time is ok";
    cmp_ok $hdr->{Time}, '<=', $after, "  Time is ok";

    $fh->close;

    title "Buffer doesn't have default Name or Time" ;
    my $buffer = $content;
    $before = time ;
    $hdr = gzipGetHeader(\$buffer, $content);
    $after = time ;

    ok ! defined $hdr->{Name}, "  Name is undef";
    cmp_ok $hdr->{Time}, '>=', $before, "  Time is ok";
    cmp_ok $hdr->{Time}, '<=', $after, "  Time is ok";
}

# TODO add more error cases
