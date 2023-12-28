% Check that we don't get an error on the LOC directive; that we don't
% interpret it as a (section-relative) negative number.

	LOC     #8000000000042000
Boot    GETA    $0,Boot        %set dynamic- and forced-trap  handler
	LOC     #8000000000042010
Toot	GETA	$5,Toot
