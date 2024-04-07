
use lib 't';
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

    plan tests => 1828 + $extra ;

    use_ok('IO::Uncompress::AnyUncompress', qw($AnyUncompressError)) ;

}

sub run
{

    my $CompressClass   = identify();
    my $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);




    my @buffers ;
    push @buffers, <<EOM ;
hello world
this is a test
some more stuff on this line
ad finally...
EOM

    push @buffers, <<EOM ;
some more stuff
line 2
EOM

    push @buffers, <<EOM ;
even more stuff
EOM

    my $b0length = length $buffers[0];
    my $bufcount = @buffers;

    {
        my $cc ;
        my $gz ;
        my $hsize ;
        my %headers = () ;


        foreach my $fb ( qw( file filehandle buffer ) )
        {

            foreach my $i (1 .. @buffers) {

                title "Testing $CompressClass with $i streams to $fb";

                my @buffs = @buffers[0..$i -1] ;

                if ($CompressClass eq 'IO::Compress::Gzip') {
                    %headers = (
                                  Strict     => 1,
                                  Comment    => "this is a comment",
                                  ExtraField => ["so" => "me extra"],
                                  HeaderCRC  => 1);

                }

                my $lex = LexFile->new( my $name );
                my $output ;
                if ($fb eq 'buffer')
                {
                    my $compressed = '';
                    $output = \$compressed;
                }
                elsif ($fb eq 'filehandle')
                {
                    $output = IO::File->new( ">$name" );
                }
                else
                {
                    $output = $name ;
                }

                my $x = $CompressClass->can('new')->($CompressClass, $output, AutoClose => 1, %headers);
                isa_ok $x, $CompressClass, '  $x' ;

                foreach my $buffer (@buffs) {
                    ok $x->write($buffer), "    Write OK" ;
                    # this will add an extra "empty" stream
                    ok $x->newStream(), "    newStream OK" ;
                }
                ok $x->close, "  Close ok" ;

                foreach my $unc ($UncompressClass, 'IO::Uncompress::AnyUncompress') {
                    title "  Testing $CompressClass with $unc and $i streams, from $fb";
                    $cc = $output ;
                    if ($fb eq 'filehandle')
                    {
                        $cc = IO::File->new( "<$name" );
                    }
                    my @opts = $unc ne $UncompressClass
                                    ? (RawInflate => 1)
                                    : ();
                    my $gz = $unc->can('new')->($unc, $cc,
                                   @opts,
                                   Strict      => 1,
                                   AutoClose   => 1,
                                   Append      => 1,
                                   MultiStream => 1,
                                   Transparent => 0)
                        or diag $$UnError;
                    isa_ok $gz, $UncompressClass, '    $gz' ;

                    my $un = '';
                    1 while $gz->read($un) > 0 ;
                    #print "[[$un]]\n" while $gz->read($un) > 0 ;
                    ok ! $gz->error(), "      ! error()"
                        or diag "Error is " . $gz->error() ;
                    ok $gz->eof(), "      eof()";
                    ok $gz->close(), "    close() ok"
                        or diag "errno $!\n" ;

                    is $gz->streamCount(), $i +1, "    streamCount ok " .  ($i +1)
                        or diag "Stream count is " . $gz->streamCount();
                    ok $un eq join('', @buffs), "    expected output" ;

                }

                foreach my $unc ($UncompressClass, 'IO::Uncompress::AnyUncompress') {
                  foreach my $blk (1, 20, $b0length - 1, $b0length, $b0length +1) {
                    title "  Testing $CompressClass with $unc, BlockSize $blk and $i streams, from $fb";
                    $cc = $output ;
                    if ($fb eq 'filehandle')
                    {
                        $cc = IO::File->new( "<$name" );
                    }
                    my @opts = $unc ne $UncompressClass
                                    ? (RawInflate => 1)
                                    : ();
                    my $gz = $unc->can('new')->( $unc, $cc,
                                   @opts,
                                   Strict      => 1,
                                   AutoClose   => 1,
                                   Append      => 1,
                                   MultiStream => 1,
                                   Transparent => 0)
                        or diag $$UnError;
                    isa_ok $gz, $UncompressClass, '    $gz' ;

                    my $un = '';
                    my $b = $blk;
                    # Want the first read to be in the middle of a stream
                    # and the second to cross a stream boundary
                    $b = 1000 while $gz->read($un, $b) > 0 ;
                    #print "[[$un]]\n" while $gz->read($un) > 0 ;
                    ok ! $gz->error(), "      ! error()"
                        or diag "Error is " . $gz->error() ;
                    ok $gz->eof(), "      eof()";
                    ok $gz->close(), "    close() ok"
                        or diag "errno $!\n" ;

                    is $gz->streamCount(), $i +1, "    streamCount ok " .  ($i +1)
                        or diag "Stream count is " . $gz->streamCount();
                    ok $un eq join('', @buffs), "    expected output" ;

                  }
                }

                foreach my $unc ($UncompressClass, 'IO::Uncompress::AnyUncompress') {

                foreach my $trans (0, 1) {
                    title "  Testing $CompressClass with $unc nextStream and $i streams, from $fb, Transparent => $trans";
                    $cc = $output ;
                    if ($fb eq 'filehandle')
                    {
                        $cc = IO::File->new( "<$name" );
                    }
                    my @opts = $unc ne $UncompressClass
                                    ? (RawInflate => 1)
                                    : ();
                    my $gz = $unc->can('new')->( $unc, $cc,
                                   @opts,
                                   Strict      => 1,
                                   AutoClose   => 1,
                                   Append      => 1,
                                   MultiStream => 0,
                                   Transparent => $trans)
                        or diag $$UnError;
                    isa_ok $gz, $UncompressClass, '    $gz' ;

                    for my $stream (1 .. $i)
                    {
                        my $buff = $buffs[$stream-1];
                        my @lines = split("\n", $buff);
                        my $lines = @lines;

                        my $un = '';
                        #while (<$gz>) {
                        while ($_ = $gz->getline()) {
                            $un .= $_;
                        }
                        is $., $lines, "    \$. is $lines";

                        ok ! $gz->error(), "      ! error()"
                            or diag "Error is " . $gz->error() ;
                        ok $gz->eof(), "      eof()";
                        is $gz->streamCount(), $stream, "    streamCount is $stream"
                            or diag "Stream count is " . $gz->streamCount();
                        is $un, $buff, "    expected output"
                            or diag "Stream count is " . $gz->streamCount();                        ;
                        #is $gz->tell(), length $buff, "    tell is ok";
                        is $gz->nextStream(), 1, "    nextStream ok";
                        is $gz->tell(), 0, "    tell is 0";
                        is $., 0, '    $. is 0';
                    }

                    {
                        my $un = '';
                        #1 while $gz->read($un) > 0 ;
                        is $., 0, "    \$. is 0";
                        $gz->read($un) ;
                        #print "[[$un]]\n" while $gz->read($un) > 0 ;
                        ok ! $gz->error(), "      ! error()"
                            or diag "Error is " . $gz->error() ;
                        ok $gz->eof(), "      eof()";
                        is $gz->streamCount(), $i+1, "    streamCount is ok"
                            or diag "Stream count is " . $gz->streamCount();
                        ok $un eq "", "    expected output" ;
                        is $gz->tell(), 0, "    tell is 0";
                    }

                    is $gz->nextStream(), 0, "    nextStream ok"
                        or diag $gz->error() ;
                    ok $gz->eof(), "      eof()";
                    ok $gz->close(), "    close() ok"
                        or diag "errno $!\n" ;

                    is $gz->streamCount(), $i +1, "    streamCount ok"
                        or diag "Stream count is " . $gz->streamCount();

                }
              }
            }
        }
    }
}


# corrupt one of the streams - all previous should be ok
# trailing stuff
# check that "tell" works ok

1;
