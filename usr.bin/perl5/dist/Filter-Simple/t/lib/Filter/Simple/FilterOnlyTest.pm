package Filter::Simple::FilterOnlyTest;

use Filter::Simple;

FILTER_ONLY
	string => sub {
		my $class = shift;
		while (my($pat, $str) = splice @_, 0, 2) {
			s/$pat/$str/g;
		}
	},
	code_no_comments => sub {
		$_ =~ /shromple/ and die "We wants no shromples!";
	};
