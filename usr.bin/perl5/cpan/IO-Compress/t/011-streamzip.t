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
use IO::Uncompress::Unzip 'unzip' ;

BEGIN
{
    plan(skip_all => "Needs Perl 5.005 or better - you have Perl $]" )
        if $] < 5.005 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 136 + $extra ;
}


my $Inc = join " ", map qq["-I$_"] => @INC;
$Inc = '"-MExtUtils::testlib"'
    if ! $ENV{PERL_CORE} && eval " require ExtUtils::testlib; " ;

my $Perl = ($ENV{'FULLPERL'} or $^X or 'perl') ;
$Perl = qq["$Perl"] if $^O eq 'MSWin32' ;

$Perl = "$Perl $Inc -w" ;
#$Perl .= " -Mblib " ;
my $binDir = $ENV{PERL_CORE} ? "../ext/IO-Compress/bin/"
                             : "./bin/";

my $hello1 = <<EOM ;
hello
this is
a test
message
x ttttt
xuuuuuu
the end
EOM




my $lex = LexFile->new( my $stderr );


sub check
{
    my $command = shift ;
    my $expected = shift ;

    my $lex = LexFile->new( my $stderr );

    my $cmd = "$command 2>$stderr";
    my $stdout = `$cmd` ;

    my $aok = 1 ;

    $aok &= is $?, 0, "  exit status is 0" ;

    $aok &= is readFile($stderr), '', "  no stderr" ;

    $aok &= is $stdout, $expected, "  expected content is ok"
        if defined $expected ;

    if (! $aok) {
        diag "Command line: $cmd";
        my ($file, $line) = (caller)[1,2];
        diag "Test called from $file, line $line";
    }

    1 while unlink $stderr;
}


# streamzip
# #########

{
    title "streamzip" ;

    my ($infile, $outfile);
    my $lex = LexFile->new( $infile, $outfile );

    writeFile($infile, $hello1) ;
    check "$Perl ${binDir}/streamzip <$infile >$outfile";

    my $uncompressed ;
    unzip $outfile => \$uncompressed;
    is $uncompressed, $hello1;
}

{
    title "streamzip - zipfile option" ;

    my ($infile, $outfile);
    my $lex = LexFile->new( $infile, $outfile );

    writeFile($infile, $hello1) ;
    check "$Perl ${binDir}/streamzip -zipfile $outfile <$infile";

    my $uncompressed ;
    unzip $outfile => \$uncompressed;
    is $uncompressed, $hello1;
}

for my $method (qw(store deflate bzip2 lzma xz zstd))
{
    SKIP:
    {
        if ($method eq 'lzma')
        {
            no warnings;
            eval { require IO::Compress::Lzma && defined &{ 'IO::Compress::Adapter::Bzip2::mkRawZipCompObject' } } ;
            skip "Method 'lzma' needs IO::Compress::Lzma\n", 8
                if $@;
        }

        if ($method eq 'zstd')
        {
            no warnings;
            eval { require IO::Compress::Zstd && defined &{ 'IO::Compress::Adapter::Zstd::mkRawZipCompObject' }} ;
            skip "Method 'zstd' needs IO::Compress::Zstd\n", 8
                if $@;
        }

        if ($method eq 'xz')
        {
            no warnings;
            eval { require IO::Compress::Xz && defined &{ 'IO::Compress::Adapter::Xz::mkRawZipCompObject' }} ;
            skip "Method 'xz' needs IO::Compress::Xz\n", 8
                if $@;
        }

        {
            title "streamzip method $method" ;

            skip "streaming unzip not supported with zstd\n", 7
                if $method eq 'zstd' ;

            my ($infile, $outfile);
            my $lex = LexFile->new( $infile, $outfile );

            writeFile($infile, $hello1) ;
            check "$Perl ${binDir}/streamzip -method $method <$infile >$outfile";

            my $uncompressed ;
            unzip $outfile => \$uncompressed;
            is $uncompressed, $hello1;
        }

        {
            title "streamzip $method- zipfile option" ;

            my ($infile, $outfile);
            my $lex = LexFile->new( $infile, $outfile );

            writeFile($infile, $hello1) ;
            check "$Perl ${binDir}/streamzip -zipfile $outfile -method $method <$infile";

            my $uncompressed ;
            unzip $outfile => \$uncompressed;
            is $uncompressed, $hello1;
        }
    }
}

for my $level (0 ..9)
{
    {
        title "streamzip level $level" ;

        my ($infile, $outfile);
        my $lex = LexFile->new( $infile, $outfile );

        writeFile($infile, $hello1) ;
        check "$Perl ${binDir}/streamzip -$level <$infile >$outfile";

        my $uncompressed ;
        unzip $outfile => \$uncompressed;
        is $uncompressed, $hello1;
    }

    {
        title "streamzip level $level- zipfile option" ;

        my ($infile, $outfile);
        my $lex = LexFile->new( $infile, $outfile );

        writeFile($infile, $hello1) ;
        check "$Perl ${binDir}/streamzip -zipfile $outfile -$level <$infile";

        my $uncompressed ;
        unzip $outfile => \$uncompressed;
        is $uncompressed, $hello1;
    }

}
