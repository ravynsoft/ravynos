
use strict;
use warnings;
use Carp;

BEGIN 
{
    eval { require Compress::Zlib ; };
    if ($@) {
        print "1..0 # Skip: Compress::Zlib is not available\n";
print "# $@\n";
        exit 0;
    }
}
require "dbm_filter_util.pl";

use Test::More;

BEGIN { use_ok('DBM_Filter') };
my $db_file;
BEGIN {
    use Config;
    foreach (qw/SDBM_File ODBM_File NDBM_File GDBM_File DB_File/) {
        if ($Config{extensions} =~ /\b$_\b/) {
            $db_file = $_;
            last;
        }
    }
    use_ok($db_file);
};
BEGIN { use_ok('Fcntl') };
BEGIN { use_ok('Compress::Zlib') };

unlink <cmpOp_dbmx*>;
END { unlink <cmpOp_dbmx*>; }

my %h1 = () ;
my $db1 = tie(%h1, $db_file,'cmpOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db1, "tied to $db_file";

# store before adding the filter

StoreData(\%h1,
	{	
		1234	=> 5678,
		-3	=> -5,
		"22"	=> "88",
		"-45"	=> "-88",
		"fred"	=> "Joe",
		"alpha"	=> "Alpha",
		"Beta"	=> "beta",
	});

VerifyData(\%h1,
	{
		1234	=> 5678,
		-3	=> -5,
		"22"	=> "88",
		"-45"	=> "-88",
		"fred"	=> "Joe",
		"alpha"	=> "Alpha",
		"Beta"	=> "beta",
	});


eval { $db1->Filter_Push('compress') };
is $@, '', "push a 'compress' filter" ;

{
    no warnings 'uninitialized';
    StoreData(\%h1,
	{	
		undef()	=> undef(),
		"400"	=> "500",
		0	=> 1,
		1	=> 0,
		"abc"	=> "de0",
		"\x00\x01"	=> "\x03\xFF",
	});

}

undef $db1;
{
    use warnings FATAL => 'untie';
    eval { untie %h1 };
    is $@, '', "untie without inner references" ;
}

# read the dbm file without the filter
my %h2 = () ;
my $db2 = tie(%h2, $db_file,'cmpOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db2, "tied to $db_file";

VerifyData(\%h2,
	{
		1234	=> 5678,
		-3	=> -5,
		"22"	=> "88",
		"-45"	=> "-88",
		"fred"	=> "Joe",
		"alpha"	=> "Alpha",
		"Beta"	=> "beta",

		compress("")	=> compress(""),
		compress("400")	=> compress("500"),
		compress("0")	=> compress("1"),
		compress("1")	=> compress("0"),
		compress("abc")	=> compress("de0"),
		compress("\x00\x01")	=> compress("\x03\xFF"),
	});

undef $db2;
{
    use warnings FATAL => 'untie';
    eval { untie %h2 };
    is $@, '', "untie without inner references" ;
}

done_testing();
