use strict; use warnings FATAL => 'all';

BEGIN { eval sprintf 'sub NEED_REPEATED_DECODE () { %d }', $] lt '5.008' }

use Text::Wrap;

$Text::Wrap::columns = 72;

require bytes;

our $Errors = 0;

$/ = q();

our @DATA = (
    [ # paragraph 0
	sub { die "there is no paragraph 0" } 
    ],
    { # paragraph 1
	OLD => { BYTES =>    44, CHARS =>   44, CHUNKS =>   44, WORDS =>   7, TABS =>  3, LINES =>  4 },
	NEW => { BYTES =>    44, CHARS =>   44, CHUNKS =>   44, WORDS =>   7, TABS =>  3, LINES =>  4 },
    },
    { # paragraph 2
	OLD => { BYTES =>  1766, CHARS => 1635, CHUNKS => 1507, WORDS => 275, TABS =>  0, LINES =>  2 },
	NEW => { BYTES =>  1766, CHARS => 1635, CHUNKS => 1507, WORDS => 275, TABS =>  0, LINES => 24 },
    },
    { # paragraph 3
	OLD => { BYTES =>   157, CHARS =>  148, CHUNKS =>  139, WORDS =>  27, TABS =>  0, LINES =>  2 },
	NEW => { BYTES =>   157, CHARS =>  148, CHUNKS =>  139, WORDS =>  27, TABS =>  0, LINES =>  3 },
    },
    { # paragraph 4
	OLD => { BYTES =>    30, CHARS =>   25, CHUNKS =>   24, WORDS =>   3, TABS =>  4, LINES =>  1 },
	NEW => { BYTES =>    30, CHARS =>   25, CHUNKS =>   24, WORDS =>   3, TABS =>  4, LINES =>  1 },
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
	for my $item (qw[ bytes chars chunks words tabs ]) {
	    if ( $DATA[$i]{NEW}{uc $item} != $DATA[$i]{OLD}{uc $item} ) {
		warn "\u$item count shouldn't change upon wrapping at table paragraph $i";
		$bad++;
	    } 
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
    warn sprintf("%s paragraph %d expected %d %s, found %d instead",
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
	    die "too many paragraphs of data";
	} 

	$DATA[$.]{OLD}{DATA} = $_;

	my($char_count,  $byte_count, $chunk_count, $word_count, $tab_count, $line_count);

	$byte_count  = bytes::length($_);
	$char_count  = length();
	$chunk_count = () = /\PM/g;
	$word_count  = () = /(?:\pL\pM*)+/g;
	$tab_count   = y/\t//;
	$line_count  = y/\n//;

	$bad++ unless check($byte_count,  $., "OLD", "BYTES");
	$bad++ unless check($char_count,  $., "OLD", "CHARS");
	$bad++ unless check($chunk_count, $., "OLD", "CHUNKS");
	$bad++ unless check($word_count,  $., "OLD", "WORDS");
	$bad++ unless check($tab_count,   $., "OLD", "TABS");
	$bad++ unless check($line_count,  $., "OLD", "LINES");

	my $nl = "\n" x chomp;

	$_ = wrap("", "", $_) . $nl;
	$_ = pack "U0a*", $_ if NEED_REPEATED_DECODE;

	$byte_count  = bytes::length($_);
	$char_count  = length();
	$chunk_count = () = /\PM/g;
	$word_count  = () = /(?:\pL\pM*)+/g;
	$tab_count   = y/\t//;
	$line_count  = y/\n//;

	$bad++ unless check($byte_count,  $., "NEW", "BYTES");
	$bad++ unless check($char_count,  $., "NEW", "CHARS");
	$bad++ unless check($chunk_count, $., "NEW", "CHUNKS");
	$bad++ unless check($word_count,  $., "NEW", "WORDS");
	$bad++ unless check($tab_count,   $., "NEW", "TABS");
	$bad++ unless check($line_count,  $., "NEW", "LINES");

	my $num = $. + 1;
	print $bad ? "not " : "", "ok $num\n";
	$Errors += $bad;

    } 

}

__DATA__
	Los dos reyes
	     y
	Los dos laberintos

Cuentan los hombres dignos de fe (pero A̳l̳á̳ sabe más) que en los primeros días hubo un rey de l̲a̲s̲ i̲s̲l̲a̲s̲ d̲e̲ B̲a̲b̲i̲l̲o̲n̲i̲a̲ que congregó a sus arquitectos y magos y les mandó construir un laberinto tan perplejo y sutil que los varones más prudentes no se aventuraban a entrar, y los que entraban se perdían.  Esa obra era un escándalo, porque la confusión y la maravilla son operaciones propias de D̳i̳o̳s̳ y no de los hombres.  Con el andar del tiempo vino a su corte un rey de los árabes, y el rey de B̲a̲b̲i̲l̲o̲n̲i̲a̲ (para hacer burla de la simplicidad de su huésped) lo hizo penetrar en el laberinto, donde vagó afrentado y confundido hasta la declinación de la tarde.  Entonces imploró socorro divino y dio con la puerta.  Sus labios no profirieron queja ninguna, pero le dijo al rey de B̲a̲b̲i̲l̲o̲n̲i̲a̲ que él en A̲r̲a̲b̲i̲a̲ tenía otro laberinto y que, si D̳i̳o̳s̳ era servido, se lo daría a conocer algún día.  Luego regresó a A̲r̲a̲b̲i̲a̲, juntó sus capitanes y sus alcaides y estragó l̲o̲s̲ r̲e̲i̲n̲o̲s̲ d̲e̲ B̲a̲b̲i̲l̲o̲n̲i̲a̲ con tan venturosa fortuna que derribó sus castillos, rompió sus gentes e hizo cautivo al mismo rey.  Lo amarró encima de un camello veloz y lo llevó al desierto.  Cabalgaron tres días, y le dijo: «¡Oh, rey del tiempo y substancia y cifra del siglo!, en B̲a̲b̲i̲l̲o̲n̲i̲a̲ me quisiste perder en un laberinto de bronce con muchas escaleras, puertas y muros; ahora e̳l̳ P̳o̳d̳e̳r̳o̳s̳o̳ ha tenido a bien que te muestre el mío, donde no hay escaleras que subir, ni puertas que forzar, ni fatigosas galerías que recorrer, ni muros que te veden el paso.»

Luego le desató las ligaduras y lo abandonó en mitad del desierto, donde murió de hambre y de sed.  La gloria sea con A̳q̳u̳é̳l̳ que no muere.

				——Jorge Luís Borges
