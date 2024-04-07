BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;
use File::Basename;
use File::Spec;
use Encode qw(decode encode find_encoding _utf8_off);

#use Test::More qw(no_plan);
use Test::More tests => 32;
BEGIN { use_ok("Encode::Guess") }

my $ascii  = join('' => map {chr($_)}(0x21..0x7e));
my $latin1 = join('' => map {chr($_)}(0xa1..0xfe));
my $utf8on  = join('' => map {chr($_)}(0x3000..0x30fe));
my $utf8off = $utf8on; _utf8_off($utf8off);
my $utf16 = encode('UTF-16', $utf8on);
my $utf32 = encode('UTF-32', $utf8on);

like(guess_encoding(''), qr/empty string/io, 'empty string');
is(guess_encoding($ascii)->name, 'ascii', 'ascii');
like(guess_encoding($latin1), qr/No appropriate encoding/io, 'no ascii');
is(guess_encoding($latin1, 'latin1')->name, 'iso-8859-1', 'iso-8859-1');
is(guess_encoding($utf8on)->name, 'utf8', 'utf8 w/ flag');
is(guess_encoding($utf8off)->name, 'utf8', 'utf8 w/o flag');
is(guess_encoding($utf16)->name, 'UTF-16', 'UTF-16');
is(guess_encoding($utf32)->name, 'UTF-32', 'UTF-32');

my $jisx0201 = File::Spec->catfile(dirname(__FILE__), 'jisx0201.utf');
my $jisx0208 = File::Spec->catfile(dirname(__FILE__), 'jisx0208.utf');
my $jisx0212 = File::Spec->catfile(dirname(__FILE__), 'jisx0212.utf');

open my $fh, $jisx0208 or die "$jisx0208: $!";
binmode($fh);
$utf8off = join('' => <$fh>);
close $fh;
$utf8on = decode('utf8', $utf8off);

my @jp = qw(7bit-jis shiftjis euc-jp);

Encode::Guess->set_suspects(@jp);

for my $jp (@jp){
    my $test = encode($jp, $utf8on);
    is(guess_encoding($test)->name, $jp, "JP:$jp");
}

is (decode('Guess', encode('euc-jp', $utf8on)), $utf8on, "decode('Guess')");
eval{ encode('Guess', $utf8on) };
like($@, qr/not defined/io, "no encode()");

{
    my $warning;
    local $SIG{__WARN__} = sub { $warning = shift };
    my $euc_jp = my $euc_jp_clone = encode('euc-jp', $utf8on);
    Encode::from_to($euc_jp, 'Guess', 'euc-jp');
    is $euc_jp_clone, $euc_jp, "from_to(..., 'Guess')";
    ok !$warning, "no warning";
    diag $warning if $warning;
}

my %CJKT = 
    (
     'euc-cn'    => File::Spec->catfile(dirname(__FILE__), 'gb2312.utf'),
     'euc-jp'    => File::Spec->catfile(dirname(__FILE__), 'jisx0208.utf'),
     'euc-kr'    => File::Spec->catfile(dirname(__FILE__), 'ksc5601.utf'),
     'big5-eten' => File::Spec->catfile(dirname(__FILE__), 'big5-eten.utf'),
);

Encode::Guess->set_suspects(keys %CJKT);

for my $name (keys %CJKT){
    open my $fh, $CJKT{$name} or die "$CJKT{$name}: $!";
    binmode($fh);
    $utf8off = join('' => <$fh>);
    close $fh;

    my $test = encode($name, decode('utf8', $utf8off));
    is(guess_encoding($test)->name, $name, "CJKT:$name");
}

my $ambiguous =  "\x{5c0f}\x{98fc}\x{5f3e}";
my $english   = "The quick brown fox jumps over the black lazy dog.";
for my $utf (qw/UTF-16 UTF-32/){
    for my $bl (qw/BE LE/){
    my $test = encode("$utf$bl" => $english);
    is(guess_encoding($test)->name, "$utf$bl", "$utf$bl");
    }
}
for my $bl (qw/BE LE/){
    my $test = encode("UTF-16$bl" => $ambiguous);
    my $result = guess_encoding($test);
    ok(! ref($result), "UTF-16$bl:$result");
}



Encode::Guess->set_suspects();
for my $jp (@jp){
    # intentionally set $1 a priori -- see Changes
    my $test = "English";
    '$1' =~ m/^(.*)/o;
    is(guess_encoding($test, ($jp))->name, 'ascii', 
       "ascii vs $jp (\$1 messed)");
    $test = encode($jp, $test . "\n\x{65e5}\x{672c}\x{8a9e}");
    is(guess_encoding($test, ($jp))->name, 
       $jp, "$jp vs ascii (\$1 messed)");
}

__END__;
