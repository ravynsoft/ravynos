$DB::single = 1;

package Demo;
$VERSION = '1.00';
use Attribute::Handlers;
no warnings 'redefine';

sub Demo : ATTR(SCALAR) {
	my ($package, $symbol, $referent, $attr, $data, $phase) = @_;
	$data = '<undef>' unless defined $data;
	print STDERR 'Scalar $', *{$symbol}{NAME},
		     " ($referent) was ascribed ${attr}\n",
		     "with data ($data)\nin phase $phase\n";
};

sub This : ATTR(SCALAR) {
	print STDERR "This at ",
		     join(":", map { defined() ? $_ : "" } caller(1)),
		     "\n";
}

sub Demo : ATTR(HASH) {
	my ($package, $symbol, $referent, $attr, $data) = @_;
	$data = '<undef>' unless defined $data;
	print STDERR 'Hash %', *{$symbol}{NAME},
		     " ($referent) was ascribed ${attr} with data ($data)\n";
};

sub Demo : ATTR(CODE) {
	my ($package, $symbol, $referent, $attr, $data) = @_;
	$data = '<undef>' unless defined $data;
	print STDERR 'Sub &', *{$symbol}{NAME},
		     " ($referent) was ascribed ${attr} with data ($data)\n";
};

sub Multi : ATTR {
	my ($package, $symbol, $referent, $attr, $data) = @_;
	$data = '<undef>' unless defined $data;
	print STDERR ref($referent), ' ', *{$symbol}{NAME},
		     " ($referent) was ascribed ${attr} with data ($data)\n";
};

sub ExplMulti : ATTR(ANY) {
	my ($package, $symbol, $referent, $attr, $data) = @_;
	$data = '<undef>' unless defined $data;
	print STDERR ref($referent), ' ', *{$symbol}{NAME},
		     " ($referent) was ascribed ${attr} with data ($data)\n";
};

1;
