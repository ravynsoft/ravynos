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
use Data::Dumper;

use IO::Compress::Zip     qw($ZipError);
use IO::Uncompress::Unzip qw($UnzipError);

BEGIN {
    plan skip_all => "Encode is not available"
        if $] < 5.006 ;

    eval { require Encode; Encode->import(); };

    plan skip_all => "Encode is not available"
        if $@ ;

    plan skip_all => "Encode not working in perl $]"
        if $] >= 5.008 && $] < 5.008004 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 28 + $extra;
}

{
    title "EFS set in zip: Create a simple zip - language encoding flag set";

    my $lex = LexFile->new( my $file1 );

    my @names = ( 'alpha \N{GREEK SMALL LETTER ALPHA}',
                  'beta \N{GREEK SMALL LETTER BETA}',
                  'gamma \N{GREEK SMALL LETTER GAMMA}',
                  'delta \N{GREEK SMALL LETTER DELTA}'
                ) ;

    my @encoded = map { Encode::encode_utf8($_) } @names;

    my @n = @names;

    my $zip = IO::Compress::Zip->new( $file1,
                    Name =>  $names[0], Efs => 1 );

    my $content = 'Hello, world!';
    ok $zip->print($content), "print";
    $zip->newStream(Name => $names[1], Efs => 1);
    ok $zip->print($content), "print";
    $zip->newStream(Name => $names[2], Efs => 1);
    ok $zip->print($content), "print";
    $zip->newStream(Name => $names[3], Efs => 1);
    ok $zip->print($content), "print";
    ok $zip->close(), "closed";

    {
        my $u = IO::Uncompress::Unzip->new( $file1, Efs => 1 )
            or die "Cannot open $file1: $UnzipError";

        my $status;
        my @efs;
        my @unzip_names;
        for ($status = 1; $status > 0; $status = $u->nextStream(Efs => 1))
        {
            push @efs, $u->getHeaderInfo()->{efs};
            push @unzip_names, $u->getHeaderInfo()->{Name};
        }

        die "Error processing $file1: $status $!\n"
            if $status < 0;

        is_deeply \@efs, [1, 1, 1, 1], "language encoding flag set"
            or diag "Got " . Dumper(\@efs);
        is_deeply \@unzip_names, [@names], "Names round tripped"
            or diag "Got " . Dumper(\@unzip_names);
    }

    {
        my $u = IO::Uncompress::Unzip->new( $file1, Efs => 0 )
            or die "Cannot open $file1: $UnzipError";

        my $status;
        my @efs;
        my @unzip_names;
        for ($status = 1; $status > 0; $status = $u->nextStream(Efs => 0))
        {
            push @efs, $u->getHeaderInfo()->{efs};
            push @unzip_names, $u->getHeaderInfo()->{Name};
        }

        die "Error processing $file1: $status $!\n"
            if $status < 0;

        is_deeply \@efs, [1, 1, 1, 1], "language encoding flag set"
            or diag "Got " . Dumper(\@efs);
        is_deeply \@unzip_names, [@names], "Names round tripped"
            or diag "Got " . Dumper(\@unzip_names);
    }
}


{
    title "Create a simple zip - language encoding flag not set";

    my $lex = LexFile->new( my $file1 );

    my @names = ( 'alpha \N{GREEK SMALL LETTER ALPHA}',
                  'beta \N{GREEK SMALL LETTER BETA}',
                  'gamma \N{GREEK SMALL LETTER GAMMA}',
                  'delta \N{GREEK SMALL LETTER DELTA}'
                ) ;

    my @n = @names;

    my $zip = IO::Compress::Zip->new( $file1,
                    Name =>  $names[0], Efs => 0 );

    my $content = 'Hello, world!';
    ok $zip->print($content), "print";
    $zip->newStream(Name => $names[1], Efs => 0);
    ok $zip->print($content), "print";
    $zip->newStream(Name => $names[2], Efs => 0);
    ok $zip->print($content), "print";
    $zip->newStream(Name => $names[3]);
    ok $zip->print($content), "print";
    ok $zip->close(), "closed";

    my $u = IO::Uncompress::Unzip->new( $file1, Efs => 0 )
        or die "Cannot open $file1: $UnzipError";

    my $status;
    my @efs;
    my @unzip_names;
    for ($status = 1; $status > 0; $status = $u->nextStream())
    {
        push @efs, $u->getHeaderInfo()->{efs};
        push @unzip_names, $u->getHeaderInfo()->{Name};
    }

    die "Error processing $file1: $status $!\n"
        if $status < 0;

    is_deeply \@efs, [0, 0, 0, 0], "language encoding flag set"
        or diag "Got " . Dumper(\@efs);
    is_deeply \@unzip_names, [@names], "Names round tripped"
        or diag "Got " . Dumper(\@unzip_names);
}

{
    title "zip: EFS => 0 filename not valid utf8 - language encoding flag not set";

    my $lex = LexFile->new( my $file1 );

    # Invalid UTF8
    my $name = "a\xFF\x{100}";

    my $zip = IO::Compress::Zip->new( $file1,
                    Name =>  $name, Efs => 0 );

    ok $zip->print("abcd"), "print";
    ok $zip->close(), "closed";

    my $u = IO::Uncompress::Unzip->new( $file1 )
        or die "Cannot open $file1: $UnzipError";

    ok $u->getHeaderInfo()->{Name} eq $name, "got bad filename";
}

{
    title "unzip: EFS => 0 filename not valid utf8 - language encoding flag set";

    my $filename = "t/files/bad-efs.zip" ;
    my $name = "\xF0\xA4\xAD";

    my $u = IO::Uncompress::Unzip->new( $filename, efs => 0 )
        or die "Cannot open $filename: $UnzipError";

    ok $u->getHeaderInfo()->{Name} eq $name, "got bad filename";
}

SKIP: {
    title "unzip: EFS => 1 filename not valid utf8 - language encoding flag set";

    # The name hard-coded into this pre-built file is not illegal UTF-EBCDIC
    skip "ASCII-centric test", 1, unless ord "A" == 65;

    my $filename = "t/files/bad-efs.zip" ;

    eval { my $u = IO::Uncompress::Unzip->new( $filename, efs => 1 )
        or die "Cannot open $filename: $UnzipError" };

    like $@, qr/Zip Filename not UTF-8/,
            "  Zip Filename not UTF-8" ;

}

{
    title "EFS => 1 - filename not valid utf8 - catch bad content writing to zip";

    my $lex = LexFile->new( my $file1 );

    # Invalid UTF8
    my $name = "a\xFF\x{100}";

    eval { my $zip = IO::Compress::Zip->new( $file1,
                    Name =>  $name, Efs => 1 ) } ;

    like $@,  qr/Wide character in zip filename/,
                 "  wide characters in zip filename";
}