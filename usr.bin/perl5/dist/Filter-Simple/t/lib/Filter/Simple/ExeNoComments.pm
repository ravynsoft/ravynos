package Filter::Simple::ExeNoComments;

use Filter::Simple;

FILTER_ONLY
  executable_no_comments => sub {
            $_ =~ /shromplex/ and die "We wants no shromplexes!";
            s/ABC/TEST/g;
	};

1;
