# tests for heredocs besides what is tested in base/lex.t

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
plan(tests => 138);

# heredoc without newline (#65838)
{
    my $string = <<'HEREDOC';
testing for 65838
HEREDOC

    my $code = "<<'HEREDOC';\n${string}HEREDOC";  # HD w/o newline, in eval-string
    my $hd = eval $code or warn "$@ ---";
    is($hd, $string, "no terminating newline in string-eval");
}


# here-doc edge cases
{
    my $string = "testing for 65838";

    fresh_perl_is(
        "print <<'HEREDOC';\n${string}\nHEREDOC",
        $string,
        {},
        "heredoc at EOF without trailing newline"
    );

    fresh_perl_is(
        qq(print <<"";\n$string\n),
        $string,
        { switches => ['-X'] },
        "blank-terminated heredoc at EOF"
    );
    fresh_perl_is(
        qq(print <<""\n$string\n),
        $string,
        { switches => ['-X'] },
        "blank-terminated heredoc at EOF and no semicolon"
    );
    fresh_perl_is(
        "print <<foo\r\nick and queasy\r\nfoo\r\n",
        'ick and queasy',
        { switches => ['-X'] },
        "crlf-terminated heredoc"
    );
    fresh_perl_is(
        "print qq|\${\\<<foo}|\nick and queasy\nfoo\n",
        'ick and queasy',
        { switches => ['-w'], stderr => 1 },
        'no warning for qq|${\<<foo}| in file'
    );
}


# here-doc parse failures
{
    fresh_perl_like(
        "print <<HEREDOC;\nwibble\n HEREDOC",
        qr/find string terminator/,
        {},
        "string terminator must start at newline"
    );

    # Loop over various lengths to try to force at least one to cause a
    # reallocation in S_scan_heredoc()
    # Timing on a modern machine suggests that this loop executes in less than
    # 0.1s, so it's a very small cost for the default build. The benefit is
    # that building with ASAN will reveal the bug and any related regressions.
    for (1..31) {
        fresh_perl_like(
            qq(print <<"";\n) . "x" x $_,
            qr/find string terminator/,
            { switches => ['-X'] },
            "empty string terminator still needs a newline (length $_)"
        );
    }

    fresh_perl_like(
        "print <<ThisTerminatorIsLongerThanTheData;\nno more newlines",
        qr/find string terminator/,
        {},
        "long terminator fails correctly"
    );

    # this would read freed memory
    fresh_perl_like(
        qq(0<<<<""0\n\n),
        # valgrind and asan reports an error between these two lines
        qr/^Number found where operator expected \(Missing operator before "0"\?\) at - line 1, near "<<""0"/,
        {},
        "don't use an invalid oldoldbufptr"
    );

    # also read freed memory, but got an invalid oldoldbufptr in a different way
    fresh_perl_like(
        qq(<<""\n\$          \n),
        # valgrind and asan reports an error between these two lines
        qr/^Final \$/,
        {},
        "don't use an invalid oldoldbufptr (some more)"
    );

    # [perl #125540] this asserted or crashed
    fresh_perl_like(
	q(map d<<<<""),
	qr/Can't find string terminator "" anywhere before EOF at - line 1\./,
	{},
	"Don't assert parsing a here-doc if we hit EOF early"
    );

    # [perl #129064] heap-buffer-overflow S_scan_heredoc
    fresh_perl_like(
        qq(<<`\\),
        # valgrind and asan reports an error between these two lines
        qr/^Unterminated delimiter for here document/,
        {},
        "delimcpy(): handle last char being backslash properly"
    );
}


# indented here-docs
{
    my $string = 'some data';

    my %delimiters = (
        q{EOF}     => "EOF",
        q{'EOF'}   => "EOF",
        q{"EOF"}   => "EOF",
        q{\EOF}    => "EOF",
        q{' EOF'}  => " EOF",
        q{'EOF '}  => "EOF ",
        q{' EOF '} => " EOF ",
        q{" EOF"}  => " EOF",
        q{"EOF "}  => "EOF ",
        q{" EOF "} => " EOF ",
        q{''}      => "",
        q{""}      => "",
    );

    my @modifiers = ("~", "~ ");

    my @script_ends = ("", "\n");

    my @tests;

    for my $start_delim (sort keys %delimiters) {
        my $end_delim = $delimiters{$start_delim};

        for my $modifier (@modifiers) {
            # For now, "<<~ EOF" and "<<~ \EOF" aren't allowed
            next if $modifier =~ /\s/ && $start_delim !~ /('|")/n;

            for my $script_end (@script_ends) {
                # Normal heredoc
                my $test =   "print <<$modifier$start_delim\n  $string\n"
                           . "  $end_delim$script_end";
                unshift @tests, [
                    $test,
                    $string,
                    "Indented here-doc: <<$modifier$start_delim with end delim '$end_delim'" . ($script_end ? "\\n" : ""),
                ];

                # Eval'd heredoc
                my $safe_start_delim = $start_delim =~ s/'/\\'/gr;
                my $eval = "
                    \$_ = '';
                    eval 's//<<$modifier$safe_start_delim.\"\"/e; print
                        $string
                        $end_delim$script_end'
                    or die \$\@
                ";
                push @tests, [
                    $eval,
                    $string,
                    "Eval'd Indented here-doc: <<$modifier$start_delim with end delim '$end_delim'" . ($script_end ? "\\n" : ""),

                ];
            }
        }
    }

    push @tests, [
        "print <<~EOF;\n\t \t$string\n\t \tEOF\n",
        $string,
        "indented here-doc with tabs and spaces",
    ];

    push @tests, [
        "print <<~EOF;\n\t \tx EOF\n\t \t$string\n\t \tEOF\n",
         "x EOF\n$string",
        "Embedded delimiter ignored",
    ];

    push @tests, [
        "print <<~EOF;\n\t \t$string\n\t \tTEOF",
        "Can't find string terminator \"EOF\" anywhere before EOF at - line 1.",
        "indented here-doc missing terminator error is correct"
    ];

    push @tests, [
        "print <<~EOF;\n $string\n$string\n   $string\n $string\n   EOF",
        "Indentation on line 1 of here-doc doesn't match delimiter at - line 1.\n",
        "indented here-doc with bad indentation"
    ];

    push @tests, [
        "print <<~EOF;\n   $string\n   $string\n$string\n $string\n   EOF",
        "Indentation on line 3 of here-doc doesn't match delimiter at - line 1.\n",
        "indented here-doc with bad indentation"
    ];

    # If our delim is " EOF ", make sure other spaced version don't match
    push @tests, [
        "print <<~' EOF ';\n $string\n EOF\nEOF \n  EOF  \n EOF \n",
        " $string\n EOF\nEOF \n  EOF  \n",
        "indented here-doc matches final delimiter correctly"
    ];

    for my $test (@tests) {
        fresh_perl_is(
            $test->[0],
            $test->[1],
            { switches => ['-w'], stderr => 1 },
            $test->[2],
        );
    }
}
fresh_perl_like(
q#<<E1;
${sub{b{]]]{} @{[ <<E2 ]}
E2
E1
#,
    qr/^syntax error/,
    {},
    "GH Issue #17397 - Syntax error inside of here doc causes segfault"
);
