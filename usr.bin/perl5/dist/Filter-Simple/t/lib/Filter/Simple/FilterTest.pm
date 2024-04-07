package Filter::Simple::FilterTest;

use Filter::Simple;

FILTER {
	my $class = shift;
	while (my($pat, $str) = splice @_, 0, 2) {
		s/$pat/$str/g;
	}
};

1;
