BEGIN { chdir 't' if -d 't' }

use Test::More;
use strict;
use lib '../lib';

plan skip_all => "File contains an alien character set" if ord "A" != 65;

my $Class   = 'Archive::Tar';
my $FClass  = 'Archive::Tar::File';
my $File    = 'src/long/bar.tar';
my @Expect = (
    qr|^c$|,
    qr|^d$|,
    qr|^directory/$|,
    qr|^directory/really.*name/$|,
    qr|^directory/.*/myfile$|,
);

use_ok( $Class );

### crazy ref to special case 'all'
for my $index ( \0, 0 .. $#Expect ) {

    my %opts    = ();
    my @expect  = ();

    my $dotest = sub {
	my $desc = shift;
	my $next = $Class->iter( $File, 0, \%opts );

	my $pp_opts = join " => ", %opts;
	ok( $next,                  "Iterator created from $File ($pp_opts $desc)" );
	isa_ok( $next, "CODE",      "   Iterator $desc" );

	my @names;
	while( my $f = $next->() ) {
	    ok( $f,                 "       File object retrieved $desc" );
	    isa_ok( $f, $FClass,    "           Object $desc" );

	    push @names, $f->name;
	}

	is( scalar(@names), scalar(@expect),
				    "   Found correct number of files $desc" );

	my $i = 0;
	for my $name ( @names ) {
	    ok( 1,                  "   Inspecting '$name'  $desc" );
	    like($name, $expect[$i],"       Matches $Expect[$i] $desc" );
	    $i++;
	}
    };

    ### do a full test vs individual filters
    if( not ref $index ) {
        my $regex       = $Expect[$index];
        @expect         = ($regex);
	%opts		= ( filter => $regex );
	$dotest->("filter $regex");
	%opts		= ( filter_cb => sub { my ($entry) = @_; $entry->name() =~ /$regex/ } );
	$dotest->("filter_cb $regex");
    } else {
        @expect         = @Expect;
	$dotest->("all");
    }
}

done_testing;
