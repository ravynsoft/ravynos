use strict;
use warnings;
use Test::More;
use Hash::Util::FieldHash qw( :all);

our @warnings;
BEGIN {
    $SIG{'__WARN__'} = sub { push @warnings, @_ };
    $| = 1;
}

my $fail_odd      = 'Odd number of elements in hash assignment at ';
my $fail_odd_anon = 'Odd number of elements in anonymous hash at ';
my $fail_ref      = 'Reference found where even-sized list expected at ';
my $fail_not_hr   = 'Not a HASH reference at ';

{
    @warnings = ();
    fieldhash my %hash;
    %hash = (1..3);
    cmp_ok(scalar(@warnings),'==',1,'odd count');
    cmp_ok(substr($warnings[0],0,length($fail_odd)),'eq',$fail_odd,'odd  msg');

    @warnings = ();
    %hash = 1;
    cmp_ok(scalar(@warnings),'==',1,'scalar count');
    cmp_ok(substr($warnings[0],0,length($fail_odd)),'eq',$fail_odd,'scalar msg');

    @warnings = ();
    %hash = { 1..3 };
    cmp_ok(scalar(@warnings),'==',2,'odd hashref count');
    cmp_ok(substr($warnings[0],0,length($fail_odd_anon)),'eq',$fail_odd_anon,'odd hashref msg 1');
    cmp_ok(substr($warnings[1],0,length($fail_ref)),'eq',$fail_ref,'odd hashref msg 2');

    @warnings = ();
    %hash = [ 1..3 ];
    cmp_ok(scalar(@warnings),'==',1,'arrayref count');
    cmp_ok(substr($warnings[0],0,length($fail_ref)),'eq',$fail_ref,'arrayref msg');

    @warnings = ();
    %hash = sub { print "fenice" };
    cmp_ok(scalar(@warnings),'==',1,'coderef count');
    cmp_ok(substr($warnings[0],0,length($fail_odd)),'eq',$fail_odd,'coderef msg');

    @warnings = ();
    $_ = { 1..10 };
    cmp_ok(scalar(@warnings),'==',0,'hashref assign');

}

done_testing;
