#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests =>  80;

my ($OUT, $filename, @chunks, @expected, $msg);

{
    # We start with files whose "paragraphs" contain no internal newlines.
    @chunks = (
        join('' => ( 1..3 )),
        join('' => ( 4..6 )),
        join('' => ( 7..9 )),
        10
    );

    {
        $msg = "'Well behaved' file: >= 2 newlines between text blocks; no internal newlines; 3 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
        );
        print $OUT $chunks[3];
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            $chunks[3],
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Well behaved' file: >= 2 newlines between text blocks; no internal newlines; 0 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Well behaved' file: >= 2 newlines between text blocks; no internal newlines; 1 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 1,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Well behaved' file: >= 2 newlines between text blocks; no internal newlines; 2 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 2,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }
}

{
    # We continue with files whose "paragraphs" contain internal newlines.
    @chunks = (
        join('' => ( 1, 2, "\n", 3 )),
        join('' => ( 4, 5, "   \n", 6 )),
        join('' => ( 7, 8, " \t\n", 9 )),
        10
    );

    {
        $msg = "'Misbehaving' file: >= 2 newlines between text blocks; no internal newlines; 3 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
        );
        print $OUT $chunks[3];
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            $chunks[3],
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Misbehaving' file: >= 2 newlines between text blocks; no internal newlines; 0 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Misbehaving' file: >= 2 newlines between text blocks; no internal newlines; 1 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 1,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Misbehaving' file: >= 2 newlines between text blocks; no internal newlines; 2 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 2,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }
}

{
    # We continue with files which start with newlines
    # but whose "paragraphs" contain no internal newlines.
    # We'll set our expectation that the leading newlines will get trimmed off
    # and everything else will proceed normally.

    @chunks = (
        join('' => ( 1..3 )),
        join('' => ( 4..6 )),
        join('' => ( 7..9 )),
        10
    );

    {
        $msg = "'Badly behaved' file: leading newlines; 3 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
        );
        print $OUT $chunks[3];
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            $chunks[3],
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Badly behaved' file: leading newlines; 0 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Badly behaved' file: leading newlines; 1 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 1,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Badly behaved' file: leading newlines; 2 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 2,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }
}

{
    # We continue with files which start with newlines
    # and whose "paragraphs" contain internal newlines.
    # We'll set our expectation that the leading newlines will get trimmed off
    # and everything else will proceed normally.

    @chunks = (
        join('' => ( 1, 2, "\n", 3 )),
        join('' => ( 4, 5, "   \n", 6 )),
        join('' => ( 7, 8, " \t\n", 9 )),
        10
    );

    {
        $msg = "'Very badly behaved' file: leading newlines; internal newlines; 3 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
        );
        print $OUT $chunks[3];
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            $chunks[3],
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Very badly behaved' file: leading newlines; internal newlines; 0 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Very badly behaved' file: leading newlines; internal newlines; 1 final newline";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 1,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }

    {
        $msg = "'Very badly behaved' file: leading newlines; internal newlines; 2 final newlines";

        ($OUT, $filename) = open_tempfile();
        print $OUT "\n\n\n";
        print $OUT "$_\n" for (
            $chunks[0],
            ("") x 1,
            $chunks[1],
            ("") x 2,
            $chunks[2],
            ("") x 3,
            $chunks[3],
            ("") x 2,
        );
        close $OUT or die;

        @expected = (
            "$chunks[0]\n\n",
            "$chunks[1]\n\n",
            "$chunks[2]\n\n",
            "$chunks[3]\n\n",
        );
        local $/ = '';
        perform_tests($filename, \@expected, $msg);
    }
}

########## SUBROUTINES ##########

sub open_tempfile {
    my $filename = tempfile();
    open my $OUT, '>', $filename or die;
    binmode $OUT;
    return ($OUT, $filename);
}

sub perform_tests {
    my ($filename, $expected, $msg) = @_;
    open my $IN, '<', $filename or die;
    my @got = <$IN>;
    my $success = 1;
    for (my $i=0; $i<=$#${expected}; $i++) {
        if ($got[$i] ne $expected->[$i]) {
            $success = 0;
            last;
        }
    }
    ok($success, $msg);

    seek $IN, 0, 0;
    for (my $i=0; $i<=$#${expected}; $i++) {
        is(<$IN>, $expected->[$i], "Got expected record $i");
    }
    close $IN or die;
}
