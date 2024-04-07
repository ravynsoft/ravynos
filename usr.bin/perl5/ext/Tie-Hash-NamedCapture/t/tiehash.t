#!./perl -w
use strict;

use Test::More;

# this would break the hash magic setup [perl #134193]
my ($ca, $c) = ( \@{^CAPTURE_ALL}, \@{^CAPTURE} );

my %hashes = (
    '+' => \%+,
    '-' => \%-,
    '{^CAPTURE}' => \%{^CAPTURE},
    '{^CAPTURE_ALL}' => \%{^CAPTURE_ALL},
);

foreach (['plus1'],
	 ['minus1', all => 1],
	 ['plus2', all => 0],
	 ['plus3', zlonk => 1],
	 ['minus2', thwapp => 0, all => 1],
	) {
    my $name = shift @$_;
    my $hash = $hashes{$name} = {};
    isa_ok(tie(%$hash, 'Tie::Hash::NamedCapture', @$_),
	   'Tie::Hash::NamedCapture', "%$name");
}

is("abcdef" =~ /(?<foo>[ab])*(?<bar>c)(?<foo>d)(?<bar>[ef]*)/, 1,
   "We matched");

foreach my $name (qw(+ {^CAPTURE} plus1 plus2 plus3)) {
    my $hash = $hashes{$name};
    is_deeply($hash, { foo => 'b', bar => 'c' }, "%$name is as expected");
}

foreach my $name (qw(- {^CAPTURE_ALL} minus1 minus2)) {
    my $hash = $hashes{$name};
    is_deeply($hash, { foo => [qw(b d)], bar => [qw(c ef)] },
	      "%$name is as expected");
}

done_testing();
