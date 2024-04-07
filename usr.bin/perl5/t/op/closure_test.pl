# This file exists to test closure prototypes with no CvOUTSIDE.  Only
# by putting this in a separate file can we get a sub (this file’s
# main CV) with no CvOUTSIDE.  When the outer sub is freed, the inner
# subs also get CvOUTSIDE set to null.

	my $x;
	$closure_test::s2 = sub {
	    $x;
	    sub { $x; '10 cubes' };
	};
