use strict; use warnings FATAL => 'all';

BEGIN { eval sprintf 'sub NEED_REPEATED_DECODE () { %d }', $] lt '5.008' }

use Text::Tabs;

require bytes;

our $Errors = 0;

our @DATA = (
    [ # DATALINE #0
	sub { die "there is no line 0" } 
    ],
    { # DATALINE #1
	OLD => { BYTES =>  71, CHARS => 59, CHUNKS => 47, WORDS => 7, TABS => 3 },
	NEW => { BYTES =>  92, CHARS => 80, CHUNKS => 68, WORDS => 7, TABS => 0 },
    },
    { # DATALINE #2
	OLD => { BYTES =>  45, CHARS => 43, CHUNKS => 41, WORDS => 6, TABS => 3 },
	NEW => { BYTES =>  65, CHARS => 63, CHUNKS => 61, WORDS => 6, TABS => 0 },
    },
    { # DATALINE #3
	OLD => { BYTES =>  47, CHARS => 45, CHUNKS => 43, WORDS => 7, TABS => 3 },
	NEW => { BYTES =>  64, CHARS => 62, CHUNKS => 60, WORDS => 7, TABS => 0 },
    },
    { # DATALINE #4
	OLD => { BYTES =>  49, CHARS => 47, CHUNKS => 45, WORDS => 7, TABS => 3 },
	NEW => { BYTES =>  69, CHARS => 67, CHUNKS => 65, WORDS => 7, TABS => 0 },
    },
    { # DATALINE #5
	OLD => { BYTES =>  83, CHARS => 62, CHUNKS => 41, WORDS => 7, TABS => 4 },
	NEW => { BYTES => 105, CHARS => 84, CHUNKS => 63, WORDS => 7, TABS => 0 },
    },
    { # DATALINE #6
	OLD => { BYTES =>  55, CHARS => 53, CHUNKS => 51, WORDS => 8, TABS => 3 },
	NEW => { BYTES =>  76, CHARS => 74, CHUNKS => 72, WORDS => 8, TABS => 0 },
    },
    { # DATALINE #7
	OLD => { BYTES =>  42, CHARS => 40, CHUNKS => 38, WORDS => 7, TABS => 4 },
	NEW => { BYTES =>  65, CHARS => 63, CHUNKS => 61, WORDS => 7, TABS => 0 },
    },
    { # DATALINE #8
	OLD => { BYTES =>  80, CHARS => 65, CHUNKS => 52, WORDS => 9, TABS => 1 },
	NEW => { BYTES =>  87, CHARS => 72, CHUNKS => 59, WORDS => 9, TABS => 0 },
    },
    { # DATALINE #9
	OLD => { BYTES =>  43, CHARS => 41, CHUNKS => 41, WORDS => 7, TABS => 3 },
	NEW => { BYTES =>  63, CHARS => 61, CHUNKS => 61, WORDS => 7, TABS => 0 },
    },
);


my $numtests = @DATA;
print "1..$numtests\n";

$Errors += table_ok();
check_data();

if ($Errors) {
    die "Error count: $Errors";
} else {
    exit(0);
} 


# first some sanity checks
sub table_ok { 
    my $bad = 0;
    for my $i ( 1 .. $#DATA ) {

	if ( $DATA[$i]{NEW}{TABS} ) {
	    warn "new data should have no tabs in it at table line $i";
	    $bad++;
	} 

	if ( $DATA[$i]{NEW}{WORDS} != $DATA[$i]{OLD}{WORDS} ) {
	    warn "word count shouldn't change upon tab expansion at table line $i";
	    $bad++;
	} 
    } 
    print $bad ? "not " : "", "ok 1\n";
    return $bad;
}

sub check($$$$) {
    die "expected 4 arguments" unless @_ == 4;
    my ($found, $index, $version, $item) = @_;
    my $expected = $DATA[$index]{$version}{$item};
    return 1 if $found == $expected;
    warn sprintf("%s line %d expected %d %s, found %d instead",
		  ucfirst(lc($version)), 
			  $index,     $expected, 
					 lc($item),  
						 $found);
    return 0;
} 

sub check_data { 

    local($_);
    while ( <DATA> ) {
	$_ = pack "U0a*", $_;

	my $bad = 0;

	if ($. > $#DATA) {
	    die "too many lines of data";
	} 

	$DATA[$.]{OLD}{DATA} = $_;

	my($char_count,  $byte_count, $chunk_count, $word_count, $tab_count);

	$byte_count  = bytes::length($_);
	$char_count  = length();
	$chunk_count = () = /\PM/g;
	$word_count  = () = /(?:\pL\pM*)+/g;
	$tab_count   = y/\t//;

	$bad++ unless check($byte_count,  $., "OLD", "BYTES");
	$bad++ unless check($char_count,  $., "OLD", "CHARS");
	$bad++ unless check($chunk_count, $., "OLD", "CHUNKS");
	$bad++ unless check($word_count,  $., "OLD", "WORDS");
	$bad++ unless check($tab_count,   $., "OLD", "TABS");

	$_ = expand($_);
	$_ = pack "U0a*", $_ if NEED_REPEATED_DECODE;

	$DATA[$.]{NEW}{DATA} = $_;

	$byte_count  = bytes::length($_);
	$char_count  = length();
	$chunk_count = () = /\PM/g;
	$word_count  = () = /(?:\pL\pM*)+/g;
	$tab_count   = y/\t//;

	$bad++ unless check($byte_count,  $., "NEW", "BYTES");
	$bad++ unless check($char_count,  $., "NEW", "CHARS");
	$bad++ unless check($chunk_count, $., "NEW", "CHUNKS");
	$bad++ unless check($word_count,  $., "NEW", "WORDS");
	$bad++ unless check($tab_count,   $., "NEW", "TABS");

	$_ = unexpand($_);
	$_ = pack "U0a*", $_ if NEED_REPEATED_DECODE;

	if ($_ ne $DATA[$.]{OLD}{DATA}) {
	    warn "expand/unexpand round-trip equivalency failed at line $.";
	    warn sprintf("  Expected:\n%s\n%v02x\n  But got:\n%s\n%v02x\n",
		    ( $DATA[$.]{OLD}{DATA} ) x 2, ($_) x 2 );
	    $bad++;
	} 

	my $num = $. + 1;
	print $bad ? "not " : "", "ok $num\n";
	$Errors += $bad;

    } 

}


__DATA__
	De los sos o̲j̲o̲s̲ 		tan fuertemientre l̲l̲o̲r̲a̲n̲d̲o̲,
	tornava la cabeça		i estávalos catando.
	Vio puertas abiertas		e uços sin cañados,
	alcándaras vázias		sin pielles e sin mantos
	e s̲i̲n̲ f̲a̲l̲c̲o̲n̲e̲s̲			e s̲i̲n̲ a̲d̲t̲o̲r̲e̲s̲ mudados.
	Sospiró mio Çid,		ca mucho avie grandes cuidados.
	Fabló mio Çid			bien e tan mesurado:
       “grado a tí, s̳e̳ñ̳o̳r̳ p̳a̳d̳r̳e̳,	que estás en alto!
	Esto me an buelto		mis enemigos malos.”
