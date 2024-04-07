
use strict;
use warnings;
use Carp;

BEGIN 
{

    require "../t/charset_tools.pl";

    eval { require Encode; };
    
    if ($@) {
        print "1..0 #  Skip: Encode is not available\n";
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
BEGIN { use_ok('charnames', qw{greek})};

use charnames qw{greek};

unlink <utf8Op_dbmx*>;
END { unlink <utf8Op_dbmx*>; }

my %h1 = () ;
my $db1 = tie(%h1, $db_file,'utf8Op_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db1, "tied to $db_file";

eval { $db1->Filter_Push('utf8') };
is $@, '', "push a 'utf8' filter" ;

{
    no warnings 'uninitialized';
    StoreData(\%h1,
	{	
		undef()	=> undef(),
		"beta"	=> "\N{beta}",
		'alpha'	=> "\N{alpha}",
		"\N{gamma}"=> "gamma",
	});

}

VerifyData(\%h1,
	{
		'alpha'	=> "\N{alpha}",
		"beta"	=> "\N{beta}",
		"\N{gamma}"=> "gamma",
		""		=> "",
	});

undef $db1;
{
    use warnings FATAL => 'untie';
    eval { untie %h1 };
    is $@, '', "untie without inner references" ;
}

# read the dbm file without the filter
my %h2 = () ;
my $db2 = tie(%h2, $db_file,'utf8Op_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db2, "tied to $db_file";

VerifyData(\%h2,
        {
        'alpha'	=> byte_utf8a_to_utf8n("\xCE\xB1"),
        'beta'	=> byte_utf8a_to_utf8n("\xCE\xB2"),
        byte_utf8a_to_utf8n("\xCE\xB3")=> "gamma",
        ""		=> "",
        });

undef $db2;
{
    use warnings FATAL => 'untie';
    eval { untie %h2 };
    is $@, '', "untie without inner references" ;
}

done_testing();
