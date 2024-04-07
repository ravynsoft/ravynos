
use strict;
use warnings;
use Carp;

require "../t/charset_tools.pl";

BEGIN 
{

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

unlink <encOp_dbmx*>;
END { unlink <encOp_dbmx*>; }

my %h1 = () ;
my $db1 = tie(%h1, $db_file,'encOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db1, "tied to $db_file";

eval { $db1->Filter_Push('encode' => 'blah') };
like $@, qr/^Encoding 'blah' is not available/, "push an illegal filter" ;

eval { $db1->Filter_Push('encode') };
is $@, '', "push an 'encode' filter (default to utf-8)" ;


{
    no warnings 'uninitialized';
    StoreData(\%h1,
	{	
		undef()	=> undef(),
		'alpha'	=> "\N{alpha}",
		"\N{gamma}"=> "gamma",
		"beta"	=> "\N{beta}",
	});

}

{
    local $TODO = "Currently broken on EBCDIC" if $::IS_EBCDIC;
    VerifyData(\%h1,
	{
		'alpha'	=> "\N{alpha}",
		"beta"	=> "\N{beta}",
		"\N{gamma}"=> "gamma",
		""		=> "",
	});
}

eval { $db1->Filter_Pop() };
is $@, '', "pop the 'utf8' filter" ;

SKIP: {
    skip "Encode doesn't currently work for most filters on EBCDIC, including 8859-16", 11 if $::IS_EBCDIC || $::IS_EBCDIC;
    # Actually the only thing failing below is the euro, because that's the
    # only thing that's added in 8859-16.
eval { $db1->Filter_Push('encode' => 'iso-8859-16') };
is $@, '', "push an 'encode' filter (specify iso-8859-16)" ;

use charnames qw{:full};
StoreData(\%h1,
	{	
		'euro'	=> "\N{EURO SIGN}",
	});

undef $db1;
{
    use warnings FATAL => 'untie';
    eval { untie %h1 };
    is $@, '', "untie without inner references" ;
}

# read the dbm file without the filter
my %h2 = () ;
my $db2 = tie(%h2, $db_file,'encOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db2, "tied to $db_file";

VerifyData(\%h2,
	   {
	    'alpha'	=> byte_utf8a_to_utf8n("\xCE\xB1"),
	    'beta'	=> byte_utf8a_to_utf8n("\xCE\xB2"),
	    byte_utf8a_to_utf8n("\xCE\xB3") => "gamma",
	    'euro'	=> uni_to_native("\xA4"),
	    ""		=> "",
	   });

undef $db2;
{
    use warnings FATAL => 'untie';
    eval { untie %h2 };
    is $@, '', "untie without inner references" ;
}

}

done_testing();
