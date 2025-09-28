#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
    require 'test.pl';
}

$| = 1;
use warnings;
use strict;
use B::Showlex ();

plan tests => 15;

my $verbose = @ARGV; # set if ANY ARGS

my $path = join " ", map { qq["-I$_"] } @INC;

my $o = `$^X $path "-MO=Showlex" -e "my \@one" 2>&1`;
like ($o, qr/undef.*: \([^)]*\) \@one.*Nullsv.*AV/s,
      "canonical usage works");

# v1.01 tests

my ($na,$nb,$nc);	# holds regex-strs
my ($out, $newlex);	# output, option-flag

sub padrep {
    my ($varname,$newlex) = @_;
    return ($newlex)
	? '\(0x[0-9a-fA-F]+\) "\\'.$varname.'" = '
	: "\\\(0x[0-9a-fA-F]+\\\) \\$varname\n";
}

for $newlex ('', '-newlex') {

    $out = runperl ( switches => ["-MO=Showlex,$newlex"],
		     prog => 'my ($a,$b)', stderr => 1 );
    $na = padrep('$a',$newlex);
    $nb = padrep('$b',$newlex);
    like ($out, qr/1: $na/ms, 'found $a in "my ($a,$b)"');
    like ($out, qr/2: $nb/ms, 'found $b in "my ($a,$b)"');

    print $out if $verbose;

    our $buf = 'arb startval';
    my $ak = B::Showlex::walk_output (\$buf);

    my $walker = B::Showlex::compile( $newlex, sub{my($foo,$bar)} );
    $walker->();
    $na = padrep('$foo',$newlex);
    $nb = padrep('$bar',$newlex);
    like ($buf, qr/1: $na/ms, 'found $foo in "sub { my ($foo,$bar) }"');
    like ($buf, qr/2: $nb/ms, 'found $bar in "sub { my ($foo,$bar) }"');

    print $buf if $verbose;

    $ak = B::Showlex::walk_output (\$buf);

    my $src = 'sub { my ($scalar,@arr,%hash) }';
    my $sub = eval $src;
    $walker = B::Showlex::compile($sub);
    $walker->();
    $na = padrep('$scalar',$newlex);
    $nb = padrep('@arr',$newlex);
    $nc = padrep('%hash',$newlex);
    like ($buf, qr/1: $na/ms, 'found $scalar in "'. $src .'"');
    like ($buf, qr/2: $nb/ms, 'found @arr    in "'. $src .'"');
    like ($buf, qr/3: $nc/ms, 'found %hash   in "'. $src .'"');

    print $buf if $verbose;

    # fibonacci function under test
    my $asub = sub {
	my ($self,%props)=@_;
	my $total;
	{ # inner block vars
	    my (@fib)=(1,2);
	    for (my $i=2; $i<10; $i++) {
		$fib[$i] = $fib[$i-2] + $fib[$i-1];
	    }
	    for my $i(0..10) {
		$total += $i;
	    }
	}
    };
    $walker = B::Showlex::compile($asub, $newlex, -nosp);
    $walker->();
    print $buf if $verbose;

    $walker = B::Concise::compile($asub, '-exec');
    $walker->();

}
