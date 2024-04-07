#!/usr/bin/perl -w

BEGIN {
    if ($ENV{PERL_CORE}) {
	require Config;
	if ($Config::Config{'extensions'} !~ /\bSocket\b/) {
		print "1..0 # Skip: Socket not built - IO.pm uses Socket";
		exit 0;
	}
    }
}

use strict;
use File::Path;
use File::Spec;
require($ENV{PERL_CORE} ? "../../t/test.pl" : "./t/test.pl");
plan(tests => 18);

{
	require XSLoader;

	my @load;
	local $^W;
	my $xsl = \&XSLoader::load;
	local *XSLoader::load = sub {
		push @load, \@_;
		&$xsl(@_);
	};

	# use_ok() calls import, which we do not want to do
	require_ok( 'IO' );
	ok( @load, 'IO should call XSLoader::load()' );
	is( $load[0][0], 'IO', '... loading the IO library' );
	is( $load[0][1], $IO::VERSION, '... with the current .pm version' );
}

my @default = map { "IO/$_.pm" } qw( Handle Seekable File Pipe Socket Dir );
delete @INC{ @default };

my $warn = '' ;
local $SIG{__WARN__} = sub { $warn = "@_" } ;

{
    no warnings ;
    IO->import();
    is( $warn, '', "... import default, should not warn");
    $warn = '' ;
}

{
    local $^W = 0;
    no if $^V >= 5.17.4, warnings => "deprecated";
    IO->import();
    is( $warn, '', "... import default, should not warn");
    $warn = '' ;
}

{
    local $^W = 1;
    IO->import();
    like( $warn, qr/^Parameterless "use IO" deprecated at/, 
              "... import default, should warn");
    $warn = '' ;
}

{
    use warnings 'deprecated' ;
    IO->import(); 
    like( $warn, qr/^Parameterless "use IO" deprecated at/, 
              "... import default, should warn");
    $warn = '' ;
}

{
    use warnings ;
    IO->import();
    like( $warn, qr/^Parameterless "use IO" deprecated at/, 
              "... import default, should warn");
    $warn = '' ;
}

foreach my $default (@default)
{
	ok( exists $INC{ $default }, "... import should default load $default" );
}

eval { IO->import( 'nothere' ) };
like( $@, qr/Can.t locate IO.nothere\.pm/, '... croaking on any error' );

my $fakedir = File::Spec->catdir( 'lib', 'IO' );
my $fakemod = File::Spec->catfile( $fakedir, 'fakemod.pm' );

my $flag;
if ( -d $fakedir or mkpath( $fakedir ))
{
	if (open( OUT, '>', $fakemod ))
	{
		(my $package = <<'		END_HERE') =~ tr/\t//d;
		package IO::fakemod;

		sub import { die "Do not import!\n" }

		sub exists { 1 }

		1;
		END_HERE

		print OUT $package;
	}

	if (close OUT)
	{
		$flag = 1;
		push @INC, 'lib';
	}
}

SKIP:
{
	skip("Could not write to disk", 2 ) unless $flag;
	eval { IO->import( 'fakemod' ) };
	ok( IO::fakemod::exists(), 'import() should import IO:: modules by name' );
	is( $@, '', '... and should not call import() on imported modules' );
}

END
{
	1 while unlink $fakemod;
	rmdir $fakedir;
}
