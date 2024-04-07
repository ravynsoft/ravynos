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

use File::Spec ;
use Test::More ;
use CompTestUtils;

my $GZIP ;


sub ExternalGzipWorks
{
    my $lex = LexFile->new( my $outfile );
    my $content = qq {
Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Ut tempus odio id
 dolor. Camelus perlus.  Larrius in lumen numen.  Dolor en quiquum filia
 est.  Quintus cenum parat.
};

    writeWithGzip($outfile, $content)
        or return 0;

    my $got ;
    readWithGzip($outfile, $got)
        or return 0;

    if ($content ne $got)
    {
        diag "Uncompressed content is wrong";
        return 0 ;
    }

    return 1 ;
}

sub readWithGzip
{
    my $file = shift ;

    my $lex = LexFile->new( my $outfile );

    my $comp = "$GZIP -d -c" ;

    if ( system("$comp $file >$outfile") == 0 )
    {
        $_[0] = readFile($outfile);
        return 1
    }

    diag "'$comp' failed: \$?=$? \$!=$!";
    return 0 ;
}

sub getGzipInfo
{
    my $file = shift ;
}

sub writeWithGzip
{
    my $file = shift ;
    my $content = shift ;
    my $options = shift || '';

    my $lex = LexFile->new( my $infile );
    writeFile($infile, $content);

    unlink $file ;
    my $comp = "$GZIP -c $options $infile >$file" ;

    return 1
        if system($comp) == 0 ;

    diag "'$comp' failed: \$?=$? \$!=$!";
    return 0 ;
}

BEGIN {

    # Check external gzip is available
    my $name = $^O =~ /mswin/i ? 'gzip.exe' : 'gzip';
    my $split = $^O =~ /mswin/i ? ";" : ":";

    for my $dir (reverse split $split, $ENV{PATH})
    {
        $GZIP = File::Spec->catfile($dir,$name)
            if -x File::Spec->catfile($dir,$name)
    }

    # Handle spaces in path to gzip
    $GZIP = "\"$GZIP\"" if defined $GZIP && $GZIP =~ /\s/;

    plan(skip_all => "Cannot find $name")
        if ! $GZIP ;

    plan(skip_all => "$name doesn't work as expected")
        if ! ExternalGzipWorks();


    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 7 + $extra ;

    use_ok('IO::Compress::Gzip',     ':all') ;
    use_ok('IO::Uncompress::Gunzip', ':all') ;

}


{
    title "Test interop with $GZIP" ;

    my $file;
    my $file1;
    my $lex = LexFile->new( $file, $file1 );
    my $content = qq {
Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Ut tempus odio id
 dolor. Camelus perlus.  Larrius in lumen numen.  Dolor en quiquum filia
 est.  Quintus cenum parat.
};
    my $got;

    ok writeWithGzip($file, $content), "writeWithGzip ok";

    gunzip $file => \$got ;
    is $got, $content, "got content";


    gzip \$content => $file1;
    $got = '';
    ok readWithGzip($file1, $got), "readWithGzip ok";
    is $got, $content, "got content";
}
