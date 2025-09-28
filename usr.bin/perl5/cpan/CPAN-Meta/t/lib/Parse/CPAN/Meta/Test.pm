package Parse::CPAN::Meta::Test;

use strict;
use Test::More ();
use Parse::CPAN::Meta;
use File::Spec;

use vars qw{@ISA @EXPORT};
BEGIN {
	require Exporter;
	@ISA    = qw{ Exporter };
	@EXPORT = qw{
		tests  yaml_ok  yaml_error  slurp  load_ok
		test_data_directory
	};
}

sub test_data_directory {
	return( "corpus" );
}

# 22 tests per call to yaml_ok
# 4  tests per call to load_ok
sub tests {
	return ( tests => count(@_) );
}

sub count {
	my $yaml_ok = shift || 0;
	my $load_ok = shift || 0;
	my $single  = shift || 0;
	my $count   = $yaml_ok * 3 + $load_ok * 4 + $single;
	return $count;
}

sub yaml_ok {
	my $string  = shift;
	my $array   = shift;
	my $name    = shift || 'unnamed';

	# Does the string parse to the structure
	my $yaml_copy = $string;
	my @yaml      = eval { Parse::CPAN::Meta::Load( $yaml_copy ); };
	Test::More::is( $@, '', "$name: Parse::CPAN::Meta parses without error" );
	Test::More::is( $yaml_copy, $string, "$name: Parse::CPAN::Meta does not modify the input string" );
	SKIP: {
		Test::More::skip( "Shortcutting after failure", 1 ) if $@;
		Test::More::is_deeply( \@yaml, $array, "$name: Parse::CPAN::Meta parses correctly" );
	}

	# Return true as a convenience
	return 1;
}

sub yaml_error {
	my $string = shift;
	my $yaml   = eval { Parse::CPAN::Meta::Load( $string ); };
	Test::More::like( $@, qr/$_[0]/, "CPAN::Meta::YAML throws expected error" );
}

sub slurp {
	my $file = shift;
	my $layer = shift;
	$layer = "" unless defined $layer;
	local $/ = undef;
	open my $fh, "<$layer", $file or die "open($file) failed: $!";
	my $source = <$fh>;
	close( $fh ) or die "close($file) failed: $!";
	$source;
}

sub load_ok {
	my $name = shift;
	my $file = shift;
	my $size = shift;
	my $layer = shift;
	Test::More::ok( -f $file, "Found $name" ) or Test::More::diag("Searched at '$file'");
	Test::More::ok( -r $file, "Can read $name" );
	my $content = slurp( $file, $layer );
	Test::More::ok( (defined $content and ! ref $content), "Loaded $name" );
	Test::More::ok( ($size < length $content), "Content of $name larger than $size bytes" );
	return $content;
}

1;
