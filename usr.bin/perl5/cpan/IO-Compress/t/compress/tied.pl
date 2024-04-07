
use lib 't';
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

our ($BadPerl, $UncompressClass);

BEGIN
{
    plan(skip_all => "Tied Filehandle needs Perl 5.005 or better" )
        if $] < 5.005 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    my $tests ;
    $BadPerl = ($] >= 5.006 and $] <= 5.008) ;

    if ($BadPerl) {
        $tests = 241 ;
    }
    else {
        $tests = 249 ;
    }

    plan tests => $tests + $extra ;

}


use IO::Handle qw(SEEK_SET SEEK_CUR SEEK_END);



sub myGZreadFile
{
    my $filename = shift ;
    my $init = shift ;


    my $fil = $UncompressClass->can('new')->( $UncompressClass, $filename,
                                    -Strict   => 1,
                                    -Append   => 1
                                    );

    my $data ;
    $data = $init if defined $init ;
    1 while $fil->read($data) > 0;

    $fil->close ;
    return $data ;
}

sub run
{

    my $CompressClass   = identify();
    $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);

    {
        next if $BadPerl ;


        title "Testing $CompressClass";


        my $x ;
        my $gz = $CompressClass->can('new')->( $CompressClass, \$x);

        my $buff ;

        eval { getc($gz) } ;
        like $@, mkErr("^getc Not Available: File opened only for output");

        eval { read($gz, $buff, 1) } ;
        like $@, mkErr("^read Not Available: File opened only for output");

        eval { <$gz>  } ;
        like $@, mkErr("^readline Not Available: File opened only for output");

    }

    {
        next if $BadPerl;
        $UncompressClass = getInverse($CompressClass);

        title "Testing $UncompressClass";

        my $gc ;
        my $guz = $CompressClass->can('new')->( $CompressClass, \$gc);
        $guz->write("abc") ;
        $guz->close();

        my $x ;
        my $gz = $UncompressClass->can('new')->( $UncompressClass, \$gc);

        my $buff ;

        eval { print $gz "abc" } ;
        like $@, mkErr("^print Not Available: File opened only for intput");

        eval { printf $gz "fmt", "abc" } ;
        like $@, mkErr("^printf Not Available: File opened only for intput");

        #eval { write($gz, $buff, 1) } ;
        #like $@, mkErr("^write Not Available: File opened only for intput");

    }

    {
        $UncompressClass = getInverse($CompressClass);

        title "Testing $CompressClass and $UncompressClass";


        {
            # Write
            # these tests come almost 100% from IO::String

            my $lex = LexFile->new( my $name );

            my $io = $CompressClass->new($name);

            is $io->tell(), 0 ;

            my $heisan = "Heisan\n";
            print $io $heisan ;

            ok ! $io->eof;

            is $io->tell(), length($heisan) ;

            print($io "a", "b", "c");

            {
                local($\) = "\n";
                print $io "d", "e";
                local($,) = ",";
                print $io "f", "g", "h";
            }

            my $foo = "1234567890";

            ok syswrite($io, $foo, length($foo)) == length($foo) ;
            if ( $] < 5.6 )
              { is $io->syswrite($foo, length $foo), length $foo }
            else
              { is $io->syswrite($foo), length $foo }
            ok $io->syswrite($foo, length($foo)) == length $foo;
            ok $io->write($foo, length($foo), 5) == 5;
            ok $io->write("xxx\n", 100, -1) == 1;

            for (1..3) {
                printf $io "i(%d)", $_;
                $io->printf("[%d]\n", $_);
            }
            select $io;
            print "\n";
            select STDOUT;

            close $io ;

            ok $io->eof;

            is myGZreadFile($name), "Heisan\nabcde\nf,g,h\n" .
                                    ("1234567890" x 3) . "67890\n" .
                                        "i(1)[1]\ni(2)[2]\ni(3)[3]\n\n";


        }

        {
            # Read
            my $str = <<EOT;
This is an example
of a paragraph


and a single line.

EOT

            my $lex = LexFile->new( my $name );

            my $iow = $CompressClass->can('new')->( $CompressClass, $name );
            print $iow $str ;
            close $iow;

            my @tmp;
            my $buf;
            {
                my $io = $UncompressClass->can('new')->( $UncompressClass, $name );

                ok ! $io->eof, "  Not EOF";
                is $io->tell(), 0, "  Tell is 0" ;
                my @lines = <$io>;
                is @lines, 6, "  Line is 6"
                    or print "# Got " . scalar(@lines) . " lines, expected 6\n" ;
                is $lines[1], "of a paragraph\n" ;
                is join('', @lines), $str ;
                is $., 6;
                is $io->tell(), length($str) ;

                ok $io->eof;

                ok ! ( defined($io->getline)  ||
                          (@tmp = $io->getlines) ||
                          defined(<$io>)         ||
                          defined($io->getc)     ||
                          read($io, $buf, 100)   != 0) ;
            }


            {
                local $/;  # slurp mode
                my $io = $UncompressClass->new($name);
                ok !$io->eof;
                my @lines = $io->getlines;
                ok $io->eof;
                ok @lines == 1 && $lines[0] eq $str;

                $io = $UncompressClass->new($name);
                ok ! $io->eof;
                my $line = <$io>;
                ok $line eq $str;
                ok $io->eof;
            }

            {
                local $/ = "";  # paragraph mode
                my $io = $UncompressClass->new($name);
                ok ! $io->eof;
                my @lines = <$io>;
                ok $io->eof;
                ok @lines == 2
                    or print "# Got " . scalar(@lines) . " lines, expected 2\n" ;
                ok $lines[0] eq "This is an example\nof a paragraph\n\n\n"
                    or print "# $lines[0]\n";
                ok $lines[1] eq "and a single line.\n\n";
            }

            {
                local $/ = "is";
                my $io = $UncompressClass->new($name);
                my @lines = ();
                my $no = 0;
                my $err = 0;
                ok ! $io->eof;
                while (<$io>) {
                    push(@lines, $_);
                    $err++ if $. != ++$no;
                }

                ok $err == 0 ;
                ok $io->eof;

                ok @lines == 3
                    or print "# Got " . scalar(@lines) . " lines, expected 3\n" ;
                ok join("-", @lines) eq
                                 "This- is- an example\n" .
                                "of a paragraph\n\n\n" .
                                "and a single line.\n\n";
            }


            # Test read

            {
                my $io = $UncompressClass->new($name);


                if (! $BadPerl) {
                    eval { read($io, $buf, -1) } ;
                    like $@, mkErr("length parameter is negative");
                }

                is read($io, $buf, 0), 0, "Requested 0 bytes" ;

                ok read($io, $buf, 3) == 3 ;
                ok $buf eq "Thi";

                ok sysread($io, $buf, 3, 2) == 3 ;
                ok $buf eq "Ths i"
                    or print "# [$buf]\n" ;;
                ok ! $io->eof;

        #        $io->seek(-4, 2);
        #
        #        ok ! $io->eof;
        #
        #        ok read($io, $buf, 20) == 4 ;
        #        ok $buf eq "e.\n\n";
        #
        #        ok read($io, $buf, 20) == 0 ;
        #        ok $buf eq "";
        #
        #        ok ! $io->eof;
            }

        }

        {
            # Read from non-compressed file

            my $str = <<EOT;
This is an example
of a paragraph


and a single line.

EOT

            my $lex = LexFile->new( my $name );

            writeFile($name, $str);
            my @tmp;
            my $buf;
            {
                my $io = $UncompressClass->can('new')->( $UncompressClass, $name, -Transparent => 1 );

                ok defined $io;
                ok ! $io->eof;
                ok $io->tell() == 0 ;
                my @lines = <$io>;
                ok @lines == 6;
                ok $lines[1] eq "of a paragraph\n" ;
                ok join('', @lines) eq $str ;
                ok $. == 6;
                ok $io->tell() == length($str) ;

                ok $io->eof;

                ok ! ( defined($io->getline)  ||
                          (@tmp = $io->getlines) ||
                          defined(<$io>)         ||
                          defined($io->getc)     ||
                          read($io, $buf, 100)   != 0) ;
            }


            {
                local $/;  # slurp mode
                my $io = $UncompressClass->new($name);
                ok ! $io->eof;
                my @lines = $io->getlines;
                ok $io->eof;
                ok @lines == 1 && $lines[0] eq $str;

                $io = $UncompressClass->new($name);
                ok ! $io->eof;
                my $line = <$io>;
                ok $line eq $str;
                ok $io->eof;
            }

            {
                local $/ = "";  # paragraph mode
                my $io = $UncompressClass->new($name);
                ok ! $io->eof;
                my @lines = <$io>;
                ok $io->eof;
                ok @lines == 2
                    or print "# expected 2 lines, got " . scalar(@lines) . "\n";
                ok $lines[0] eq "This is an example\nof a paragraph\n\n\n"
                    or print "# [$lines[0]]\n" ;
                ok $lines[1] eq "and a single line.\n\n";
            }

            {
                local $/ = "is";
                my $io = $UncompressClass->new($name);
                my @lines = ();
                my $no = 0;
                my $err = 0;
                ok ! $io->eof;
                while (<$io>) {
                    push(@lines, $_);
                    $err++ if $. != ++$no;
                }

                ok $err == 0 ;
                ok $io->eof;

                ok @lines == 3 ;
                ok join("-", @lines) eq
                                 "This- is- an example\n" .
                                "of a paragraph\n\n\n" .
                                "and a single line.\n\n";
            }


            # Test read

            {
                my $io = $UncompressClass->new($name);

                ok read($io, $buf, 3) == 3 ;
                ok $buf eq "Thi";

                ok sysread($io, $buf, 3, 2) == 3 ;
                ok $buf eq "Ths i";
                ok ! $io->eof;

        #        $io->seek(-4, 2);
        #
        #        ok ! $io->eof;
        #
        #        ok read($io, $buf, 20) == 4 ;
        #        ok $buf eq "e.\n\n";
        #
        #        ok read($io, $buf, 20) == 0 ;
        #        ok $buf eq "";
        #
        #        ok ! $io->eof;
            }


        }

        {
            # Vary the length parameter in a read

            my $str = <<EOT;
x
x
This is an example
of a paragraph


and a single line.

EOT
            $str = $str x 100 ;


            foreach my $bufsize (1, 3, 512, 4096, length($str)-1, length($str), length($str)+1)
            {
                foreach my $trans (0, 1)
                {
                    foreach my $append (0, 1)
                    {
                        title "Read Tests - buf length $bufsize, Transparent $trans, Append $append" ;

                        my $lex = LexFile->new( my $name );

                        if ($trans) {
                            writeFile($name, $str) ;
                        }
                        else {
                            my $iow = $CompressClass->can('new')->( $CompressClass, $name );
                            print $iow $str ;
                            close $iow;
                        }


                        my $io = $UncompressClass->new($name,
                                                       -Append => $append,
                                                       -Transparent  => $trans);

                        my $buf;

                        is $io->tell(), 0;

                        if ($append) {
                            1 while $io->read($buf, $bufsize) > 0;
                        }
                        else {
                            my $tmp ;
                            $buf .= $tmp while $io->read($tmp, $bufsize) > 0 ;
                        }
                        is length $buf, length $str;
                        ok $buf eq $str ;
                        ok ! $io->error() ;
                        ok $io->eof;
                    }
                }
            }
        }

    }
}

1;
