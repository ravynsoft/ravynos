#!./perl -w
#
#  Copyright 2005, Adam Kennedy.
#
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

# Man, blessed.t scared the hell out of me. For a second there I thought
# I'd lose Test::More...

# This file tests several known-error cases relating to STORABLE_attach, in
# which Storable should (correctly) throw errors.

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Test::More tests => 40;
use Storable ();

#####################################################################
# Error 1
# 
# Classes that implement STORABLE_thaw _cannot_ have references
# returned by their STORABLE_freeze method. When they do, Storable
# should throw an exception



# Good Case - should not die
{
	my $goodfreeze = bless {}, 'My::GoodFreeze';
	my $frozen = undef;
	eval {
		$frozen = Storable::freeze( $goodfreeze );
	};
	ok( ! $@, 'Storable does not die when STORABLE_freeze does not return references' );
	ok( $frozen, 'Storable freezes to a string successfully' );

	package My::GoodFreeze;

	sub STORABLE_freeze {
		my ($self, $clone) = @_;
		
		# Illegally include a reference in this return
		return ('');
	}

	sub STORABLE_attach {
		my ($class, $clone, $string) = @_;
		return bless { }, 'My::GoodFreeze';
	}
}



# Error Case - should die on freeze
{
	my $badfreeze = bless {}, 'My::BadFreeze';
	eval {
		Storable::freeze( $badfreeze );
	};
	ok( $@, 'Storable dies correctly when STORABLE_freeze returns a reference' );
	# Check for a unique substring of the error message
	ok( $@ =~ /cannot return references/, 'Storable dies with the expected error' );

	package My::BadFreeze;

	sub STORABLE_freeze {
		my ($self, $clone) = @_;
		
		# Illegally include a reference in this return
		return ('', []);
	}

	sub STORABLE_attach {
		my ($class, $clone, $string) = @_;
		return bless { }, 'My::BadFreeze';
	}
}





#####################################################################
# Error 2
#
# If, for some reason, a STORABLE_attach object is accidentally stored
# with references, this should be checked and an error should be thrown.



# Good Case - should not die
{
	my $goodthaw = bless {}, 'My::GoodThaw';
	my $frozen = undef;
	eval {
		$frozen = Storable::freeze( $goodthaw );
	};
	ok( $frozen, 'Storable freezes to a string as expected' );
	my $thawed = eval {
		Storable::thaw( $frozen );
	};
	isa_ok( $thawed, 'My::GoodThaw' );
	is( $thawed->{foo}, 'bar', 'My::GoodThaw thawed correctly as expected' );

	package My::GoodThaw;

	sub STORABLE_freeze {
		my ($self, $clone) = @_;

		return ('');
	}

	sub STORABLE_attach {
		my ($class, $clone, $string) = @_;
		return bless { 'foo' => 'bar' }, 'My::GoodThaw';
	}
}



# Bad Case - should die on thaw
{
	# Create the frozen string normally
	my $badthaw = bless { }, 'My::BadThaw';
	my $frozen = undef;
	eval {
		$frozen = Storable::freeze( $badthaw );
	};
	ok( $frozen, 'BadThaw was frozen with references correctly' );

	# Set up the error condition by deleting the normal STORABLE_thaw,
	# and creating a STORABLE_attach.
	*My::BadThaw::STORABLE_attach = *My::BadThaw::STORABLE_thaw;
	*My::BadThaw::STORABLE_attach = *My::BadThaw::STORABLE_thaw; # Suppress a warning
	delete ${'My::BadThaw::'}{STORABLE_thaw};

	# Trigger the error condition
	my $thawed = undef;
	eval {
		$thawed = Storable::thaw( $frozen );
	};
	ok( $@, 'My::BadThaw object dies when thawing as expected' );
	# Check for a snippet from the error message
	ok( $@ =~ /unexpected references/, 'Dies with the expected error message' );

	package My::BadThaw;

	sub STORABLE_freeze {
		my ($self, $clone) = @_;

		return ('', []);
	}

	# Start with no STORABLE_attach method so we can get a
	# frozen object-containing-a-reference into the freeze string.
	sub STORABLE_thaw {
		my ($class, $clone, $string) = @_;
		return bless { 'foo' => 'bar' }, 'My::BadThaw';
	}
}




#####################################################################
# Error 3
#
# Die if what is returned by STORABLE_attach is not something of that class



# Good Case - should not die
{
	my $goodattach = bless { }, 'My::GoodAttach';
	my $frozen = Storable::freeze( $goodattach );
	ok( $frozen, 'My::GoodAttach return as expected' );
	my $thawed = eval {
		Storable::thaw( $frozen );
	};
	isa_ok( $thawed, 'My::GoodAttach' );
	is( ref($thawed), 'My::GoodAttach::Subclass',
		'The slightly-tricky good "returns a subclass" case returns as expected' );

	package My::GoodAttach;

	sub STORABLE_freeze {
		my ($self, $cloning) = @_;
		return ('');
	}

	sub STORABLE_attach {
		my ($class, $cloning, $string) = @_;

		return bless { }, 'My::GoodAttach::Subclass';
	}

	package My::GoodAttach::Subclass;

	BEGIN {
		@ISA = 'My::GoodAttach';
	}
}

# Good case - multiple references to the same object should be attached properly
{
	my $obj = bless { id => 111 }, 'My::GoodAttach::MultipleReferences';
    my $arr = [$obj];

    push @$arr, $obj;

	my $frozen = Storable::freeze($arr);

	ok( $frozen, 'My::GoodAttach return as expected' );

	my $thawed = eval {
		Storable::thaw( $frozen );
	};

	isa_ok( $thawed->[0], 'My::GoodAttach::MultipleReferences' );
	isa_ok( $thawed->[1], 'My::GoodAttach::MultipleReferences' );

	is($thawed->[0], $thawed->[1], 'References to the same object are attached properly');
	is($thawed->[1]{id}, $obj->{id}, 'Object with multiple references attached properly');

    package My::GoodAttach::MultipleReferences;

    sub STORABLE_freeze {
        my ($obj) = @_;
        $obj->{id}
    }

    sub STORABLE_attach {
        my ($class, $cloning, $id) = @_;
        bless { id => $id }, $class;
    }

}



# Bad Cases - die on thaw
{
	my $returnvalue = undef;

	# Create and freeze the object
	my $badattach = bless { }, 'My::BadAttach';
	my $frozen = Storable::freeze( $badattach );
	ok( $frozen, 'BadAttach freezes as expected' );

	# Try a number of different return values, all of which
	# should cause Storable to die.
	my @badthings = (
		undef,
		'',
		1,
		[],
		{},
		\"foo",
		(bless { }, 'Foo'),
		);
	foreach ( @badthings ) {
		$returnvalue = $_;

		my $thawed = undef;
		eval {
			$thawed = Storable::thaw( $frozen );
		};
		ok( $@, 'BadAttach dies on thaw' );
		ok( $@ =~ /STORABLE_attach did not return a My::BadAttach object/,
			'BadAttach dies on thaw with the expected error message' );
		is( $thawed, undef, 'Double checking $thawed was not set' );
	}
	
	package My::BadAttach;

	sub STORABLE_freeze {
		my ($self, $cloning) = @_;
		return ('');
	}

	sub STORABLE_attach {
		my ($class, $cloning, $string) = @_;

		return $returnvalue;
	}
}
