#!perl
BEGIN {
}

use strict;
use warnings;

use Test::More;
use Config;

our $DBM_Class;

my ($create, $write);
BEGIN {
    plan(skip_all => "$DBM_Class was not built")
	unless $Config{extensions} =~ /\b$DBM_Class\b/;
    plan(skip_all => "$DBM_Class not compatible with C++")
	 if $DBM_Class eq 'ODBM_File' && $Config{d_cplusplus};

    use_ok($DBM_Class);

    if ($::Create_and_Write) {
	($create, $write) = eval $::Create_and_Write;
	isnt($create, undef, "(eval q{$::Create_and_Write})[0]");
	isnt($write, undef, "(eval q{$::Create_and_Write})[1]");
    } else {
	#If Fcntl is not available, try 0x202 or 0x102 for O_RDWR|O_CREAT
	use_ok('Fcntl');
	$create = O_RDWR()|O_CREAT();
	$write = O_RDWR();
    }
}

unlink <Op_dbmx.*>;

umask(0);
my %h;
isa_ok(tie(%h, $DBM_Class, 'Op_dbmx', $create, 0640), $DBM_Class);

my $Dfile = "Op_dbmx.pag";
if (! -e $Dfile) {
	($Dfile) = <Op_dbmx*>;
}
SKIP: {
    skip "different file permission semantics on $^O", 1
	if $^O eq 'amigaos' || $^O eq 'os2' || $^O eq 'MSWin32' || $^O eq 'cygwin' || $^O eq 'vos';
    my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,
	$blksize,$blocks) = stat($Dfile);
    is($mode & 0777, 0640);
}
my $i = 0;
while (my ($key,$value) = each(%h)) {
    $i++;
}
is($i, 0);

$h{'goner1'} = 'snork';

$h{'abc'} = 'ABC';
$h{'def'} = 'DEF';
$h{'jkl','mno'} = "JKL\034MNO";
$h{'a',2,3,4,5} = join("\034",'A',2,3,4,5);
$h{'a'} = 'A';
$h{'b'} = 'B';
$h{'c'} = 'C';
$h{'d'} = 'D';
$h{'e'} = 'E';
$h{'f'} = 'F';
$h{'g'} = 'G';
$h{'h'} = 'H';
$h{'i'} = 'I';

$h{'goner2'} = 'snork';
delete $h{'goner2'};

untie(%h);
isa_ok(tie(%h, $DBM_Class, 'Op_dbmx', $write, 0640), $DBM_Class);

$h{'j'} = 'J';
$h{'k'} = 'K';
$h{'l'} = 'L';
$h{'m'} = 'M';
$h{'n'} = 'N';
$h{'o'} = 'O';
$h{'p'} = 'P';
$h{'q'} = 'Q';
$h{'r'} = 'R';
$h{'s'} = 'S';
$h{'t'} = 'T';
$h{'u'} = 'U';
$h{'v'} = 'V';
$h{'w'} = 'W';
$h{'x'} = 'X';
$h{'y'} = 'Y';
$h{'z'} = 'Z';

$h{'goner3'} = 'snork';

delete $h{'goner1'};
delete $h{'goner3'};

my @keys = keys(%h);
my @values = values(%h);

is($#keys, 29);
is($#values, 29);

while (my ($key, $value) = each(%h)) {
    if ($key eq $keys[$i] && $value eq $values[$i] && $key eq lc($value)) {
	$key =~ y/a-z/A-Z/;
	$i++ if $key eq $value;
    }
}

is($i, 30);

@keys = ('blurfl', keys(%h), 'dyick');
is($#keys, 31);

$h{'foo'} = '';
$h{''} = 'bar';

my $ok = 1;
for ($i = 1; $i < 200; $i++) { $h{$i + 0} = $i + 0; }
for ($i = 1; $i < 200; $i++) { $ok = 0 unless $h{$i} == $i; }
is($ok, 1, 'check cache overflow and numeric keys and contents');

my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,
   $blksize,$blocks) = stat($Dfile);
cmp_ok($size, '>', 0);

@h{0..200} = 200..400;
my @foo = @h{0..200};
is(join(':',200..400), join(':',@foo));

is($h{'foo'}, '');
is($h{''}, 'bar');

if($DBM_Class eq 'SDBM_File' || $DBM_Class eq 'NDBM_File') {
    is(exists $h{goner1}, '');
    is(exists $h{foo}, 1);
}

untie %h;
unlink <Op_dbmx*>, $Dfile;

{
   # sub-class test

   package Another;

   open my $file, '>', 'SubDB.pm' or die "Cannot open SubDB.pm: $!\n";
   printf $file <<'EOM', $DBM_Class, $DBM_Class, $DBM_Class;

   package SubDB;

   use strict;
   use warnings;

   require Exporter;
   use %s;
   our @ISA=qw(%s);
   our @EXPORT = @%s::EXPORT;

   sub STORE {
	my $self = shift;
        my $key = shift;
        my $value = shift;
        $self->SUPER::STORE($key, $value * 2);
   }

   sub FETCH {
	my $self = shift;
        my $key = shift;
        $self->SUPER::FETCH($key) - 1;
   }

   sub A_new_method
   {
	my $self = shift;
        my $key = shift;
        my $value = $self->FETCH($key);
	return "[[$value]]";
   }

   1;
EOM

    close $file or die "Could not close: $!";

    BEGIN { push @INC, '.'; }
    unlink <dbhash_tmp*>;

    main::use_ok('SubDB');
    my %h;
    my $X;
    eval '
	$X = tie(%h, "SubDB", "dbhash_tmp", $create, 0640 );
	';

    main::is($@, "");

    my $ret = eval '$h{"fred"} = 3; return $h{"fred"} ';
    main::is($@, "");
    main::is($ret, 5);

    $ret = eval '$X->A_new_method("fred") ';
    main::is($@, "");
    main::is($ret, "[[5]]");

    if ($DBM_Class eq 'GDBM_File') {
        $ret = eval 'GDBM_WRCREAT eq main::GDBM_WRCREAT';
        main::is($@, "");
        main::is($ret, 1);
    }

    undef $X;
    untie(%h);
    unlink "SubDB.pm", <dbhash_tmp*>;

}

untie %h;
unlink <Op_dbmx*>, $Dfile;

{
   # DBM Filter tests
   my (%h, $db);
   my ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;

   sub checkOutput
   {
       my($fk, $sk, $fv, $sv) = @_;
       local $Test::Builder::Level = $Test::Builder::Level + 1;
       is($fetch_key, $fk);
       is($store_key, $sk);
       is($fetch_value, $fv);
       is($store_value, $sv);
       is($_, 'original');
   }

   unlink <Op_dbmx*>;
   $db = tie %h, $DBM_Class, 'Op_dbmx', $create, 0640;
   isa_ok($db, $DBM_Class);

   $db->filter_fetch_key   (sub { $fetch_key = $_ });
   $db->filter_store_key   (sub { $store_key = $_ });
   $db->filter_fetch_value (sub { $fetch_value = $_});
   $db->filter_store_value (sub { $store_value = $_ });

   $_ = "original";

   $h{"fred"} = "joe";
   #                   fk   sk     fv   sv
   checkOutput("", "fred", "", "joe");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($h{"fred"}, "joe");
   #                   fk    sk     fv    sv
   checkOutput("", "fred", "joe", "");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($db->FIRSTKEY(), "fred");
   #                    fk     sk  fv  sv
   checkOutput("fred", "", "", "");

   # replace the filters, but remember the previous set
   my ($old_fk) = $db->filter_fetch_key
   			(sub { $_ = uc $_; $fetch_key = $_ });
   my ($old_sk) = $db->filter_store_key
   			(sub { $_ = lc $_; $store_key = $_ });
   my ($old_fv) = $db->filter_fetch_value
   			(sub { $_ = "[$_]"; $fetch_value = $_ });
   my ($old_sv) = $db->filter_store_value
   			(sub { s/o/x/g; $store_value = $_ });

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   $h{"Fred"} = "Joe";
   #                   fk   sk     fv    sv
   checkOutput("", "fred", "", "Jxe");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($h{"Fred"}, "[Jxe]");
   #                   fk   sk     fv    sv
   checkOutput("", "fred", "[Jxe]", "");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($db->FIRSTKEY(), "FRED");
   #                   fk   sk     fv    sv
   checkOutput("FRED", "", "", "");

   # put the original filters back
   $db->filter_fetch_key   ($old_fk);
   $db->filter_store_key   ($old_sk);
   $db->filter_fetch_value ($old_fv);
   $db->filter_store_value ($old_sv);

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   $h{"fred"} = "joe";
   checkOutput("", "fred", "", "joe");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($h{"fred"}, "joe");
   checkOutput("", "fred", "joe", "");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($db->FIRSTKEY(), "fred");
   checkOutput("fred", "", "", "");

   # delete the filters
   $db->filter_fetch_key   (undef);
   $db->filter_store_key   (undef);
   $db->filter_fetch_value (undef);
   $db->filter_store_value (undef);

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   $h{"fred"} = "joe";
   checkOutput("", "", "", "");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($h{"fred"}, "joe");
   checkOutput("", "", "", "");

   ($fetch_key, $store_key, $fetch_value, $store_value) = ("") x 4;
   is($db->FIRSTKEY(), "fred");
   checkOutput("", "", "", "");

   undef $db;
   untie %h;
   unlink <Op_dbmx*>;
}

{
    # DBM Filter with a closure

    my (%h, $db);

    unlink <Op_dbmx*>;
    $db = tie %h, $DBM_Class, 'Op_dbmx', $create, 0640;
    isa_ok($db, $DBM_Class);

    my %result = ();

    sub Closure
    {
        my ($name) = @_;
	my $count = 0;
	my @kept = ();

	return sub { ++$count;
		     push @kept, $_;
		     $result{$name} = "$name - $count: [@kept]";
		   }
    }

    $db->filter_store_key(Closure("store key"));
    $db->filter_store_value(Closure("store value"));
    $db->filter_fetch_key(Closure("fetch key"));
    $db->filter_fetch_value(Closure("fetch value"));

    $_ = "original";

    $h{"fred"} = "joe";
    is($result{"store key"}, "store key - 1: [fred]");
    is($result{"store value"}, "store value - 1: [joe]");
    is($result{"fetch key"}, undef);
    is($result{"fetch value"}, undef);
    is($_, "original");

    is($db->FIRSTKEY(), "fred");
    is($result{"store key"}, "store key - 1: [fred]");
    is($result{"store value"}, "store value - 1: [joe]");
    is($result{"fetch key"}, "fetch key - 1: [fred]");
    is($result{"fetch value"}, undef);
    is($_, "original");

    $h{"jim"}  = "john";
    is($result{"store key"}, "store key - 2: [fred jim]");
    is($result{"store value"}, "store value - 2: [joe john]");
    is($result{"fetch key"}, "fetch key - 1: [fred]");
    is($result{"fetch value"}, undef);
    is($_, "original");

    is($h{"fred"}, "joe");
    is($result{"store key"}, "store key - 3: [fred jim fred]");
    is($result{"store value"}, "store value - 2: [joe john]");
    is($result{"fetch key"}, "fetch key - 1: [fred]");
    is($result{"fetch value"}, "fetch value - 1: [joe]");
    is($_, "original");

    undef $db;
    untie %h;
    unlink <Op_dbmx*>;
}		

{
   # DBM Filter recursion detection
   my (%h, $db);
   unlink <Op_dbmx*>;

   $db = tie %h, $DBM_Class, 'Op_dbmx', $create, 0640;
   isa_ok($db, $DBM_Class);

   $db->filter_store_key (sub { $_ = $h{$_} });

   eval '$h{1} = 1234';
   like($@, qr/^recursion detected in filter_store_key at/);

   undef $db;
   untie %h;
   unlink <Op_dbmx*>;
}

{
    # Bug ID 20001013.009 (#4434)
    #
    # test that $hash{KEY} = undef doesn't produce the warning
    #     Use of uninitialized value in null operation

    unlink <Op_dbmx*>;
    my %h;
    my $a = "";
    local $SIG{__WARN__} = sub {$a = $_[0]};

    isa_ok(tie(%h, $DBM_Class, 'Op_dbmx', $create, 0640), $DBM_Class);
    $h{ABC} = undef;
    is($a, "");
    untie %h;
    unlink <Op_dbmx*>;
}

{
    # When iterating over a tied hash using "each", the key passed to FETCH
    # will be recycled and passed to NEXTKEY. If a Source Filter modifies the
    # key in FETCH via a filter_fetch_key method we need to check that the
    # modified key doesn't get passed to NEXTKEY.
    # Also Test "keys" & "values" while we are at it.

    unlink <Op_dbmx*>;
    my $bad_key = 0;
    my %h = ();
    my $db = tie %h, $DBM_Class, 'Op_dbmx', $create, 0640;
    isa_ok($db, $DBM_Class);
    $db->filter_fetch_key (sub { $_ =~ s/^Beta_/Alpha_/ if defined $_});
    $db->filter_store_key (sub { $bad_key = 1 if /^Beta_/; $_ =~ s/^Alpha_/Beta_/});

    $h{'Alpha_ABC'} = 2;
    $h{'Alpha_DEF'} = 5;

    is($h{'Alpha_ABC'}, 2);
    is($h{'Alpha_DEF'}, 5);

    my ($k, $v) = ("", "");
    while (($k, $v) = each %h) {}
    is($bad_key, 0);

    $bad_key = 0;
    foreach $k (keys %h) {}
    is($bad_key, 0);

    $bad_key = 0;
    foreach $v (values %h) {}
    is($bad_key, 0);

    undef $db;
    untie %h;
    unlink <Op_dbmx*>;
}

{
   # Check that DBM Filter can cope with read-only $_

   my %h;
   unlink <Op1_dbmx*>;

   my $db = tie %h, $DBM_Class, 'Op1_dbmx', $create, 0640;
   isa_ok($db, $DBM_Class);

   $db->filter_fetch_key   (sub { });
   $db->filter_store_key   (sub { });
   $db->filter_fetch_value (sub { });
   $db->filter_store_value (sub { });

   $_ = "original";

   $h{"fred"} = "joe";
   is($h{"fred"}, "joe");

   is_deeply([eval { map { $h{$_} } (1, 2, 3) }], [undef, undef, undef]);
   is($@, '');


   # delete the filters
   $db->filter_fetch_key   (undef);
   $db->filter_store_key   (undef);
   $db->filter_fetch_value (undef);
   $db->filter_store_value (undef);

   $h{"fred"} = "joe";

   is($h{"fred"}, "joe");

   is($db->FIRSTKEY(), "fred");

   is_deeply([eval { map { $h{$_} } (1, 2, 3) }], [undef, undef, undef]);
   is($@, '');

   undef $db;
   untie %h;
   unlink <Op1_dbmx*>;
}

done_testing();
1;
