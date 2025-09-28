use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Tabs;

my @tests = (split(/\nEND\n/s, <<DONE));
TEST 1 u
                x
END
		x
END
TEST 2 e
		x
END
                x
END
TEST 3 e
	x
		y
			z
END
        x
                y
                        z
END
TEST 4 u
        x
                y
                        z
END
	x
		y
			z
END
TEST 5 u
This    Is      a       test    of      a       line with many embedded tabs
END
This	Is	a	test	of	a	line with many embedded tabs
END
TEST 6 e
This	Is	a	test	of	a	line with many embedded tabs
END
This    Is      a       test    of      a       line with many embedded tabs
END
TEST 7 u
            x
END
	    x
END
TEST 8 e
	
		
   	

           
END
        
                
        

           
END
TEST 9 u
           
END
	   
END
TEST 10 u
	
		
   	

           
END
	
		
	

	   
END
TEST 11 u
foobar                  IN	A		140.174.82.12

END
foobar			IN	A		140.174.82.12

END
DONE


my $numtests = scalar(@tests) / 2;
print "1..$numtests\n";

while (@tests) {
	my $in = shift(@tests);
	my $out = shift(@tests);

	$in =~ s/^TEST\s*(\d+)?\s*(\S+)?\n//;

	my $f = $2 eq 'e' ? \&expand : \&unexpand;

	my $back = &$f($in);

	ok( $back eq $out );
}
