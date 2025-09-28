#!perl -w
use strict;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}
use File::Copy ();
use File::Path ();
use File::Spec ();
plan(tests => 10);

my $test_dir = File::Spec->catdir(qw(lib deprecate));
chdir $test_dir or die "Can't chdir $test_dir";
@INC = ( File::Spec->catdir( (File::Spec->updir)x3, qw(lib)) );

my %libdir = (
	privlibexp	=> File::Spec->catdir(qw(lib perl)),
	sitelibexp	=> File::Spec->catdir(qw(lib site)),
	archlibexp	=> File::Spec->catdir(qw(lib perl arch)),
	sitearchexp	=> File::Spec->catdir(qw(lib site arch)),
);

File::Path::make_path(values %libdir); 

push @INC, @libdir{qw(archlibexp privlibexp sitearchexp sitelibexp)};

our %tests = (
	privlibexp	=> 1,
	sitelibexp	=> 0,
	archlibexp	=> 1,
	sitearchexp	=> 0,
);

no warnings 'once';
local %deprecate::Config = (%libdir);

my $module = 'Deprecated.pm';
for my $lib (sort keys %tests) {
    my $dir = $libdir{$lib};
    my $pm = File::Spec->catfile($dir, $module);
    File::Copy::copy($module, $pm);

    my $warn = '';
    {   local $SIG{__WARN__} = sub { $warn .= $_[0]; };
        use warnings qw(deprecated);
#line 1001
	require Deprecated;
#line
    }
    if( $tests{$lib} ) {
        like($warn, qr/^Deprecated\s+will\s+be\s+removed\b/, "$lib - message");
        my $me = quotemeta($0);
        like($warn, qr/$me,?\s+line\s+1001\.?\n*$/, "$lib - location");
    }
    else {
	ok( !$warn, "$lib - no message" );
    }

    delete $INC{$module};
    unlink_all $pm;
}

my $sub_dir = 'Optionally';
my $opt_mod = $sub_dir .'.pm';
for my $lib (sort keys %tests) {
    my $dir = File::Spec->catdir($libdir{$lib}, $sub_dir);
    File::Path::make_path($dir);

    my $pm = File::Spec->catfile($dir, $module);
    File::Copy::copy($opt_mod, $pm);

    my $warn = '';
    {   local $SIG{__WARN__} = sub { $warn .= $_[0]; };
        use warnings qw(deprecated);
	require Optionally::Deprecated;
    }
    if( $tests{$lib} ) {
        like($warn, qr/^Optionally::Deprecated\s+will\s+be\s+removed\b/,
		"$lib - use if - message");
    }
    else {
	ok( !$warn, "$lib - use if - no message" );
    }

    delete $INC{"$sub_dir/$module"};
    unlink_all $pm;
}

END { File::Path::remove_tree('lib') }
