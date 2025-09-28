#!./perl -w

#
# test auto defined() test insertion
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(. ../lib) );
    $SIG{__WARN__} = sub { $warns++; warn $_[0] };
}

plan( tests => 27 );

my $unix_mode = 1;

if ($^O eq 'VMS') {
    # We have to know if VMS is in UNIX mode.  In UNIX mode, trailing dots
    # should not be present.  There are actually two settings that control this.

    $unix_mode = 0;
    my $unix_rpt = 0;
    my $drop_dot = 0;
    if (eval 'require VMS::Feature') {
        $unix_rpt = VMS::Feature::current('filename_unix_report');
        $drop_dot = VMS::Feature::current('readdir_dropdotnotype');
    } else {
        my $unix_report = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        $unix_rpt = $unix_report =~ /^[ET1]/i; 
        my $drop_dot_notype = $ENV{'DECC$READDIR_DROPDOTNOTYPE'} || '';
        $drop_dot = $drop_dot_notype =~ /^[ET1]/i;
    }
    $unix_mode = 1 if $drop_dot && unix_rpt;
}

# $wanted_filename should be 0 for readdir() and glob() tests.
# This is because it is the only valid filename that is false in a boolean test.

# $filename = '0';
# print "hi\n" if $filename; # doesn't print

# In the case of VMS, '0' isn't always the filename that you get.
# Which makes those particular tests pointless.

$wanted_filename = $unix_mode ? '0' : '0.';
$saved_filename = './0';

cmp_ok($warns,'==',0,'no warns at start');

ok(open(FILE,">$saved_filename"),'created work file');
print FILE "0\n";
print FILE "1\n";
close(FILE);

open(FILE,"<$saved_filename");
ok(defined(FILE),'opened work file');
my $seen = 0;
my $dummy;
while (my $name = <FILE>)
 {
  chomp($name);
  $seen++ if $name eq '0';
 }
cmp_ok($seen,'==',1,'seen in while()');

seek(FILE,0,0);
$seen = 0;
my $line = '';
do
 {
  chomp($line);
  $seen++ if $line eq '0';
 } while ($line = <FILE>);
cmp_ok($seen,'==',1,'seen in do/while');

seek(FILE,0,0);
$seen = 0;
while (($seen ? $dummy : $name) = <FILE> )
 {
  chomp($name);
  $seen++ if $name eq '0';
 }
cmp_ok($seen,'==',2,'seen in while() ternary');

seek(FILE,0,0);
$seen = 0;
my %where;
while ($where{$seen} = <FILE>)
 {
  chomp($where{$seen});
  $seen++ if $where{$seen} eq '0';
 }
cmp_ok($seen,'==',1,'seen in hash while()');
close FILE;

opendir(DIR,'.');
ok(defined(DIR),'opened current directory');
$seen = 0;
while (my $name = readdir(DIR))
 {
  $seen++ if $name eq $wanted_filename;
 }
cmp_ok($seen,'==',1,'saw work file once');

rewinddir(DIR);
$seen = 0;
$dummy = '';
while (($seen ? $dummy : $name) = readdir(DIR))
 {
  $seen++ if $name eq $wanted_filename;
 }
cmp_ok($seen,'>',0,'saw file in while() ternary');

rewinddir(DIR);
$seen = 0;
while ($where{$seen} = readdir(DIR))
 {
  $seen++ if $where{$seen} eq $wanted_filename;
 }
cmp_ok($seen,'==',1,'saw file in hash while()');

rewinddir(DIR);
$seen = 0;
$_ = 'not 0';
while (readdir(DIR))
 {
  $seen++ if $_ eq $wanted_filename;
 }
cmp_ok($seen,'==',1,'saw file in bare while(readdir){...}');

rewinddir(DIR);
$seen = 0;
$_ = 'not 0';

$_ eq $wanted_filename && $seen++ while readdir(DIR);
cmp_ok($seen,'==',1,'saw file in bare "... while readdir"');

rewinddir(DIR);
$seen = 0;
$_ = "";  # suppress uninit warning
do
 {
  $seen++ if $_ eq $wanted_filename;
 } while (readdir(DIR));
cmp_ok($seen,'==',1,'saw file in bare do{...}while(readdir)');

$seen = 0;
while (my $name = glob('*'))
 {
  $seen++ if $name eq $wanted_filename;
 }
cmp_ok($seen,'==',1,'saw file in glob while()');

$seen = 0;
$dummy = '';
while (($seen ? $dummy : $name) = glob('*'))
 {
  $seen++ if $name eq $wanted_filename;
 }
cmp_ok($seen,'>',0,'saw file in glob hash while() ternary');

$seen = 0;
while ($where{$seen} = glob('*'))
 {
  $seen++ if $where{$seen} eq $wanted_filename;
 }
cmp_ok($seen,'==',1,'seen in glob hash while()');

unlink($saved_filename);
ok(!(-f $saved_filename),'work file unlinked');

my %hash = (0 => 1, 1 => 2);
my @array = 1;
my $neg_sum= 0;

$seen = 0;

while (my $name = each %hash)
 {
  $neg_sum = $name - $neg_sum;
  $seen++ if $name eq '0';
 }
cmp_ok(abs($neg_sum),'==',1,'abs(neg_sum) should equal 1');
cmp_ok($seen,'==',1,'seen in each');

$seen = 0;
$dummy = '';
while (($seen ? $dummy : $name) = each %hash)
 {
  $seen++ if $name eq '0';
 }
cmp_ok($seen,'==',$neg_sum < 0 ? 1 : 2,'seen in each ternary');

$seen = 0;
while ($where{$seen} = each %hash)
 {
  $seen++ if $where{$seen} eq '0';
 }
cmp_ok($seen,'==',1,'seen in each hash');

$seen = 0;
undef $_;
while (each %hash)
 {
  $seen++ if $_ eq '0';
 }
cmp_ok($seen,'==',1,'0 seen in $_ in while(each %hash)');

$seen = 0;
undef $_;
while (each @array)
 {
  $seen++ if $_ eq '0';
 }
cmp_ok($seen,'==',1,'0 seen in $_ in while(each @array)');

$seen = 0;
undef $_;
$_ eq '0' and $seen++ while each %hash;
cmp_ok($seen,'==',1,'0 seen in $_ in while(each %hash) as stm mod');

$seen = 0;
undef $_;
$_ eq '0' and $seen++ while each @array;
cmp_ok($seen,'==',1,'0 seen in $_ in while(each @array) as stm mod');

cmp_ok($warns,'==',0,'no warns at finish');
