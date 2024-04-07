#!./perl -i.inplace
# note the extra switch, for the test below

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use Test::More;

use English qw( -no_match_vars ) ;
use Config;
use Errno;

is( $PID, $$, '$PID' );

$_ = 1;
is( $ARG, $_, '$ARG' );

sub foo {
	is($ARG[0], $_[0], '@ARG' );
}
foo(1);

"abc" =~ /b/;

ok( !$PREMATCH, '$PREMATCH undefined' );
ok( !$MATCH, '$MATCH undefined' );
ok( !$POSTMATCH, '$POSTMATCH undefined' );

$OFS = " ";
$ORS = "\n";

{
	local(*IN, *OUT);
        pipe(IN, OUT);
	select(OUT);
	$| = 1;
	print 'ok', '7';

	# since $| is 1, this should be true
	ok( $OUTPUT_AUTOFLUSH, '$OUTPUT_AUTOFLUSH should be true' );

	my $close = close OUT;
	ok( !($close) == $CHILD_ERROR, '$CHILD_ERROR should be false' );

	my $foo = <IN>;
	like( $foo, qr/ok 7/, '$OFS' );

	# chomp is true because $ORS is "\n"
	ok( chomp($foo), '$ORS should be \n' );
}

is( $FORMAT_NAME, 'OUT', '$FORMAT_NAME' );
is( $FORMAT_TOP_NAME, 'OUT_TOP', '$FORMAT_TOP_NAME' );
is( $FORMAT_FORMFEED, "\f", '$FORMAT_FORMFEED' );
is( $FORMAT_LINES_LEFT, 0, '$FORMAT_LINES_LEFT' );
is( $FORMAT_LINES_PER_PAGE, 60, '$FORMAT_LINES_PER_PAGE' );
is( $FORMAT_LINE_BREAK_CHARACTERS, " \n-", '$FORMAT_LINE_BREAK_CHARACTERS');
is( $FORMAT_PAGE_NUMBER, 0, '$FORMAT_PAGE_NUMBER' );
is( $ACCUMULATOR, $^A, '$ACCUMULATOR' );

undef $OUTPUT_FIELD_SEPARATOR;

if ($threads) { $" = "\n" } else { $LIST_SEPARATOR = "\n" };
@foo = (8, 9);
@foo = split(/\n/, "@foo");
is( $foo[0], 8, '$"' );
is( $foo[1], 9, '$LIST_SEPARATOR' );

undef $OUTPUT_RECORD_SEPARATOR;

eval 'NO SUCH FUNCTION';
like( $EVAL_ERROR, qr/method/, '$EVAL_ERROR' );

is( $UID, $<, '$UID' );
is( $GID, $(, '$GID' );
is( $EUID, $>, '$EUID' );
is( $EGID, $), '$EGID' );

is( $PROGRAM_NAME, $0, '$PROGRAM_NAME' );
is( $BASETIME, $^T, '$BASETIME' );

is( $PERL_VERSION, $^V, '$PERL_VERSION' );
is( $OLD_PERL_VERSION, $], '$OLD_PERL_VERSION' );
is( $DEBUGGING, $^D, '$DEBUGGING' );

is( $WARNING, 0, '$WARNING' );
like( $EXECUTABLE_NAME, qr/perl/i, '$EXECUTABLE_NAME' );
is( $OSNAME, $Config{osname}, '$OSNAME' );

# may be non-portable
ok( $SYSTEM_FD_MAX >= 2, '$SYSTEM_FD_MAX should be at least 2' );

is( $INPLACE_EDIT, '.inplace', '$INPLACE_EDIT' );

'aabbcc' =~ /(.{2}).+(.{2})(?{ 9 })/;
is( $LAST_PAREN_MATCH, 'cc', '$LAST_PAREN_MATCH' );
is( $LAST_REGEXP_CODE_RESULT, 9, '$LAST_REGEXP_CODE_RESULT' );

is( $LAST_MATCH_START[1], 0, '@LAST_MATCH_START' );
is( $LAST_MATCH_END[1], 2, '@LAST_MATCH_END' );

ok( !$PERLDB, '$PERLDB should be false' );

{
	local $INPUT_RECORD_SEPARATOR = "\n\n";
	like( <DATA>, qr/a paragraph./, '$INPUT_RECORD_SEPARATOR' );
}
like( <DATA>, qr/second paragraph..\z/s, '$INPUT_RECORD_SEPARATOR' );

is( $INPUT_LINE_NUMBER, 2, '$INPUT_LINE_NUMBER' );

my %hash;
$SUBSCRIPT_SEPARATOR = '|';
$hash{d,e,f} = 1;
$SUBSEP = ',';
$hash{'a', 'b', 'c'} = 1;
my @keys = sort keys %hash;

is( $keys[0], 'a,b,c', '$SUBSCRIPT_SEPARATOR' );
is( $keys[1], 'd|e|f', '$SUBSCRIPT_SEPARATOR' );

eval { is( $EXCEPTIONS_BEING_CAUGHT, 1, '$EXCEPTIONS_BEING_CAUGHT' ) };
ok( !$EXCEPTIONS_BEING_CAUGHT, '$EXCEPTIONS_BEING_CAUGHT should be false' );

eval { local *F; my $f = 'asdasdasd'; ++$f while -e $f; open(F, '<', $f); };
is( $OS_ERROR, $ERRNO, '$OS_ERROR' );
ok( $OS_ERROR{ENOENT}, '%OS_ERROR (ENOENT should be set)' );

package B;

use English;

"abc" =~ /b/;

main::is( $PREMATCH, 'a', '$PREMATCH defined' );
main::is( $MATCH, 'b', '$MATCH defined' );
main::is( $POSTMATCH, 'c', '$POSTMATCH defined' );

{
    my $s = "xyz";
    $s =~ s/y/t$MATCH/;
    main::is( $s, "xtyz", '$MATCH defined in right side of s///' );
}

package C;

use English qw( -no_match_vars ) ;

"abc" =~ /b/;

main::ok( !$PREMATCH, '$PREMATCH disabled' );
main::ok( !$MATCH, '$MATCH disabled' );
main::ok( !$POSTMATCH, '$POSTMATCH disabled' );


# Check that both variables change when localized.
{
    local $LIST_SEPARATOR = "wibble";
    ::is $", 'wibble', '$" changes when $LIST_SEPARATOR is localized';

    local $" = 'frooble';
    ::is $LIST_SEPARATOR, 'frooble';
}

# because of the 'package' statements above, we have to prefix Test::More::
Test::More::done_testing();

__END__
This is a line.
This is a paragraph.

This is a second paragraph.
It has several lines.
