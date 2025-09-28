
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

unlink <intOp_dbmx*>;
END { unlink <intOp_dbmx*>; }

my %h1 = () ;
my $db1 = tie(%h1, $db_file,'intOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db1, "tied to $db_file";

# store before adding the filter

StoreData(\%h1,
	{	
		1234	=> 5678,
		-3	=> -5,
		"22"	=> "88",
		"-45"	=> "-88",
	});

VerifyData(\%h1,
	{
		1234	=> 5678,
		-3	=> -5,
		22	=> 88,
		-45	=> -88,
	});


eval { $db1->Filter_Push('int32') };
is $@, '', "push an 'int32' filter" ;

{
    no warnings 'uninitialized';
    StoreData(\%h1,
	{	
		"400"	=> "500",
                undef()        => 1,
		1	=> 0,
		-47	=> -6,
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
my $db2 = tie(%h2, $db_file,'intOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db2, "tied to $db_file";

VerifyData(\%h2,
	{
		1234	=> 5678,
		-3	=> -5,
		22	=> 88,
		-45	=> -88,

		#undef()	=> undef(),
		pack("i", 400)	=> pack("i", 500),
		pack("i", 0)	=> pack("i", 1),
		pack("i", 1)	=> pack("i", 0),
		pack("i", -47)	=> pack("i", -6),
	});

undef $db2;
{
    use warnings FATAL => 'untie';
    eval { untie %h2 };
    is $@, '', "untie without inner references" ;
}

done_testing();
