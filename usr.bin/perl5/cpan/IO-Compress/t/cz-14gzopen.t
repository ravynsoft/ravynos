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
use IO::File ;

BEGIN {
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 264 + $extra ;

    use_ok('Compress::Zlib', 2) ;
    use_ok('IO::Compress::Gzip::Constants') ;
}

{
    SKIP: {
        skip "TEST_SKIP_VERSION_CHECK is set", 1
            if $ENV{TEST_SKIP_VERSION_CHECK};
        # Check zlib_version and ZLIB_VERSION are the same.
        is Compress::Zlib::zlib_version, ZLIB_VERSION,
            "ZLIB_VERSION matches Compress::Zlib::zlib_version" ;
    }
}

{
    # gzip tests
    #===========

    #my $name = "test.gz" ;
    my $lex = LexFile->new( my $name );

    my $hello = <<EOM ;
hello world
this is a test
EOM

    my $len   = length $hello ;

    my ($x, $uncomp) ;

    ok my $fil = gzopen($name, "wb") ;

    is $gzerrno, 0, 'gzerrno is 0';
    is $fil->gzerror(), 0, "gzerror() returned 0";

    is $fil->gztell(), 0, "gztell returned 0";
    is $gzerrno, 0, 'gzerrno is 0';

    is $fil->gzwrite($hello), $len ;
    is $gzerrno, 0, 'gzerrno is 0';

    is $fil->gztell(), $len, "gztell returned $len";
    is $gzerrno, 0, 'gzerrno is 0';

    ok ! $fil->gzclose ;

    ok $fil = gzopen($name, "rb") ;

    ok ! $fil->gzeof() ;
    is $gzerrno, 0, 'gzerrno is 0';
    is $fil->gztell(), 0;

    is $fil->gzread($uncomp), $len;

    is $fil->gztell(), $len;
    ok   $fil->gzeof() ;

    # gzread after eof bahavior

    my $xyz = "123" ;
    is $fil->gzread($xyz), 0, "gzread returns 0 on eof" ;
    is $xyz, "", "gzread on eof zaps the output buffer [Match 1,x behavior]" ;

    ok ! $fil->gzclose ;
    ok   $fil->gzeof() ;

    ok $hello eq $uncomp ;
}

{
    title 'check that a number can be gzipped';
    my $lex = LexFile->new( my $name );


    my $number = 7603 ;
    my $num_len = 4 ;

    ok my $fil = gzopen($name, "wb") ;

    is $gzerrno, 0;

    is $fil->gzwrite($number), $num_len, "gzwrite returned $num_len" ;
    is $gzerrno, 0, 'gzerrno is 0';
    ok ! $fil->gzflush(Z_FINISH) ;

    is $gzerrno, 0, 'gzerrno is 0';

    ok ! $fil->gzclose ;

    cmp_ok $gzerrno, '==', 0;

    ok $fil = gzopen($name, "rb") ;

    my $uncomp;
    ok ((my $x = $fil->gzread($uncomp)) == $num_len) ;

    ok $fil->gzerror() == 0 || $fil->gzerror() == Z_STREAM_END;
    ok $gzerrno == 0 || $gzerrno == Z_STREAM_END;
    ok   $fil->gzeof() ;

    ok ! $fil->gzclose ;
    ok   $fil->gzeof() ;

    ok $gzerrno == 0
        or print "# gzerrno is $gzerrno\n" ;

    1 while unlink $name ;

    ok $number == $uncomp ;
    ok $number eq $uncomp ;
}

{
    title "now a bigger gzip test";

    my $text = 'text' ;
    my $lex = LexFile->new( my $file );


    ok my $f = gzopen($file, "wb") ;

    # generate a long random string
    my $contents = '' ;
    foreach (1 .. 5000)
    { $contents .= chr int rand 256 }

    my $len = length $contents ;

    is $f->gzwrite($contents), $len ;

    ok ! $f->gzclose ;

    ok $f = gzopen($file, "rb") ;

    ok ! $f->gzeof() ;

    my $uncompressed ;
    is $f->gzread($uncompressed, $len), $len ;

    is $contents, $uncompressed

        or print "# Length orig $len" .
                ", Length uncompressed " . length($uncompressed) . "\n" ;

    ok $f->gzeof() ;
    ok ! $f->gzclose ;

}

{
    title "gzip - readline tests";
    # ======================

    # first create a small gzipped text file
    my $lex = LexFile->new( my $name );

    my @text = (<<EOM, <<EOM, <<EOM, <<EOM) ;
this is line 1
EOM
the second line
EOM
the line after the previous line
EOM
the final line
EOM

    my $text = join("", @text) ;

    ok my $fil = gzopen($name, "wb") ;
    is $fil->gzwrite($text), length($text) ;
    ok ! $fil->gzclose ;

    # now try to read it back in
    ok $fil = gzopen($name, "rb") ;
    ok ! $fil->gzeof() ;
    my $line = '';
    for my $i (0 .. @text -2)
    {
        ok $fil->gzreadline($line) > 0;
        is $line, $text[$i] ;
        ok ! $fil->gzeof() ;
    }

    # now read the last line
    ok $fil->gzreadline($line) > 0;
    is $line, $text[-1] ;
    ok $fil->gzeof() ;

    # read past the eof
    is $fil->gzreadline($line), 0;

    ok   $fil->gzeof() ;
    ok ! $fil->gzclose ;
    ok   $fil->gzeof() ;
}

{
    title "A text file with a very long line (bigger than the internal buffer)";
    my $lex = LexFile->new( my $name );

    my $line1 = ("abcdefghijklmnopq" x 2000) . "\n" ;
    my $line2 = "second line\n" ;
    my $text = $line1 . $line2 ;
    ok my $fil = gzopen($name, "wb"), " gzopen ok" ;
    is $fil->gzwrite($text), length $text, "  gzwrite ok" ;
    ok ! $fil->gzclose, "  gzclose" ;

    # now try to read it back in
    ok $fil = gzopen($name, "rb"), "  gzopen" ;
    ok ! $fil->gzeof(), "! eof" ;
    my $i = 0 ;
    my @got = ();
    my $line;
    while ($fil->gzreadline($line) > 0) {
        $got[$i] = $line ;
        ++ $i ;
    }
    is $i, 2, "  looped twice" ;
    is $got[0], $line1, "  got line 1" ;
    is $got[1], $line2, "  hot line 2" ;

    ok   $fil->gzeof(), "  gzeof" ;
    ok ! $fil->gzclose, "  gzclose" ;
    ok   $fil->gzeof(), "  gzeof" ;
}

{
    title "a text file which is not terminated by an EOL";

    my $lex = LexFile->new( my $name );

    my $line1 = "hello hello, I'm back again\n" ;
    my $line2 = "there is no end in sight" ;

    my $text = $line1 . $line2 ;
    ok my $fil = gzopen($name, "wb"), "  gzopen" ;
    is $fil->gzwrite($text), length $text, "  gzwrite" ;
    ok ! $fil->gzclose, "  gzclose" ;

    # now try to read it back in
    ok $fil = gzopen($name, "rb"), "  gzopen" ;
    my @got = () ;
    my $i = 0 ;
    my $line;
    while ($fil->gzreadline($line) > 0) {
        $got[$i] = $line ;
        ++ $i ;
    }
    is $i, 2, "  got 2 lines" ;
    is $got[0], $line1, "  line 1 ok" ;
    is $got[1], $line2, "  line 2 ok" ;

    ok   $fil->gzeof(), "  gzeof" ;
    ok ! $fil->gzclose, "  gzclose" ;
}

{

    title 'mix gzread and gzreadline';

    # case 1: read a line, then a block. The block is
    #         smaller than the internal block used by
    #	  gzreadline
    my $lex = LexFile->new( my $name );
    my $line1 = "hello hello, I'm back again\n" ;
    my $line2 = "abc" x 200 ;
    my $line3 = "def" x 200 ;
    my $line;

    my $text = $line1 . $line2 . $line3 ;
    my $fil;
    ok $fil = gzopen($name, "wb"), ' gzopen for write ok' ;
    is $fil->gzwrite($text), length $text, '    gzwrite ok' ;
    is $fil->gztell(), length $text, '    gztell ok' ;
    ok ! $fil->gzclose, '  gzclose ok' ;

    # now try to read it back in
    ok $fil = gzopen($name, "rb"), '  gzopen for read ok' ;
    ok ! $fil->gzeof(), '    !gzeof' ;
    cmp_ok $fil->gzreadline($line), '>', 0, '    gzreadline' ;
    is $fil->gztell(), length $line1, '    gztell ok' ;
    ok ! $fil->gzeof(), '    !gzeof' ;
    is $line, $line1, '    got expected line' ;
    cmp_ok $fil->gzread($line, length $line2), '>', 0, '    gzread ok' ;
    is $fil->gztell(), length($line1)+length($line2), '    gztell ok' ;
    ok ! $fil->gzeof(), '    !gzeof' ;
    is $line, $line2, '    read expected block' ;
    cmp_ok $fil->gzread($line, length $line3), '>', 0, '    gzread ok' ;
    is $fil->gztell(), length($text), '    gztell ok' ;
    ok   $fil->gzeof(), '    !gzeof' ;
    is $line, $line3, '    read expected block' ;
    ok ! $fil->gzclose, '  gzclose'  ;
}

{
    title "Pass gzopen a filehandle - use IO::File" ;

    my $lex = LexFile->new( my $name );

    my $hello = "hello" ;
    my $len = length $hello ;

    my $f = IO::File->new( ">$name" );
    ok $f;

    my $fil;
    ok $fil = gzopen($f, "wb") ;

    ok $fil->gzwrite($hello) == $len ;

    ok ! $fil->gzclose ;

    $f = IO::File->new( "<$name" );
    ok $fil = gzopen($name, "rb") ;

    my $uncomp; my $x;
    ok (($x = $fil->gzread($uncomp)) == $len)
        or print "# length $x, expected $len\n" ;

    ok   $fil->gzeof() ;
    ok ! $fil->gzclose ;
    ok   $fil->gzeof() ;

    is $uncomp, $hello, "got expected output" ;
}


{
    title "Pass gzopen a filehandle - use open" ;

    my $lex = LexFile->new( my $name );

    my $hello = "hello" ;
    my $len = length $hello ;

    open F, ">$name" ;

    my $fil;
    ok $fil = gzopen(*F, "wb") ;

    is $fil->gzwrite($hello), $len ;

    ok ! $fil->gzclose ;

    open F, "<$name" ;
    ok $fil = gzopen(*F, "rb") ;

    my $uncomp; my $x;
    $x = $fil->gzread($uncomp);
    is $x, $len ;

    ok   $fil->gzeof() ;
    ok ! $fil->gzclose ;
    ok   $fil->gzeof() ;

    is $uncomp, $hello ;


}

foreach my $stdio ( ['-', '-'], [*STDIN, *STDOUT])
{
    my $stdin = $stdio->[0];
    my $stdout = $stdio->[1];

    title "Pass gzopen a filehandle - use $stdin" ;

    my $lex = LexFile->new( my $name );

    my $hello = "hello" ;
    my $len = length $hello ;

    ok open(SAVEOUT, ">&STDOUT"), "  save STDOUT";
    my $dummy = fileno SAVEOUT;
    ok open(STDOUT, ">$name"), "  redirect STDOUT" ;

    my $status = 0 ;

    my $fil = gzopen($stdout, "wb") ;

    $status = $fil &&
              ($fil->gzwrite($hello) == $len) &&
              ($fil->gzclose == 0) ;

    open(STDOUT, ">&SAVEOUT");

    ok $status, "  wrote to stdout";

       open(SAVEIN, "<&STDIN");
    ok open(STDIN, "<$name"), "  redirect STDIN";
    $dummy = fileno SAVEIN;

    ok $fil = gzopen($stdin, "rb") ;

    my $uncomp; my $x;
    ok (($x = $fil->gzread($uncomp)) == $len)
        or print "# length $x, expected $len\n" ;

    ok   $fil->gzeof() ;
    ok ! $fil->gzclose ;
    ok   $fil->gzeof() ;

       open(STDIN, "<&SAVEIN");

    is $uncomp, $hello ;


}

{
    title 'test parameters for gzopen';
    my $lex = LexFile->new( my $name );

    my $fil;

    # missing parameters
    eval ' $fil = gzopen()  ' ;
    like $@, mkEvalErr('Not enough arguments .*? Compress::Zlib::gzopen'),
        '  gzopen with missing mode fails' ;

    # unknown parameters
    $fil = gzopen($name, "xy") ;
    ok ! defined $fil, '  gzopen with unknown mode fails' ;

    $fil = gzopen($name, "ab") ;
    ok $fil, '  gzopen with mode "ab" is ok' ;

    $fil = gzopen($name, "wb6") ;
    ok $fil, '  gzopen with mode "wb6" is ok' ;

    $fil = gzopen($name, "wbf") ;
    ok $fil, '  gzopen with mode "wbf" is ok' ;

    $fil = gzopen($name, "wbh") ;
    ok $fil, '  gzopen with mode "wbh" is ok' ;
}

{
    title 'Read operations when opened for writing';

    my $lex = LexFile->new( my $name );
    my $fil;
    ok $fil = gzopen($name, "wb"), '  gzopen for writing' ;
    ok !$fil->gzeof(), '    !eof'; ;
    is $fil->gzread(), Z_STREAM_ERROR, "    gzread returns Z_STREAM_ERROR" ;
    ok ! $fil->gzclose, "  gzclose ok" ;
}

{
    title 'write operations when opened for reading';

    my $lex = LexFile->new( my $name );
    my $text = "hello" ;
    my $fil;
    ok $fil = gzopen($name, "wb"), "  gzopen for writing" ;
    is $fil->gzwrite($text), length $text, "    gzwrite ok" ;
    ok ! $fil->gzclose, "  gzclose ok" ;

    ok $fil = gzopen($name, "rb"), "  gzopen for reading" ;
    is $fil->gzwrite(), Z_STREAM_ERROR, "  gzwrite returns Z_STREAM_ERROR" ;
}

{
    title 'read/write a non-readable/writable file';

    SKIP:
    {
        skip "Cannot create non-writable file", 3
            if $^O eq 'cygwin';

        my $lex = LexFile->new( my $name );
        writeFile($name, "abc");
        chmod 0444, $name
            or skip "Cannot create non-writable file", 3 ;

        skip "Cannot create non-writable file", 3
            if -w $name ;

        ok ! -w $name, "  input file not writable";

        my $fil = gzopen($name, "wb") ;
        ok !$fil, "  gzopen returns undef" ;
        ok $gzerrno, "  gzerrno ok" or
            diag " gzerrno $gzerrno\n";

        chmod 0777, $name ;
    }

    SKIP:
    {
        my $lex = LexFile->new( my $name );
        skip "Cannot create non-readable file", 3
            if $^O eq 'cygwin';

        writeFile($name, "abc");
        chmod 0222, $name ;

        skip "Cannot create non-readable file", 3
            if -r $name ;

        ok ! -r $name, "  input file not readable";
        $gzerrno = 0;
        my $fil = gzopen($name, "rb") ;
        ok !$fil, "  gzopen returns undef" ;
        ok $gzerrno, "  gzerrno ok";
        chmod 0777, $name ;
    }

}

{
    title "gzseek" ;

    my $buff ;
    my $lex = LexFile->new( my $name );

    my $first = "beginning" ;
    my $last  = "the end" ;
    my $iow = gzopen($name, "w");
    $iow->gzwrite($first) ;
    ok $iow->gzseek(5, SEEK_CUR) ;
    is $iow->gztell(), length($first)+5;
    ok $iow->gzseek(0, SEEK_CUR) ;
    is $iow->gztell(), length($first)+5;
    ok $iow->gzseek(length($first)+10, SEEK_SET) ;
    is $iow->gztell(), length($first)+10;

    $iow->gzwrite($last) ;
    $iow->gzclose ;

    ok GZreadFile($name) eq $first . "\x00" x 10 . $last ;

    my $io = gzopen($name, "r");
    ok $io->gzseek(length($first), SEEK_CUR) ;
    ok ! $io->gzeof;
    is $io->gztell(), length($first);

    ok $io->gzread($buff, 5) ;
    is $buff, "\x00" x 5 ;
    is $io->gztell(), length($first) + 5;

    is $io->gzread($buff, 0), 0 ;
    #is $buff, "\x00" x 5 ;
    is $io->gztell(), length($first) + 5;

    ok $io->gzseek(0, SEEK_CUR) ;
    my $here = $io->gztell() ;
    is $here, length($first)+5;

    ok $io->gzseek($here+5, SEEK_SET) ;
    is $io->gztell(), $here+5 ;
    ok $io->gzread($buff, 100) ;
    ok $buff eq $last ;
    ok $io->gzeof;
}

{
    # seek error cases
    my $lex = LexFile->new( my $name );

    my $a = gzopen($name, "w");

    ok ! $a->gzerror()
        or print "# gzerrno is $Compress::Zlib::gzerrno \n" ;
    eval { $a->gzseek(-1, 10) ; };
    like $@, mkErr("gzseek: unknown value, 10, for whence parameter");

    eval { $a->gzseek(-1, SEEK_END) ; };
    like $@, mkErr("gzseek: cannot seek backwards");

    $a->gzwrite("fred");
    $a->gzclose ;


    my $u = gzopen($name, "r");

    eval { $u->gzseek(-1, 10) ; };
    like $@, mkErr("gzseek: unknown value, 10, for whence parameter");

    eval { $u->gzseek(-1, SEEK_END) ; };
    like $@, mkErr("gzseek: SEEK_END not allowed");

    eval { $u->gzseek(-1, SEEK_CUR) ; };
    like $@, mkErr("gzseek: cannot seek backwards");
}

{
    title "gzread ver 1.x compat -- the output buffer is always zapped.";
    my $lex = LexFile->new( my $name );

    my $a = gzopen($name, "w");
    $a->gzwrite("fred");
    $a->gzclose ;

    my $u = gzopen($name, "r");

    my $buf1 ;
    is $u->gzread($buf1, 0), 0, "  gzread returns 0";
    ok defined $buf1, "  output buffer defined";
    is $buf1, "", "  output buffer empty string";

    my $buf2 = "qwerty";
    is $u->gzread($buf2, 0), 0, "  gzread returns 0";
    ok defined $buf2, "  output buffer defined";
    is $buf2, "", "  output buffer empty string";
}

{
    title 'gzreadline does not support $/';

    my $lex = LexFile->new( my $name );

    my $a = gzopen($name, "w");
    my $text = "fred\n";
    my $len = length $text;
    $a->gzwrite($text);
    $a->gzwrite("\n\n");
    $a->gzclose ;

    for my $delim ( undef, "", 0, 1, "abc", $text, "\n\n", "\n" )
    {
        local $/ = $delim;
        my $u = gzopen($name, "r");
        my $line;
        is $u->gzreadline($line), length $text, "  read $len bytes";
        is $line, $text, "  got expected line";
        ok ! $u->gzclose, "  closed" ;
        is $/, $delim, '  $/ unchanged by gzreadline';
    }
}

{
    title 'gzflush called twice with Z_SYNC_FLUSH - no compression';

    my $lex = LexFile->new( my $name );

    ok my $a = gzopen($name, "w");

    is $a->gzflush(Z_SYNC_FLUSH), Z_OK, "gzflush returns Z_OK";
    is $a->gzflush(Z_SYNC_FLUSH), Z_OK, "gzflush returns Z_OK";
}



{
    title 'gzflush called twice - after compression';

    my $lex = LexFile->new( my $name );

    ok my $a = gzopen($name, "w");
    my $text = "fred\n";
    my $len = length $text;
    is $a->gzwrite($text), length($text), "gzwrite ok";

    is $a->gzflush(Z_SYNC_FLUSH), Z_OK, "gzflush returns Z_OK";
    is $a->gzflush(Z_SYNC_FLUSH), Z_OK, "gzflush returns Z_OK";
}
