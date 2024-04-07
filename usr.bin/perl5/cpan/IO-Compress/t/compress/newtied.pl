use lib 't';
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

our ($BadPerl, $UncompressClass);

BEGIN
{
    plan(skip_all => "Extra Tied Filehandle needs Perl 5.6 or better - you have Perl $]" )
        if $] < 5.006 ;

    my $tests ;

    $BadPerl = ($] >= 5.006 and $] <= 5.008) ;

    if ($BadPerl) {
        $tests = 78 ;
    }
    else {
        $tests = 84 ;
    }

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

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
        title "Testing $CompressClass and $UncompressClass";



        {
            # Write
            # these tests come almost 100% from IO::String

            my $lex = LexFile->new( my $name );

            my $io = $CompressClass->new($name);

            is tell($io), 0 ;
            is $io->tell(), 0 ;

            my $heisan = "Heisan\n";
            print $io $heisan ;

            ok ! eof($io);
            ok ! $io->eof();

            is tell($io), length($heisan) ;
            is $io->tell(), length($heisan) ;

            $io->print("a", "b", "c");

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

            ok eof($io);
            ok $io->eof();

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

                ok ! $io->eof;
                ok ! eof $io;
                is $io->tell(), 0 ;
                is tell($io), 0 ;
                my @lines = <$io>;
                is @lines, 6
                    or print "# Got " . scalar(@lines) . " lines, expected 6\n" ;
                is $lines[1], "of a paragraph\n" ;
                is join('', @lines), $str ;
                is $., 6;
        #print "TELL says " . tell($io) , " should be ${ \length($str) }\n" ;
                is $io->tell(), length($str) ;
                is tell($io), length($str) ;

                ok $io->eof;
                ok eof $io;

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

                ok $io, "opened ok" ;

                #eval { read($io, $buf, -1); } ;
                #like $@, mkErr("length parameter is negative"), "xxx $io $UncompressClass $RawInflateError" ;

                #eval { read($io, 1) } ;
                #like $@, mkErr("buffer parameter is read-only");

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
            title "seek tests" ;

            my $lex = LexFile->new( my $name );

            my $first = "beginning" ;
            my $last  = "the end" ;
            my $iow = $CompressClass->can('new')->( $CompressClass, $name );
            print $iow $first ;
            ok seek $iow, 10, SEEK_CUR ;
            is tell($iow), length($first)+10;
            ok $iow->seek(0, SEEK_CUR) ;
            is tell($iow), length($first)+10;
            print $iow $last ;
            close $iow;

            my $io = $UncompressClass->new($name);
            ok myGZreadFile($name) eq $first . "\x00" x 10 . $last ;

            $io = $UncompressClass->new($name);
            ok seek $io, length($first)+10, SEEK_CUR ;
            ok ! $io->eof;
            is tell($io), length($first)+10;
            ok seek $io, 0, SEEK_CUR ;
            is tell($io), length($first)+10;
            my $buff ;
            ok read $io, $buff, 100 ;
            ok $buff eq $last ;
            ok $io->eof;
        }

        if (! $BadPerl)
        {
            # seek error cases
            my $b ;
            my $a = $CompressClass->can('new')->( $CompressClass, \$b)  ;

            ok ! $a->error() ;
            eval { seek($a, -1, 10) ; };
            like $@, mkErr("seek: unknown value, 10, for whence parameter");

            eval { seek($a, -1, SEEK_END) ; };
            like $@, mkErr("cannot seek backwards");

            print $a "fred";
            close $a ;


            my $u = $UncompressClass->can('new')->( $UncompressClass, \$b)  ;

            eval { seek($u, -1, 10) ; };
            like $@, mkErr("seek: unknown value, 10, for whence parameter");

            eval { seek($u, -1, SEEK_END) ; };
            like $@, mkErr("seek: SEEK_END not allowed");

            eval { seek($u, -1, SEEK_CUR) ; };
            like $@, mkErr("cannot seek backwards");
        }

        {
            title 'fileno' ;

            my $lex = LexFile->new( my $name );

            my $hello = <<EOM ;
hello world
this is a test
EOM

            {
              my $fh ;
              ok $fh = IO::File->new( ">$name" );
              my $x ;
              ok $x = $CompressClass->can('new')->( $CompressClass, $fh );

              ok $x->fileno() == fileno($fh) ;
              ok $x->fileno() == fileno($x) ;
              ok $x->write($hello) ;
              ok $x->close ;
              $fh->close() ;
            }

            my $uncomp;
            {
              my $x ;
              ok my $fh1 = IO::File->new( "<$name" );
              ok $x = $UncompressClass->can('new')->( $UncompressClass, $fh1, -Append => 1 );
              ok $x->fileno() == fileno $fh1 ;
              ok $x->fileno() == fileno $x ;

              1 while $x->read($uncomp) > 0 ;

              ok $x->close ;
            }

            ok $hello eq $uncomp ;
        }
    }
}

1;
