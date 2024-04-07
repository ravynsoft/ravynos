#!./perl

# Verify that B::Concise properly reports whether functions are XS,
# perl, or optimized constant subs.

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use Carp;
use Test::More 'no_plan';

require_ok("B::Concise");

my %matchers = 
    ( constant	=> qr{ (?-x: is a constant sub, optimized to a \w+)
		      |(?-x: exists in stash, but has no START) }x,
      XS	=> qr/ is XS code/,
      perl	=> qr/ (next|db)state/,
      core	=> qr/ coreargs/, # CORE:: subs have no nextstate
      noSTART	=> qr/ has no START/,
);

use constant a_constant => 3;
use constant a_list_constant => 4,5,6;

my @subs_to_test = (
    'a stub'		  => noSTART  => \&baz,
    'a Perl sub'	  => perl     => sub { foo(); bar (); },
    'a constant Perl sub' => constant => sub () { 3 },
    'a constant constant' => constant => \&a_constant,
    'a list constant'	  => constant => \&a_list_constant,
    'an XSUB'		  => XS	      => \&utf8::encode,
    'a CORE:: sub'	  => core     => \&CORE::lc,
);
  
############

B::Concise::compile('-nobanner');	# set a silent default

while (@subs_to_test) {
    my ($func_name, $want, $sub) = splice @subs_to_test, 0, 3;

    croak "unknown type $want: $func_name\n"
	unless defined $matchers{$want};

    my ($buf, $err) = render($sub);
    my $res = like($buf, $matchers{$want}, "$want sub:\t $func_name");

    unless ($res) {
	# Test failed.  Report type that would give success.
	for my $m (keys %matchers) {
	    diag ("$name is of type $m"), last if $buf =~ $matchers{$m};
	}
    }
}

sub render {
    my ($func_name) = @_;

    B::Concise::reset_sequence();
    B::Concise::walk_output(\my $buf);

    my $walker = B::Concise::compile($func_name);
    eval { $walker->() };
    diag("err: $@ $buf") if $@;
    diag("verbose: $buf") if $opts{V};

    return ($buf, $@);
}

__END__
