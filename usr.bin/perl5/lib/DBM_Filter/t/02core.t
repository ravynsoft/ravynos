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

##### Keep above code identical to 01error.t #####

our $db;
my %files = ();

sub writeFile
{
    my $filename = shift ;
    my $content = shift;
    open F, '>', "DBM_Filter/$filename.pm" or croak "Cannot open $filename: $!" ;
    print F $content ;
    close F;
    $files{"DBM_Filter/$filename.pm"} ++;
}

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

unlink <coreOp_dbmx*>;
END { unlink <coreOp_dbmx*>; }

writeFile('times_ten', <<'EOM');
    package DBM_Filter::times_ten;
    sub Store { $_ *= 10 }
    sub Fetch { $_ /= 10 }
    1;
EOM

writeFile('append_A', <<'EOM');
    package DBM_Filter::append_A;
    sub Store { $_ .= 'A' }
    sub Fetch { s/A$//    }
    1;
EOM

writeFile('append_B', <<'EOM');
    package DBM_Filter::append_B;
    sub Store { $_ .= 'B' }
    sub Fetch { s/B$//    }
    1;
EOM

writeFile('append_C', <<'EOM');
    package DBM_Filter::append_C;
    sub Store { $_ .= 'C' }
    sub Fetch { s/C$//    }
    1;
EOM

writeFile('append_D', <<'EOM');
    package DBM_Filter::append_D;
    sub Store { $_ .= 'D' }
    sub Fetch { s/D$//    }
    1;
EOM

writeFile('append', <<'EOM');
    package DBM_Filter::append;
    sub Filter
    {
         my $string = shift ;
         return {
                    Store => sub { $_ .= $string   },
                    Fetch => sub { s/${string}$//  }
                }
    }
    1;
EOM

writeFile('double', <<'EOM');
    package DBM_Filter::double;
    sub Store { $_ *= 2 }
    sub Fetch { $_ /= 2 }
    1;
EOM

writeFile('uc', <<'EOM');
    package DBM_Filter::uc;
    sub Store { $_ = uc $_ }
    sub Fetch { $_ = lc $_ }
    1;
EOM

writeFile('reverse', <<'EOM');
    package DBM_Filter::reverse;
    sub Store { $_ = reverse $_ }
    sub Fetch { $_ = reverse $_ }
    1;
EOM


my %PreData = (
	'abc'	=> 'def',
	'123'	=> '456',
	);

my %PostData = (
	'alpha'	=> 'beta',
	'green'	=> 'blue',
	);

sub doPreData
{
    my $h = shift ;

    $$h{"abc"} = "def";
    $$h{"123"} = "456";
    ok $$h{"abc"} eq "def", "read eq written" ;
    ok $$h{"123"} eq "456", "read eq written" ;

}

sub doPostData
{
    my $h = shift ;

    no warnings 'uninitialized';
    $$h{undef()} = undef();
    $$h{"alpha"} = "beta";
    $$h{"green"} = "blue";
    ok $$h{""} eq "", "read eq written" ;
    ok $$h{"green"} eq "blue", "read eq written" ;
    ok $$h{"green"} eq "blue", "read eq written" ;

}

sub checkRaw
{
    my $filename = shift ;
    my %expected = @_ ;
    my %h;

    # read the dbm file without the filter
    ok tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640), "tied to $db_file";

    my %bad = ();
    while (my ($k, $v) = each %h) {
        if ( defined $expected{$k} &&  $expected{$k} eq $v ) {
            delete $expected{$k} ;
        }
        else
          { $bad{$k} = $v }
    }

    ok keys(%expected) + keys(%bad) == 0, "Raw hash is ok"; 

    if ( keys(%expected) + keys(%bad) ) {
        my $bad = "Expected does not match actual\nExpected:\n" ;
        while (my ($k, $v) = each %expected) {
            $bad .= "\t'$k' =>\t'$v'\n";
        }
        $bad .= "\nGot:\n" ;
        while (my ($k, $v) = each %bad) {
            $bad .= "\t'$k' =>\t'$v'\n";
        }
        diag $bad ;
    }
    
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }
    unlink <coreOp_dbmx*>;
}

{
    #diag "Test Set: Key and Value Filter, no stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'A'	=> 'A',
	    'alphaA'	=> 'betaA',
	    'greenA'	=> 'blueA';

}

{
    #diag "Test Set: Key Only Filter, no stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Key_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'A'	=> '',
	    'alphaA'	=> 'beta',
	    'greenA'	=> 'blue';

}

{
    #diag "Test Set: Value Only Filter, no stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Value_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    ''	=> 'A',
	    'alpha'	=> 'betaA',
	    'green'	=> 'blueA';

}

{
    #diag "Test Set: Key and Value Filter, with stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Push('append_B') };
    is $@, '', "push 'append_B' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'AB'	=> 'AB',
	    'alphaAB'	=> 'betaAB',
	    'greenAB'	=> 'blueAB';

}

{
    #diag "Test Set: Key Filter != Value Filter, with stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Value_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Key_Push('append_B') };
    is $@, '', "push 'append_B' filter" ;
    
    eval { $db->Filter_Value_Push('append_C') };
    is $@, '', "push 'append_C' filter" ;
    
    eval { $db->Filter_Key_Push('append_D') };
    is $@, '', "push 'append_D' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'BD'	=> 'AC',
	    'alphaBD'	=> 'betaAC',
	    'greenBD'	=> 'blueAC';

}

{
    #diag "Test Set: Key only Filter, with stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Key_Push('append_B') };
    is $@, '', "push 'append_B' filter" ;
    
    eval { $db->Filter_Key_Push('append_D') };
    is $@, '', "push 'append_D' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'BD'	=> '',
	    'alphaBD'	=> 'beta',
	    'greenBD'	=> 'blue';

}

{
    #diag "Test Set: Value only Filter, with stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Value_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Value_Push('append_C') };
    is $@, '', "push 'append_C' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    ''	=> 'AC',
	    'alpha'	=> 'betaAC',
	    'green'	=> 'blueAC';

}

{
    #diag "Test Set: Combination Key/Value + Key Filter != Value Filter, with stacking, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Value_Push('append_C') };
    is $@, '', "push 'append_C' filter" ;
    
    eval { $db->Filter_Key_Push('append_D') };
    is $@, '', "push 'append_D' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'AD'	=> 'AC',
	    'alphaAD'	=> 'betaAC',
	    'greenAD'	=> 'blueAC';

}

{
    #diag "Test Set: Combination Key/Value + Key + Key/Value, no closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Push('append_A') };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Key_Push('append_B') };
    is $@, '', "push 'append_B' filter" ;
    
    eval { $db->Filter_Push('append_C') };
    is $@, '', "push 'append_C' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'ABC'	=> 'AC',
	    'alphaABC'	=> 'betaAC',
	    'greenABC'	=> 'blueAC';

}

{
    #diag "Test Set: Combination Key/Value + Key + Key/Value, with closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Push('append' => 'A') };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Key_Push('append' => 'B') };
    is $@, '', "push 'append_B' filter" ;
    
    eval { $db->Filter_Push('append' => 'C') };
    is $@, '', "push 'append_C' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'ABC'	=> 'AC',
	    'alphaABC'	=> 'betaAC',
	    'greenABC'	=> 'blueAC';

}

{
    #diag "Test Set: Combination Key/Value + Key + Key/Value, immediate";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { 
        $db->Filter_Push(
                Store => sub { $_ .= 'A' },
                Fetch => sub { s/A$//    }) };
    is $@, '', "push 'append_A' filter" ;
    
    eval { 
        $db->Filter_Key_Push(
                Store => sub { $_ .= 'B' },
                Fetch => sub { s/B$//    }) };
    is $@, '', "push 'append_B' filter" ;
    
    eval { 
        $db->Filter_Push(
                Store => sub { $_ .= 'C' },
                Fetch => sub { s/C$//    }) };
    is $@, '', "push 'append_C' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'ABC'	=> 'AC',
	    'alphaABC'	=> 'betaAC',
	    'greenABC'	=> 'blueAC';

}

{
    #diag "Test Set: Combination Key/Value + Key + Key/Value, immediate, closure";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { 
        $db->Filter_Push(
                Store => sub { $_ .= 'A' },
                Fetch => sub { s/A$//    }) };
    is $@, '', "push 'append_A' filter" ;
    
    eval { $db->Filter_Key_Push('append_B') };
    is $@, '', "push 'append_B' filter" ;
    
    eval { $db->Filter_Push('append' => 'C') };
    is $@, '', "push 'append_C' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'ABC'	=> 'AC',
	    'alphaABC'	=> 'betaAC',
	    'greenABC'	=> 'blueAC';

}

{
    #diag "Test Set: Filtered & Filter_Pop";

    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    ok ! $db->Filtered, "not filtered" ;

    eval { 
        $db->Filter_Push(
                Store => sub { $_ .= 'A' },
                Fetch => sub { s/A$//    }) };
    is $@, '', "push 'append_A' filter" ;
    
    ok $db->Filtered, "is filtered" ;

    eval { $db->Filter_Key_Push('append_B') };
    is $@, '', "push 'append_B' filter" ;
    
    ok $db->Filtered, "is filtered" ;
    
    eval { $db->Filter_Push('append' => 'C') };
    is $@, '', "push 'append_C' filter" ;
    
    ok $db->Filtered, "is filtered" ;
    
    doPostData(\%h);
    
    eval { $db->Filter_Pop() };
    is $@, '', "Filter_Pop";
    
    ok $db->Filtered, "is filtered" ;

    $h{'after'} = 'noon';
    is $h{'after'}, 'noon', "read eq written";

    eval { $db->Filter_Pop() };
    is $@, '', "Filter_Pop";
    
    ok $db->Filtered, "is filtered" ;

    $h{'morning'} = 'after';
    is $h{'morning'}, 'after', "read eq written";

    eval { $db->Filter_Pop() };
    is $@, '', "Filter_Pop";
    
    ok ! $db->Filtered, "not filtered" ;

    $h{'and'} = 'finally';
    is $h{'and'}, 'finally', "read eq written";

    eval { $db->Filter_Pop() };
    is $@, '', "Filter_Pop";
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'ABC'	=> 'AC',
	    'alphaABC'	=> 'betaAC',
	    'greenABC'	=> 'blueAC',
	    'afterAB'	=> 'noonA',
	    'morningA'	=> 'afterA',
	    'and'	=> 'finally';

}

{
    #diag "Test Set: define the filter package in-line";

    {
        package DBM_Filter::append_X;

        sub Store { $_ .= 'X' }
        sub Fetch { s/X$//    }
    }
    
    my %h = () ;
    my $db = tie(%h, $db_file,'coreOp_dbmx', O_RDWR|O_CREAT, 0640) ;
    ok $db, "tied to $db_file";
    
    doPreData(\%h);

    eval { $db->Filter_Push('append_X') };
    is $@, '', "push 'append_X' filter" ;
    
    doPostData(\%h);
    
    undef $db;
    {
        use warnings FATAL => 'untie';
        eval { untie %h };
        is $@, '', "untie without inner references" ;
    }

    checkRaw 'coreOp_dbmx',
	    'abc'	=> 'def',
	    '123'	=> '456',
	    'X'  	=> 'X',
	    'alphaX'	=> 'betaX',
	    'greenX'	=> 'blueX';

}

done_testing();
