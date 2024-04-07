
use strict;
use warnings;
use Carp;

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

unlink <nullOp_dbmx*>;
END { unlink <nullOp_dbmx*>; }

my %h1 = () ;
my $db1 = tie(%h1, $db_file,'nullOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db1, "tied to $db_file";

# store before adding the filter

StoreData(\%h1,
	{	
		"abc"	=> "def",
	});

VerifyData(\%h1,
	{
		"abc"	=> "def",
	});


eval { $db1->Filter_Push('null') };
is $@, '', "push a 'null' filter" ;

{
    no warnings 'uninitialized';
    StoreData(\%h1,
	{	
		undef()	=> undef(),
		"alpha"	=> "beta",
	});

    VerifyData(\%h1,
	{
		undef()	=> undef(),
		"abc"	=> "", # not "def", because the filter is in place
		"alpha"	=> "beta", 
	});
}

    while (my ($k, $v) = each %h1) {
        no warnings 'uninitialized';
        #diag "After Match [$k][$v]"; 
    }


undef $db1;
{
    use warnings FATAL => 'untie';
    eval { untie %h1 };
    is $@, '', "untie without inner references" ;
}

# read the dbm file without the filter, check for null termination
my %h2 = () ;
my $db2 = tie(%h2, $db_file,'nullOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db2, "tied to $db_file";

VerifyData(\%h2,
	{
		"abc"		=> "def",
		"alpha\x00"	=> "beta\x00",
		"\x00"		=> "\x00",
	});

undef $db2;
{
    use warnings FATAL => 'untie';
    eval { untie %h2 };
    is $@, '', "untie without inner references" ;
}

done_testing();
