#!./perl

# tests for DATA filehandle operations

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

$|=1;

# It is important that all these tests are run via fresh_perl because
# that way they get written to disk in text mode and will have CR-LF
# line endings on Windows.  Otherwise the failures related to Perl
# code being read in binary mode will not be observed.

run_multiple_progs('', \*DATA);

done_testing();

__END__
# https://github.com/Perl/perl5/issues/7207#issuecomment-543940952
while (<DATA>) {
    chomp;
    print "$.: '$_'\n";
    system();
}
__DATA__
1
2
3
EXPECT
1: '1'
2: '2'
3: '3'
########
# https://github.com/Perl/perl5/issues/7207#issuecomment-543940955
my $line1 = <DATA>;
`echo foo`;
my $line2 = <DATA>;
if ($line1 eq "one\n") { print "ok 1\n" } else { print "not ok 1\n" }
if ($line2 eq "two\n") { print "ok 2\n" } else { print "not ok 2\n" }
__DATA__
one
two
EXPECT
ok 1
ok 2
########
# https://github.com/Perl/perl5/issues/7207#issuecomment-543940992
my @data_positions = tell(DATA);
while (<DATA>){
    if (/^__DATA__$/) {
        push @data_positions, tell(DATA);
    }
}

my @fh_positions;
open(my $fh, '<', $0) or die;
while (<$fh>){
    if (/^__DATA__$/) {
        push @fh_positions, tell($fh);
    }
}

print "not " unless "@data_positions" eq "@fh_positions";
print "ok";

__DATA__
ab
__DATA__
ab

__DATA__
ab
__DATA__
lotsa junk
nothing
EXPECT
ok
########
# Which package is __DATA__ in?
package foo;
BEGIN{*foo::=*bar::}
print <DATA>;
__DATA__
123
EXPECT
123
