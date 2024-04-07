#!./perl

# quickie tests to see if h2ph actually runs and does more or less what is
# expected

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

require './test.pl';

my $extracted_program = '../utils/h2ph'; # unix, nt, ...
if ($^O eq 'VMS') { $extracted_program = '[-.utils]h2ph.com'; }
if (!(-e $extracted_program)) {
    print "1..0 # Skip: $extracted_program was not built\n";
    exit 0;
}

plan(6);

# quickly compare two text files
sub txt_compare {
    local $/;
    my ($A, $B);
    for (($A,$B) = @_) { open(_,"<",$_) ? $_ = <_> : die "$_ : $!"; close _ }
    $A cmp $B;
}

my $result = runperl( progfile => $extracted_program,
                      stderr => 1,
                      args => ['-d.', '-Q', 'lib/h2ph.h']);
is( $result, '', "output is free of warnings" );
is( $?, 0, "$extracted_program runs successfully" );

is ( txt_compare("lib/h2ph.ph", "lib/h2ph.pht"),
     0,
     "generated file has expected contents" );

$result = runperl( progfile => 'lib/h2ph.pht',
                   switches => ['-c'],
                   stderr => 1 );
like( $result, qr/syntax OK$/, "output compiles");

$result = runperl( progfile => '_h2ph_pre.ph',
                   switches => ['-c'],
                   stderr => 1 );
like( $result, qr/syntax OK$/, "preamble compiles");

$result = runperl( switches => ['-I.', "-w"],
                   stderr => 1,
                   prog => <<'PROG' );
$SIG{__WARN__} = sub { die $_[0] }; require q(lib/h2ph.pht);
PROG
is( $result, '', "output free of warnings" );

# cleanup
END {
    1 while unlink("lib/h2ph.ph");
    1 while unlink("_h2ph_pre.ph");
}
