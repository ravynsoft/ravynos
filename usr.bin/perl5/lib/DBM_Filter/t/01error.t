use strict;
use warnings;

BEGIN {
    chdir 't' if -d 't';
    @INC = qw(. ../lib);
}

use Carp;
use File::Temp qw(tempdir);

my $tempdir;
{
    $tempdir = tempdir( "./DBMFXXXXXXXX", CLEANUP => 1);
    push @INC, $tempdir;
    chdir $tempdir or die "Failed to chdir to '$tempdir': $!";
    @INC[-1] = "../../lib";
    if ( ! -d 'DBM_Filter')
    {
        mkdir 'DBM_Filter', 0777 
	    or die "Cannot create directory 'DBM_Filter': $!\n" ;
    }
}

##### Keep above code identical to 02core.t #####

our $db;

sub writeFile
{
    my $filename = shift ;
    my $content = shift;
    open F, '>', $filename or croak "Cannot open $filename: $!" ;
    print F $content ;
    close F;
}

sub runFilter
{
    my $name = shift ;
    my $filter = shift ;

    #print "# runFilter $name\n" ;
    my $filename = "DBM_Filter/$name.pm";
    $filter = "package DBM_Filter::$name ;\n$filter"
        unless $filter =~ /^\s*package/ ;

    writeFile($filename, $filter);
    eval { $db->Filter_Push($name) };
    unlink $filename;
    return $@;
}

use Test::More;

BEGIN { use_ok('DBM_Filter') };
my $db_file;
BEGIN {
    use Config;
    foreach (qw/ODBM_File SDBM_File NDBM_File GDBM_File DB_File/) {
        if ($Config{extensions} =~ /\b$_\b/) {
            $db_file = $_;
            last;
        }
    }
    use_ok($db_file);
};
BEGIN { use_ok('Fcntl') };

unlink <errOp_dbmx*>;
END { unlink <errOp_dbmx*>; }

my %h1 = () ;
my %h2 = () ;
$db = tie(%h1, $db_file,'errOp_dbmx', O_RDWR|O_CREAT, 0640) ;

ok $db, "tied to $db_file ok";


# Error cases

eval { $db->Filter_Push() ; };
like $@, qr/^Filter_Push: no parameters present/,
        "croak if not parameters passed to Filter_Push";

eval { $db->Filter_Push("unknown_class") ; };
like $@, qr/^Filter_Push: Cannot Load DBM Filter 'DBM_Filter::unknown_class'/, 
        "croak on unknown class" ;

eval { $db->Filter_Push("Some::unknown_class") ; };
like $@, qr/^Filter_Push: Cannot Load DBM Filter 'Some::unknown_class'/, 
        "croak on unknown fully qualified class" ;

eval { $db->Filter_Push('Store') ; };
like $@, qr/^Filter_Push: not even params/,
        "croak if not passing even number or params to Filter_Push";

runFilter('bad1', <<'EOM');
    package DBM_Filter::bad1 ;
    1;
EOM

like $@, qr/^Filter_Push: No methods \(Filter, Fetch or Store\) found in class 'DBM_Filter::bad1'/,
        "croak if none of Filter/Fetch/Store in filter" ;


runFilter('bad2', <<'EOM');
    package DBM_Filter::bad2 ;

    sub Filter
    {
        return 2;
    }

    1;
EOM

like $@, qr/^Filter_Push: 'DBM_Filter::bad2::Filter' did not return a hash reference./,
        "croak if Filter doesn't return hash reference" ;

runFilter('bad3', <<'EOM');
    package DBM_Filter::bad3 ;

    sub Filter
    {
        return { BadKey => sub { } } ;

    }

    1;
EOM

like $@, qr/^Filter_Push: Unknown key 'BadKey'/,
        "croak if bad keyword returned from Filter";

runFilter('bad4', <<'EOM');
    package DBM_Filter::bad4 ;

    sub Filter
    {
        return { Store => "abc" } ;
    }

    1;
EOM

like $@, qr/^Filter_Push: value associated with key 'Store' is not a code reference/,
        "croak if not a code reference";

runFilter('bad5', <<'EOM');
    package DBM_Filter::bad5 ;

    sub Filter
    {
        return { } ;
    }

    1;
EOM

like $@, qr/^Filter_Push: expected both Store & Fetch - got neither/,
        "croak if neither fetch or store is present";

runFilter('bad6', <<'EOM');
    package DBM_Filter::bad6 ;

    sub Filter
    {
        return { Store => sub {} } ;
    }

    1;
EOM

like $@, qr/^Filter_Push: expected both Store & Fetch - got Store/,
        "croak if store is present but fetch isn't";

runFilter('bad7', <<'EOM');
    package DBM_Filter::bad7 ;

    sub Filter
    {
        return { Fetch => sub {} } ;
    }

    1;
EOM

like $@, qr/^Filter_Push: expected both Store & Fetch - got Fetch/,
        "croak if fetch is present but store isn't";

runFilter('bad8', <<'EOM');
    package DBM_Filter::bad8 ;

    sub Filter {}
    sub Store {}
    sub Fetch {}

    1;
EOM

like $@, qr/^Filter_Push: Can't mix Filter with Store and Fetch in class 'DBM_Filter::bad8'/,
        "croak if Fetch, Store and Filter";

runFilter('bad9', <<'EOM');
    package DBM_Filter::bad9 ;

    sub Filter {}
    sub Store {}

    1;
EOM

like $@, qr/^Filter_Push: Can't mix Filter with Store and Fetch in class 'DBM_Filter::bad9'/,
        "croak if Store and Filter";

runFilter('bad10', <<'EOM');
    package DBM_Filter::bad10 ;

    sub Filter {}
    sub Fetch {}

    1;
EOM

like $@, qr/^Filter_Push: Can't mix Filter with Store and Fetch in class 'DBM_Filter::bad10'/,
        "croak if Fetch and Filter";

runFilter('bad11', <<'EOM');
    package DBM_Filter::bad11 ;

    sub Fetch {}

    1;
EOM

like $@, qr/^Filter_Push: Missing method 'Store' in class 'DBM_Filter::bad11'/,
        "croak if Fetch but no Store";

runFilter('bad12', <<'EOM');
    package DBM_Filter::bad12 ;

    sub Store {}

    1;
EOM

like $@, qr/^Filter_Push: Missing method 'Fetch' in class 'DBM_Filter::bad12'/,
        "croak if Store but no Fetch";

undef $db;
{
    use warnings FATAL => 'untie';
    eval { untie %h1 };
    is $@, '', "untie without inner references" ;
}

done_testing();
