#!./perl 

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}


use Config;
if ( !$Config{d_alarm} ) {
    skip_all("alarm() not implemented on this platform");
}

plan tests => 5;
my $Perl = which_perl();

my ($start_time, $end_time);

eval {
    local $SIG{ALRM} = sub { $end_time = time; die "ALARM!\n" };
    $start_time = time;
    alarm 3;

    # perlfunc recommends against using sleep in combination with alarm.
    1 while (($end_time = time) - $start_time < 6);
    alarm 0;
};
alarm 0;
my $diff = $end_time - $start_time;

# alarm time might be one second less than you said.
is( $@, "ALARM!\n",             'alarm w/$SIG{ALRM} vs inf loop' );
ok( abs($diff - 3) <= 1,   "   right time (waited $diff secs for 3-sec alarm)" );


eval {
    local $SIG{ALRM} = sub { $end_time = time; die "ALARM!\n" };
    $start_time = time;
    alarm 3;
    system(qq{$Perl -e "sleep 6"});
    $end_time = time;
    alarm 0;
};
alarm 0;
$diff = $end_time - $start_time;

# alarm time might be one second less than you said.
is( $@, "ALARM!\n",             'alarm w/$SIG{ALRM} vs system()' );

{
    local $TODO = "Why does system() block alarm() on $^O?"
		if $^O eq 'VMS';
    ok( abs($diff - 3) <= 1,   "   right time (waited $diff secs for 3-sec alarm)" );
}


{
    local $SIG{"ALRM"} = sub { die };
    eval { alarm(1); my $x = qx($Perl -e "sleep 3"); alarm(0); };
    chomp (my $foo = "foo\n");
    ok($foo eq "foo", '[perl #33928] chomp() fails after alarm(), `sleep`');
}
